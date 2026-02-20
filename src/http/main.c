#include "based_logging.h"
#include "http.h"
#include "vos.h"
#include <string.h>

int main(void)
{
    bsd_SetLogLevel(bsd_LogLevel_Debug);
    http_Request req = {};
    strcpy(req.hostName, "192.168.1.7");
    strcpy(req.hostURL, "/v1/hiscores/get");
    req.method = http_MethodGET;
    req.port = 8080;
    
    http_Response resp;
    resp.content = malloc(4096);
    resp.contentLen = 4096;
    int ret = http_SendRequest(&req, &resp);

    if (ret)
    {
        BSD_ERR("HTTP request failed with ret %d", ret);
        return 1;
    }

    resp.content[resp.contentLen-1] = 0;
    BSD_INF("Status: %d, body: %s", resp.status, resp.content);
    free(resp.content);

    return 0;
}