#include "based_basic.h"
#include "based_logging.h"
#include "vos.h"
#include "vos_socket.h"
#include "http.h"
#include "stb_ds.h"

#include <stdio.h>
#include <string.h>
#include <raylib.h>

#define HTTP_RECV_BUFSIZE 4096
#define MAX_BODY_LEN 8192

int http_RecvAndParse(http_Connection* conn, http_Response* resp);

// reads the body length into the buffer
http_Error ReadBody(char* dest, http_Connection* conn, u32 len)
{
    ReceiveBuffer* buf = &conn->buf;
    u32 remaining = buf->len - buf->pos;

    if (remaining >= len)
    {
        memcpy(dest, buf->data + buf->pos, len);
        buf->pos += len;
        return 0;
    }

    // flush the buffer, then recv the rest

    memcpy(dest, buf->data + buf->pos, remaining);
    dest += remaining;
    buf->pos += remaining;

    s32 toRecv = len - remaining;

    while (toRecv > 0)
    {
        s32 recvLen = vos_Recv(conn->sok, dest, toRecv, 0);
        if (recvLen < 0)
        {
            return http_ReceiveFailed;
        }

        dest += recvLen;
        toRecv -= recvLen;
    }

    return 0;
}

http_Error CreateTCPSocket(vos_SocketID* sok)
{
    *sok = vos_Socket(AF_INET, SOCK_STREAM, 0);
    if (*sok == VOS_INVALID_SOCKET)
    {
        vos_NetError err = vos_GetNetError();
        BSD_ERR("Create socket failed with error %d", err);
        return http_SocketFailure;
    }
    return http_Success;
}

http_Error http_ConnectionCreate(http_Connection** connection)
{
    if (*connection != NULL)
    {
        vos_SocketClose((*connection)->sok);
        MemFree((*connection)->buf.data);
        MemFree(*connection);
    }

    http_Connection* newConnection = MemAlloc(sizeof(http_Connection));
    newConnection->buf.data = MemAlloc(HTTP_RECV_BUFSIZE);

    newConnection->buf.cap = HTTP_RECV_BUFSIZE;
    newConnection->buf.len = 0;
    newConnection->buf.pos = 0;

    http_Error err = CreateTCPSocket(&newConnection->sok);
    if (err)
    {
        return err;
    }

    *connection = newConnection;
    return http_Success;
}

void http_ConnectionClose(http_Connection* connection)
{
    vos_SocketClose(connection->sok);
    MemFree(connection->buf.data);
    MemFree(connection);
}

// NOTE: probably needs a non-blocking API if we ever want to do anything serious over HTTP
int http_ReqAndWaitForResp(http_Connection* conn, const http_Request* req, http_Response* resp)
{
    static char* methods[] =
    {
        "GET",
        "POST"
    };

    DBG_ASSERT_MSG(req->method < http_MethodNum, "Invalid HTTP method: %d", req->method);

    int port = req->port;
    if (port == 0)
    {
        BSD_DBG("Called %s with port 0, will default to port 80", __FUNCTION__);
        port = 80;
    }

    // NOTE: opening and closing a socket for every request is horrible if we ever use this for
    // frequent requests. currently we are not so i guess it's fine?
    unsigned int ipaddr;
    int err = vos_GetIPv4FromHost(req->hostName, &ipaddr);
    if (err)
    {
        BSD_ERR("Failed to resolve host name");
        return -1;
    }

    char reqBuf[HTTP_MAX_REQ_SIZE];

    // TODO: fill this in with any relevant headers
    char headers[HTTP_MAX_REQ_SIZE] = "Connection: close\r\n";

    int headerLen = strlen(headers);

    if (req->body.str)
    {
        sprintf(headers+headerLen, "Content-Length: %d\r\n", req->body.len);
    }

    int reqLen = snprintf(
        reqBuf,
        HTTP_MAX_REQ_SIZE,
        "%s %s HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "%s"
        "\r\n",
        methods[req->method],
        req->hostURL,
        req->hostName,
        port,
        headers);

    BSD_DBG("Sending request: %s", reqBuf);
    
    DBG_ASSERT_MSG(reqLen <= HTTP_MAX_REQ_SIZE, "Output of snprintf is truncated");

    BSD_DBG("Trying to connect to IP: %0x", ipaddr);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ipaddr;
    addr.sin_port = HTON16(port);
    err = vos_ConnectWithTimeout(conn->sok, (struct sockaddr*)&addr, sizeof(struct sockaddr_in), 1000);
    if (err)
    {
        BSD_ERR("Connect failed with code %d", err);
        vos_SocketClose(conn->sok);
        return -1;
    }

    BSD_DBG("Connection success");
    int bytesSent = vos_Send(conn->sok, reqBuf, reqLen, 0);
    if (bytesSent < 0)
    {
        BSD_ERR("Send HTTP request failed");
        vos_SocketClose(conn->sok);
        return -1;
    }

    if (bytesSent != reqLen)
    {
        return http_FailedToSendReq;
    }

    if (req->body.str)
    {
        bytesSent = vos_Send(conn->sok, req->body.str, req->body.len, 0);
        if (bytesSent < 0)
        {
            BSD_ERR("Send HTTP request body failed");
            vos_SocketClose(conn->sok);
            return -1;
        }

        if (bytesSent != req->body.len)
        {
            BSD_ERR("Failed to send entire request body, got %d, expected %d", bytesSent, req->body.len);
            return -1;
        }
    }

#if 0
    char* respBuf = reqBuf; // use the same buffer, cos why not
    int len = vos_Recv(conn->sok, respBuf, HTTP_MAX_RESP_SIZE - 1, 0);
    if (len <= 0)
    {
        BSD_ERR("Error while receiving HTTP response");
        vos_SocketClose(conn->sok);
        return -1;
    }
    DBG_ASSERT_MSG(len < HTTP_MAX_RESP_SIZE - 1, "Response too large to fit in buffer");
#endif

    err = http_RecvAndParse(conn, resp);
    if (err)
    {
        BSD_ERR("Error while parsing HTTP response");
        vos_SocketClose(conn->sok);
    }
    return err;
}

typedef enum
{
    ParsingStatusLine,
    ParsingHeaders,
    ParsingBody,
    ParsingDone,
} http_ParseState;

#define STR_LIT_LEN(str) (sizeof(str) - 1)
#define HTTP_MIN_LINE_LEN STR_LIT_LEN("HTTP/1.1 200")
s32 http_ParseStatus(http_String line)
{
    if (line.len < HTTP_MIN_LINE_LEN)
    {
        BSD_ERR("Status line too short, length %d", line.len);
        return -1;
    }
    
    static const char httpVersionStr[] = "HTTP/1.1 ";

    u32 httpVerLen = STR_LIT_LEN(httpVersionStr);
    if (memcmp(line.str, httpVersionStr, httpVerLen) != 0)
    {
        BSD_ERR("Unexpected HTTP version");
        return -1;
    }

    char* statusCodeStrStart = line.str + httpVerLen;
    char* statusCodeStrPos = statusCodeStrStart;
    char* lineEnd = line.str + line.len;

    u32 statusCode = 0;
    // epic positive integer parser
    while (bsd_IsDigit(*statusCodeStrPos) && statusCodeStrPos < lineEnd)
    {
        statusCode *= 10;
        statusCode += *statusCodeStrPos - '0';
        statusCodeStrPos++;
    }

    if (statusCodeStrPos - statusCodeStrStart != 3)
    {
        BSD_ERR("HTTP status is of unexpected length");
        return -1;
    }

    if (statusCode < 100 || statusCode > 599)
    {
        BSD_ERR("Invalid status code");
        return -1;
    }
    return statusCode;
}

http_Error GetLine(http_String* line, http_Connection* conn)
{
    DBG_ASSERT_MSG(conn != NULL, "NULL connection!");

    bool foundNewLine = false;
    ReceiveBuffer* buf = &conn->buf;

    u32 readPos = buf->pos;
    while (!foundNewLine)
    {
        if (buf->len == readPos)
        {
            // we need to retrieve more data

            if (buf->cap == buf->len)
            {
                u32 remainingBytes = buf->len - buf->pos;
                if (remainingBytes >= HTTP_RECV_BUFSIZE)
                {
                    // this means that the line doesn't fit inside the buffer.
                    // we won't support header lines > buf size bytes
                    return http_HeaderLineTooLong;
                }

                // move the remaining bytes to the start
                // memcpy assumes non-aliasing pointers so we will do this by hand
                for (int i = 0; i < remainingBytes; i++)
                {
                    buf->data[i] = buf->data[buf->pos+i];
                }
                buf->pos = 0;
                buf->len = remainingBytes;
            }

            // recv data into buf
            u32 maxRecvBytes = buf->cap - buf->len;
            s32 recvLen = vos_Recv(conn->sok, buf->data+buf->pos, maxRecvBytes, 0);
            if (recvLen <= 0)
            {
                BSD_ERR("Receive bytes failed");
                return http_ReceiveFailed;
            }

            buf->len += recvLen;
        }
        
        // we are guaranteed to have data at readPos here.

        if (buf->data[readPos] == '\n')
        {
            if (readPos == 0 || buf->data[readPos-1] != '\r')
            {
                return http_InvalidHeaderLine;
            }
            line->str = (char*)buf->data + buf->pos;
            // minus 2 to skip the newline chars
            line->len = readPos + 1 - buf->pos - 2;
            buf->pos = readPos + 1;
            return http_Success;
        }
        readPos++;
    }

    return http_ParsingFailed;
}

static stbds_string_arena http_arena = {0};
#define MAX_KEY_LEN 512
#define MAX_VAL_LEN 1024
s32 http_ParseHeader(http_String line, http_Header** headerTable)
{
    char* colonPos = memchr(line.str, ':', line.len);


    u32 keylen = colonPos - line.str;

    if (keylen >= MAX_KEY_LEN)
    {
        BSD_ERR("Header key too long");
        return -1;
    }

    // sh_new_arena(headerTable); we expect to be called at the higher level
    char tmpKeyBuf[MAX_KEY_LEN];

    memcpy(tmpKeyBuf, line.str, keylen);
    tmpKeyBuf[keylen] = 0;

    char* curPos = colonPos + 1;

    // skip any spaces
    while (*curPos == ' ' && curPos < line.str + line.len)
    {
        curPos++;
    }

    u32 remaining = line.str + line.len - curPos;

    if (remaining >= MAX_VAL_LEN)
    {
        BSD_ERR("Header value too long");
        return -1;
    }

    char tmpValBuf[MAX_VAL_LEN];
    memcpy(tmpValBuf, curPos, remaining);
    tmpValBuf[remaining] = 0;

    char* value = stbds_stralloc(&http_arena, tmpValBuf);

    BSD_DBG("Setting headers key %s val %s", tmpKeyBuf, tmpValBuf);
    shput(*headerTable, tmpKeyBuf, value);

    BSD_DBG("Got headers key %s val %s", tmpKeyBuf, shget(*headerTable, tmpKeyBuf));
    return 0;
}

// http_RespFree() needs to be called on the response struct
int http_RecvAndParse(http_Connection* conn, http_Response* resp)
{
    DBG_ASSERT_MSG(resp != NULL, "Response struct is NULL");
    DBG_ASSERT_MSG(resp->content.str == NULL, "memory leak danger!");
    DBG_ASSERT_MSG(resp->headers == NULL, "memory leak danger!");


    http_ParseState state = ParsingStatusLine;
    http_Error ret = http_Success;

    sh_new_arena(resp->headers);

    while (state != ParsingDone)
    {
        switch (state)
        {
            case ParsingStatusLine:
            {
                http_String line = {0};
                http_Error err = GetLine(&line, conn);
                if (err)
                {
                    ret = err;
                    goto error;
                }

                s32 statusCode = http_ParseStatus(line);
                if (statusCode < 0)
                {
                    ret = http_ParsingFailed;
                    goto error;
                }
                resp->status = statusCode;
                state = ParsingHeaders;
            } break;
            case ParsingHeaders:
            {
                http_String line;
                http_Error err = GetLine(&line, conn);
                if (err)
                {
                    ret = err;
                    goto error;
                }

                if (line.len == 0)
                {
                    state = ParsingBody;
                }
                else
                {
                    s32 err = http_ParseHeader(line, &resp->headers);
                    if (err)
                    {
                        ret = http_ParsingFailed;
                        goto error;
                    }
                }
            } break;
            case ParsingBody:
            {
                char* contentLenStr =  shget(resp->headers, "Content-Length");
                if (!contentLenStr)
                {
                    BSD_ERR("Could not find content length header");
                    ret = http_ParsingFailed;
                    goto error;
                }

                char* numEnd;
                long contentLen = strtol(contentLenStr, &numEnd, 10);
                if (contentLen > MAX_BODY_LEN)
                {
                    ret = http_ParsingFailed;
                    goto error;
                }

                DBG_ASSERT_MSG(resp->content.str == NULL, "library interally allocates");
                resp->content.str = MemAlloc(contentLen);
                http_Error err = ReadBody(resp->content.str, conn, contentLen);
                if (err)
                {
                    MemFree(resp->content.str);
                    ret = err;
                    goto error;
                }
                resp->content.len = contentLen;

                state = ParsingDone;
            } break;
            default:
            {
                BSD_ERR("Unsupported state %d", state);
                goto error;
            } break;
        }
    }

    DBG_ASSERT_MSG(resp->status >= 100 && resp->status <= 511, "Invalid status code");
    return 0;

error:
    shfree(resp->headers);
    resp->headers = NULL;
    DBG_ASSERT_MSG(ret != http_Success, "returning success on error!");
    return ret;
}

void http_ResponseFree(http_Response* resp)
{
    if (resp->headers)
    {
        shfree(resp->headers);
        resp->headers = NULL;
    }
    if (resp->content.str)
    {
        MemFree(resp->content.str);
        resp->content.str = NULL;
    }
    stbds_strreset(&http_arena);
}