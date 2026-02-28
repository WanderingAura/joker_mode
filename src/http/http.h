#include "based_basic.h"
#include "vos_socket.h"

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

typedef enum
{
    http_Success,
    http_SocketFailure
} http_Error;


typedef struct
{
    u8* recvBuf;
    u32 cap;
    u32 len;
    u32 pos;
    vos_SocketID sok;
} http_Connection;

int http_SendRequest(http_Connection* conn, const http_Request* req, http_Response* resp);
int http_ParseResponse(vos_SocketID sok, char* buf, int len, http_Response* resp);