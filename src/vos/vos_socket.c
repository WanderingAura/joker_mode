typedef enum
{
    vos_NetErrorSuccess = 0,
    vos_NetErrorInternal,
    vos_NetErrorInvalidArg,
    vos_NetErrorAPIBusy,

} vos_NetError;

#ifdef __linux__
#include <sys/socket.h>
#include <unistd.h>

typedef struct sockaddr sockaddr;
typedef int vos_SocketID;
#define vos_INVALID_SOCKET (-1)
#elif defined(_WIN32)
#include <winsock2.h>
typedef SOCKET vos_SocketID;
#define vos_INVALID_SOCKET INVALID_SOCKET
#else
#error "Unexpected OS"
#endif

// TODO: maybe we should have two different implementations of this...
int vos_NetInit()
{
    int ret = 0;
#ifdef _WIN32
    WSADATA wsaData;
    ret = WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
    return ret;
}

vos_SocketID vos_Socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

int vos_Send(vos_SocketID sok, const void* buf, int len, int flags)
{
    return send(sok, buf, len, flags);
}

int vos_Recv(vos_SocketID sok, void* buf, int len, int flags)
{
    return recv(sok, buf, len, flags);
}

int vos_Connect(vos_SocketID sok, const sockaddr* addr, int addrlen)
{
    return connect(sok, addr, addrlen);
}

int vos_SocketClose(vos_SocketID sok)
{
#ifdef _WIN32
    return closesocket(sok);
#elif defined(__linux__)
    return close(sok);
#endif
}