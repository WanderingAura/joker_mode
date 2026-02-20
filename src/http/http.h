#include "based_basic.h"
#include "vos.h"

#define HTTP_MAX_REQ_SIZE 4096
#define HTTP_MAX_RESP_SIZE HTTP_MAX_REQ_SIZE

typedef struct
{
    int status;
    char* content;
    int contentLen;
} http_Response;

typedef enum : u32
{
    http_MethodGET,
    http_MethodPOST,
    http_MethodNum,
} http_Method;

typedef struct
{
    char hostName[64];
    char hostURL[64];
    u16 port;
    http_Method method;
    char* body;
} http_Request;

int http_SendRequest(const http_Request* req, http_Response* resp);
int http_ParseResponse(vos_SocketHandle sok, char* buf, int len, http_Response* resp);