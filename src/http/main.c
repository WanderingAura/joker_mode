#include "based_logging.h"
#include "http.h"
#include "vos.h"
#include <string.h>
#include "stb_ds.h"

int main(void)
{
    bsd_SetLogLevel(bsd_LogLevel_Debug);

    http_Connection* conn = NULL;
    http_Error err = http_ConnectionCreate(&conn);
    if (err)
    {
        BSD_ERR("Failed to create connection, err: %d", err);
        return 1;
    }

    http_Request req = {0};
    strcpy(req.hostName, "192.168.1.7");
    strcpy(req.hostURL, "/v1/hiscores/get");
    req.method = http_MethodGET;
    req.port = 8080;

    http_Response resp = {0};

    err = http_ReqAndWaitForResp(conn, &req, &resp);
    if (err)
    {
        BSD_ERR("HTTP req failed %d", err);
        return 1;
    }

    BSD_INF("Got response, status: %d body: %.*s", resp.status, resp.content.len, resp.content.str);

    BSD_INF("=====headers======");
    for (u32 i = 0; i < shlen(resp.headers); i++)
    {
        BSD_INF("Key: %s, Value: %s", resp.headers[i].key, resp.headers[i].value);
    }


    // http_Response resp;
    // resp.content = malloc(4096);
    // resp.contentLen = 4096;
    // int ret = http_SendRequest(&req, &resp);

    // if (ret)
    // {
    //     BSD_ERR("HTTP request failed with ret %d", ret);
    //     return 1;
    // }

    // resp.content[resp.contentLen-1] = 0;
    // BSD_INF("Status: %d, body: %s", resp.status, resp.content);
    // free(resp.content);

    return 0;
}