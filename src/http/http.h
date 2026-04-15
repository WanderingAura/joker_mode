#pragma once
#include "based_basic.h"
#include "vos_socket.h"

#define HTTP_MAX_REQ_SIZE 4096
#define HTTP_MAX_RESP_SIZE HTTP_MAX_REQ_SIZE
#define HTTP_MAX_HOST_NAME 64
#define HTTP_MAX_HOST_URL 64

typedef struct
{
    char* str;
    u32 len;
} http_String;

typedef struct
{
    char* key;
    char* value;
} http_Header;

typedef struct
{
    int status;
    http_String content;
    http_Header* headers;
} http_Response;


typedef enum
{
    http_MethodGET,
    http_MethodPOST,
    http_MethodNum,
} http_Method;

typedef struct
{
    char hostName[HTTP_MAX_HOST_NAME];
    char hostURL[HTTP_MAX_HOST_URL];
    u16 port;
    http_Method method;
    http_String body;
} http_Request;

typedef enum
{
    http_Success,
    http_SocketFailure,
    http_ReceiveFailed,
    http_InvalidHeaderLine,
    http_FailedToSendReq,
    http_HeaderLineTooLong,
    http_ParsingFailed,
} http_Error;

typedef struct
{
    u8* data;
    u32 cap;
    u32 len;
    u32 pos;
} ReceiveBuffer;

typedef struct
{
    ReceiveBuffer buf;
    vos_SocketID sok;
} http_Connection;

int http_ReqAndWaitForResp(http_Connection* conn, const http_Request* req, http_Response* resp);
http_Error http_ConnectionCreate(http_Connection** connection);
void http_ResponseFree(http_Response* resp);
void http_ConnectionClose(http_Connection* connection);