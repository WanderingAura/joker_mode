#include "vos_socket.h"

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

typedef struct sockaddr sockaddr;
typedef int vos_SocketID;
#define vos_INVALID_SOCKET (-1)
#define vos_Poll(fd, num, timeout) poll(fd, num, timeout)
typedef struct pollfd vos_PollFD;
#elif defined(_WIN32)
typedef SOCKET vos_SocketID;
typedef WSAPOLLFD vos_PollFD;
#define vos_Poll(fd, num, timeout) WSAPoll(fd, num, timeout)
#define vos_INVALID_SOCKET INVALID_SOCKET
#else
#error "Unexpected OS"
#endif

vos_NetError vos_NetInit()
{
    vos_NetError rc = vos_NetErrorSuccess;
#ifdef _WIN32
    WSADATA wsaData;
    rc = WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
    return rc;
}

vos_NetError NetErrFromPlatformErr(int errCode)
{
    vos_NetError err = {0};
#ifdef __linux__
    switch (errCode)
    {
        case EWOULDBLOCK:
#if EWOULDBLOCK != EAGAIN
        case EAGAIN:
#endif
        {
            err = vos_NetErrorInProgress;
        }
        break;
        case EACCES:
        case EPERM:
        {
            err = vos_NetErrorPermDenied;
        }
        break;
        case ENOBUFS:
        case ENOMEM:
        {
            err = vos_NetErrorOutOfMemory;
        }
        break;
        case EAFNOSUPPORT:
        {
            err = vos_NetErrorUnsupported;
        }
        break;
        case EINVAL:
        {
            err = vos_NetErrorInvalidArg;
        }
        break;
        case EADDRINUSE:
        {
            err = vos_NetErrorAddressAlreadyInUse;
        }
        break;
        case EADDRNOTAVAIL:
        {
            err = vos_NetErrorAddressNotAvailable;
        }
        break;
        case ECONNREFUSED:
        {
            err = vos_NetErrorConnectionRefused;
        }
        break;
        default:
        {
            BSD_ERR("Unsupported linux socket error: %d", errCode);
            err = errCode;
        }
        break;
    }
#elif defined(_WIN32)
    switch (errCode)
    {
        case WSAEINPROGRESS:
        case WSAEWOULDBLOCK:
        {
            err = vos_NetErrorInProgress;
        }
        break;

        default:
        {
            BSD_ERR("Unsupported windows socket error: %d", errCode);
            err = errCode;
        }
        break;
    }
#endif
    return err;
}

vos_NetError vos_GetNetError()
{
    vos_NetError err = {0};
#ifdef __linux__
    int osErr = errno;
#elif defined(_WIN32)
    int osErr = WSAGetLastError();
#endif
    err = NetErrFromPlatformErr(osErr);
    return err;
}

vos_SocketID vos_Socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

s32 vos_Send(vos_SocketID sok, const void* buf, int len, int flags)
{
    int sentLen = send(sok, buf, len, flags);
    if (sentLen < 0)
    {
        return -vos_GetNetError();
    }
    return sentLen;
}

s32 vos_Recv(vos_SocketID sok, void* buf, int len, int flags)
{
    int recvLen = recv(sok, buf, len, flags);
    if (recvLen < 0)
    {
        return -vos_GetNetError();
    }
    return recvLen;
}

vos_NetError vos_SocketSetBlocking(vos_SocketID sok, bool blocking)
{
    int rc;
#ifdef __linux__
    int flags = fcntl(sok, F_GETFL, 0);
    if (flags == -1) return -1;

    if (blocking)
    {
        flags &= ~O_NONBLOCK;
    }
    else
    {
        flags |= O_NONBLOCK;
    }

    rc = fcntl(sok, F_SETFL, flags);
#elif defined(_WIN32)
    u_long mode = blocking ? 0UL : 1UL;
    rc = ioctlsocket(sok, FIONBIO, &mode);
#endif

    if (rc != 0)
    {
        return vos_GetNetError();
    }
    return vos_NetErrorSuccess;
}

vos_NetError vos_Connect(vos_SocketID sok, const struct sockaddr* addr, int addrlen)
{
    int rc = connect(sok, addr, addrlen);
    if (rc != 0)
    {
        return vos_GetNetError();
    }
    return vos_NetErrorSuccess;
}


vos_NetError vos_ConnectWithTimeout(vos_SocketID sok, const struct sockaddr* addr, int addrlen, u32 timeout_ms)
{
    DBG_ASSERT_MSG(timeout_ms < 30000, "Timeout too high");

    // TODO: check if the socket is already non-blocking

    int rc = vos_SocketSetBlocking(sok, false);
    if (rc != 0)
    {
        return rc;
    }

    rc = connect(sok, addr, addrlen);
    if (rc == 0)
    {
        rc = vos_SocketSetBlocking(sok, true);
        if (rc != 0)
        {
            return vos_GetNetError();
        }
        return 0;
    }

    vos_NetError err = vos_GetNetError();
    if (err != vos_NetErrorInProgress)
    {
        vos_SocketSetBlocking(sok, true);
        return err;
    }

    vos_PollFD pfd = {0};
    pfd.fd = sok;
    pfd.events = POLLOUT;

    rc = vos_Poll(&pfd, 1, timeout_ms);
    if (rc == 0)
    {
        vos_SocketSetBlocking(sok, true);
        return vos_NetErrorTimedOut;
    }

    if (rc < 0)
    {
        vos_NetError err = vos_GetNetError();
        vos_SocketSetBlocking(sok, true);
        return err;
    }

    int soError = 0;
    socklen_t soErrorLen = sizeof(soError);
    if (getsockopt(sok, SOL_SOCKET, SO_ERROR, (void*)&soError, &soErrorLen) < 0) // void* is for windows being windows
    {
        vos_NetError err = vos_GetNetError();
        vos_SocketSetBlocking(sok, true);
        return err;
    }

    if (soError != 0)
    {
        vos_SocketSetBlocking(sok, true);
        return NetErrFromPlatformErr(soError);
    }

    err = vos_SocketSetBlocking(sok, true);
    if (err != vos_NetErrorSuccess)
    {
        return err;
    }
    return vos_NetErrorSuccess;
}

vos_NetError vos_SocketClose(vos_SocketID sok)
{
    int rc;
#ifdef _WIN32
    rc = closesocket(sok);
#elif defined(__linux__)
    rc = close(sok);
#endif

    if (rc != 0)
    {
        return vos_GetNetError();
    }
    return vos_NetErrorSuccess;
}

int vos_GetIPv4FromHost(const char* host, unsigned int* ipaddr)
{
    struct addrinfo* res;
    struct addrinfo hints = {0};
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