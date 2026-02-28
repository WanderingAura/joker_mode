#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include "based_basic.h"
#ifndef __linux__
  #error This file should only be compiled on linux
#endif
#include <dlfcn.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <error.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include "vos.h"
#include "based_logging.h"

#define BLOCKING_SOCKET_TIMEOUT_MS 2000L
#define COPY_COMMAND_LEN_MAX (FILE_NAME_LEN_MAX * 3)
#define COPY_COMMAND_BYTES 4

vos_DLLHandle vos_DLLLoad(const char* file)
{
    char tmpFile[FILE_NAME_LEN_MAX];
    GetTmpDLLName(tmpFile, file);

    char copyCommand[COPY_COMMAND_LEN_MAX];
    int n = sprintf(copyCommand, "cp %s %s", file, tmpFile);
    assert((size_t)n == strlen(file) + strlen(tmpFile) + COPY_COMMAND_BYTES);

    int ret = system(copyCommand);
    if (ret == -1)
    {
        int err = errno;
        BSD_ERR("Failed to copy DLL: %s\n", strerror(err));
        return NULL;
    }
    else if (ret != 0)
    {
        BSD_ERR("Copy command failed. Child exited with status %d\n", ret);
        return NULL;
    }

    // TODO: should we use RTLD_NOW or RTLD_LAZY? i'm thinking it doesn't make a difference
    // because we will only be resolving like 5 symbols, but RTLD_NOW will crash early if
    // there's a problem so i'm using that for now.
    vos_DLLHandle handle = dlopen(tmpFile, RTLD_NOW);
    if (handle == NULL)
    {
        BSD_ERR("An error occured while loading the %s library: %s\n", file, dlerror());
    }
    return handle;
}

s64 vos_DLLUnload(vos_DLLHandle handle)
{
    int ret = dlclose(handle);
    if (ret != 0)
    {
        BSD_ERR("An error occurred while unloading library: %s\n", dlerror());
    }
    return ret;
}

vos_DLLFuncPtr vos_DLLGetFunc(vos_DLLHandle handle, const char* funcName)
{
    vos_DLLFuncPtr ptr = dlsym(handle, funcName);
    if (ptr == NULL)
    {
        BSD_ERR("Getting library function %s failed: %s\n", funcName, dlerror());
    }
    return ptr;
}

#if 0
vos_SocketHandle vos_Socket(vos_SocketInfo* info)
{
    int domain;
    if (info->ipver == vos_IPv4)
    {
        domain = AF_INET;
    }
    else if (info->ipver == vos_IPv6)
    {
        domain = AF_INET6;
    }
    else
    {
        DBG_ASSERT_MSG(false, "Unknown IP type %d", info->ipver);
    }
    
    int protocolType = SOCK_NONBLOCK * (info->mode == vos_SocketModeNonBlocking);
    if (info->type == vos_SocketTCP)
    {
        protocolType |= SOCK_STREAM;
    }
    else if (info->type == vos_SocketUDP)
    {
        DBG_ASSERT_MSG(false, "only tcp for now");
        protocolType |= SOCK_DGRAM;
    }
    else
    {
        DBG_ASSERT_MSG(false, "Unknown socket type %d", info->type);
    }

    int sfd = socket(domain, protocolType, 0);
    if (sfd == -1)
    {
        int err = errno;
        BSD_ERR("Failed to create socket: %s", strerror(err));
        return sfd;
    }

    if (info->mode == vos_SocketModeBlocking)
    {
        BSD_DBG("Creating blocking socket");
        struct timeval tv;
        tv.tv_sec = BLOCKING_SOCKET_TIMEOUT_MS / 1000;
        tv.tv_usec = (BLOCKING_SOCKET_TIMEOUT_MS % 1000) * 1000;
        int ret = setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        ret |= setsockopt(sfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        if (ret)
        {
            BSD_ERR("setsockopt failed");
        }
    }

    DBG_ASSERT_MSG(info->ipver == vos_IPv4, "Currently only supports IPv4");
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;

    int err = bind(sfd, (struct sockaddr*)&addr, sizeof(addr));
    if (err)
    {
        int err = errno;
        BSD_ERR("Failed to bind socket: %s", strerror(err));
        goto cleanup;
    }

    return sfd;
cleanup:
    err = close(sfd);
    if (err)
    {
        int err = errno;
        BSD_ERR("Failed to close socket: %s", strerror(err));
    }
    return vos_INVALID_SOCKET_HANDLE;
}

int vos_SocketClose(vos_SocketHandle sok)
{
    int err = close(sok);
    if (err)
    {
        int err = errno;
        BSD_ERR("Failed to close socket: %s", strerror(err));
    }
    return err;
}

int vos_Connect(vos_SocketHandle sok, u32 ipaddr, u16 port)
{
    DBG_ASSERT_MSG(sok != vos_INVALID_SOCKET_HANDLE, "Invalid socket handle");
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ipaddr;
    addr.sin_port = port;

    int flags = fcntl(sok, F_GETFL, 0);
    fcntl(sok, F_SETFL, flags | O_NONBLOCK);

    int err = connect(sok, (struct sockaddr*)&addr, sizeof(addr));
    if (!err)
    {
        fcntl(sok, F_SETFL, flags);
        return 0;
    }

    if (errno != EINPROGRESS)
    {
        int err = errno;
        BSD_ERR("Failed to connect: %s", strerror(err));
        return -1;
    }

    fd_set wfds;

    FD_ZERO(&wfds);
    FD_SET(sok, &wfds);

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    int r = select(sok + 1, NULL, &wfds, NULL, &tv);

    if (r == 0) {
        // ⏱ timeout
        errno = ETIMEDOUT;
        BSD_ERR("Connection failed: %s", strerror(ETIMEDOUT));
        return -1;
    }

    if (r < 0) {
        BSD_ERR("select failed");
        return -1;
    }

    // 3️⃣ check connection result
    int so_error = 0;
    socklen_t len = sizeof(so_error);

    if (getsockopt(sok, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
        BSD_ERR("getsockopt failed");
        return -1;
    }

    if (so_error != 0) {
        errno = so_error;
        BSD_ERR("so error");
        return -1;
    }
    struct sockaddr_storage peer;
    socklen_t lenx = sizeof(peer);

    errno = 0;
    int ra = getpeername(sok, (struct sockaddr*)&peer, &lenx);
    printf("getpeername = %d errno=%d\n", ra, errno);
    fcntl(sok, F_SETFL, flags);
    int f = fcntl(sok, F_GETFL, 0);
    printf("flags after connect: 0x%x\n", f);
    BSD_DBG("Connection success socket: %d", sok);
    return 0;
}

int vos_Send(vos_SocketHandle sok, void* buf, size_t len)
{
    struct sockaddr_storage peer;
    socklen_t lenx = sizeof(peer);

    int r = getpeername(sok, (struct sockaddr*)&peer, &lenx);
    printf("getpeername = %d errno=%d\n", r, errno);
    DBG_ASSERT_MSG(sok != vos_INVALID_SOCKET_HANDLE, "invalid socket");
    BSD_DBG("Sending on socket: %d", sok);
    errno = 0;
    int bytesSent = send(sok, buf, len, 0);
    if (bytesSent < len)
    {
        int err = errno;
        BSD_ERR("Failed to send: %s", strerror(err));
        return -1;
    }
    return bytesSent; 
}

int vos_Receive(vos_SocketHandle sok, void* buf, size_t maxlen)
{
    DBG_ASSERT_MSG(sok != vos_INVALID_SOCKET_HANDLE, "invalid socket");

    int len = recv(sok, buf, maxlen, 0);
    if (len <= 0)
    {
        int err = errno;
        BSD_ERR("Failed to receive: %s", strerror(err));
    }
    return len;
}

int vos_GetIPv4FromHost(const char* host, unsigned int* ipaddr)
{
    struct addrinfo* res;
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(host, NULL, &hints, &res);
    if (err)
    {
        BSD_ERR("Get IPv4 addr failed for host %s: %s", host, gai_strerror(err));
        return -1;
    }

    if (res->ai_family == AF_INET)
    {
        struct sockaddr_in *ipv4 = (struct sockaddr_in*)res->ai_addr;
        *ipaddr = ipv4->sin_addr.s_addr;
    }
    else
    {
        BSD_ERR("No IPv4 address found for host %s", host);
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return 0;
}
#endif