#include "based_basic.h"
#include "based_logging.h"
#include "vos.h"
#include "http.h"

#include <string.h>


// NOTE: probably needs a non-blocking API if we ever want to do anything serious over HTTP
int http_SendRequest(const http_Request* req, http_Response* resp)
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
    char headers[HTTP_MAX_REQ_SIZE] = "Connection: close\n";

    int reqLen = snprintf(
        reqBuf,
        HTTP_MAX_REQ_SIZE,
        "%s / HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "%s"
        "\r\n",
        methods[req->method],
        req->hostName,
        port,
        headers);

    BSD_DBG("Sending request: %s", reqBuf);
    
    DBG_ASSERT_MSG(reqLen <= HTTP_MAX_REQ_SIZE, "Output of snprintf is truncated");

    vos_SocketInfo info;
    info.ipver = vos_IPv4;
    info.mode = vos_SocketModeBlocking;
    info.type = vos_SocketTCP;
    vos_SocketHandle sok = vos_Socket(&info);
    if (sok == vos_INVALID_SOCKET_HANDLE)
    {
        BSD_ERR("Failed to create socket");
        return -1;
    }

    BSD_DBG("Trying to connect to IP: %0x", ipaddr);
    err = vos_Connect(sok, ipaddr, HTON16(port));
    if (err)
    {
        BSD_ERR("Connect failed with code %d", err);
        vos_SocketClose(sok);
        return -1;
    }

    BSD_DBG("Connection success");
    int bytesSent = vos_Send(sok, reqBuf, reqLen);
    if (bytesSent < 0)
    {
        BSD_ERR("Send HTTP request failed");
        vos_SocketClose(sok);
        return -1;
    }

    char* respBuf = reqBuf; // use the same buffer, cos why not
    int len = vos_Receive(sok, respBuf, HTTP_MAX_RESP_SIZE - 1);
    if (len <= 0)
    {
        BSD_ERR("Error while receiving HTTP response");
        vos_SocketClose(sok);
        return -1;
    }
    DBG_ASSERT_MSG(len < HTTP_MAX_RESP_SIZE - 1, "Response too large to fit in buffer");

    err = http_ParseResponse(sok, respBuf, len, resp);
    if (err)
    {
        BSD_ERR("Error while parsing HTTP response");
        vos_SocketClose(sok);
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

int http_ParseResponse(vos_SocketHandle sok, char* buf, int len, http_Response* resp)
{
    DBG_ASSERT_MSG(buf != NULL, "Response buffer is NULL");
    DBG_ASSERT_MSG(resp != NULL, "Response struct is NULL");
    DBG_ASSERT_MSG(resp->content != NULL, "Response content is uninitialised");
    DBG_ASSERT_MSG(resp->contentLen > 0, "Response content len is zero");
    DBG_ASSERT_MSG(len <= HTTP_MAX_RESP_SIZE, "Length exceeds max size");


    const char* readPtr = buf;
    const char* end = readPtr + len;
    int left = len;

    BSD_DBG("Received response: %s", buf);
    BSD_DBG("length of buf: %d", strlen(buf));

    http_ParseState state = ParsingStatusLine;
    s64 bodyLen = -1;
    while (state != ParsingDone)
    {
        switch (state)
        {
            case ParsingStatusLine:
            {
                if (len < 4 || memcmp("HTTP", buf, 4) != 0)
                {
                    BSD_ERR("Received non-HTTP response!");
                    goto error;
                }
                char* nextLine = memchr(readPtr, '\n', left) + 1;
                if (nextLine >= end)
                {
                    BSD_ERR("Failed to find new line after status line");
                    goto error;
                }
                char* statusCodeStr = memchr(readPtr, ' ', left) + 1;
                if (statusCodeStr >= nextLine)
                {
                    BSD_ERR("Parsing error: Failed to find status code");
                    goto error;
                }

                char* endOfNumber;
                s64 statusCode = strtol(statusCodeStr, &endOfNumber, 10);

                if (statusCodeStr == endOfNumber)
                {
                    BSD_ERR("Status Code could not be parsed as a number");
                    goto error;
                }

                if (statusCode < 100 || statusCode > 511)
                {
                    BSD_ERR("Status Code out of range: %ld", statusCode);
                }

                resp->status = statusCode;
                left -= nextLine - readPtr;
                readPtr = nextLine;
                // TODO: if we have a status code that causes a possibly empty body we should
                // skip straight to the end
                state = ParsingHeaders;
            } break;
            case ParsingHeaders:
            {
                if (readPtr + 1 < end && *readPtr == '\r' && *(readPtr+1) == '\n')
                {
                    readPtr += 2;
                    left -= 2;
                    state = ParsingBody;
                    break;
                }
                char* nextLine = memchr(readPtr, '\n', left) + 1;
                if (NULL)
                {
                    BSD_ERR("Failed to find next line after header line");
                    goto error;
                }

                int lineLen = nextLine - readPtr;
                if (lineLen > sizeof("Content-Length:") &&
                    memcmp(readPtr, "Content-Length:", strlen("Content-Length:")) == 0)
                {
                    // found the content length field
                    char* valueStart = memchr(readPtr, ' ', lineLen) + 1;
                    if (valueStart >= nextLine)
                    {
                        BSD_ERR("Could not find the value for Content-Length field");
                        goto error;
                    }

                    char* endOfNumber;
                    bodyLen = strtol(valueStart, &endOfNumber, 10);
                    if (valueStart == endOfNumber)
                    {
                        BSD_ERR("Content-Length could not be parsed as a number");
                        goto error;
                    }
                    // WARNING: bad actor may overflow bodylen, but whatever we'll just let them figure
                    // figure out how they can exploit us
                }

                left -= nextLine - readPtr;
                readPtr = nextLine;
            } break;
            case ParsingBody:
            {
                if (bodyLen < 0)
                {
                    BSD_ERR("Received HTTP request with unknown body length");
                    goto error;
                }
                if (left >= resp->contentLen)
                {
                    BSD_ERR("The content of this response is too large");
                    goto error;
                }

                if (left < bodyLen)
                {
                    int gotLen = vos_Receive(sok, (char*)buf + len, bodyLen - left);
                    if (gotLen != bodyLen - left)
                    {
                        BSD_ERR("Could not recv the rest of the body, got len: %d", len);
                        goto error;
                    }
                }
                
                memcpy(resp->content, readPtr, bodyLen);
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
done:
    return 0;

error:
    return -1;
}