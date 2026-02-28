#pragma once
#include "based_basic.h"

#include <stdbool.h>
#include "based_basic.h"

typedef enum
{
    vos_NetErrorSuccess = 0,
    vos_NetErrorInternal,
    vos_NetErrorInvalidArg,
    vos_NetErrorAPIBusy,
    vos_NetErrorInProgress,
    vos_NetErrorPermDenied,
    vos_NetErrorOutOfMemory,
    vos_NetErrorUnsupported,
    vos_NetErrorAddressAlreadyInUse,
    vos_NetErrorAddressNotAvailable,
    vos_NetErrorConnectionRefused,
    vos_NetErrorTimedOut,
} vos_NetError;

#ifdef __linux__
typedef int vos_SocketID;
#define VOS_INVALID_SOCKET (-1)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#elif defined(_WIN32)
#include <winsock2.h>
typedef SOCKET vos_SocketID;
#else
#error "unknown OS"
#endif

vos_SocketID vos_Socket(int domain, int type, int protocol);
vos_NetError vos_SocketClose(vos_SocketID sok);

s32 vos_Send(vos_SocketID sok, const void* buf, int len, int flags);
s32 vos_Recv(vos_SocketID sok, void* buf, int len, int flags);

vos_NetError vos_Connect(vos_SocketID sok, const struct sockaddr* addr, int addrlen);
vos_NetError vos_ConnectWithTimeout(vos_SocketID sok, const struct sockaddr* addr, int addrlen, u32 timeout_ms);

vos_NetError vos_SocketSetBlocking(vos_SocketID sok, bool blocking);
vos_NetError vos_GetNetError();
int vos_GetIPv4FromHost(const char* host, unsigned int* ipaddr);