#pragma once
#include "based_basic.h"

#include <stdbool.h>
#include "based_basic.h"


#if defined(__BYTE_ORDER__)
  #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define VOS_BIG_ENDIAN
  #else
    #define VOS_LITTLE_ENDIAN
  #endif
#elif defined(_MSC_VER) // it's safe to assume windows is always little endian
  #define VOS_LITTLE_ENDIAN
#else
  #error "Cannot determine endianess"
#endif

#ifdef VOS_LITTLE_ENDIAN
#define HTON16(x) \
  (u16)((((x) & 0x00ffu) << 8) | \
        (((x) & 0xff00u) >> 8) )

#define HTON32(x) \
  (u32)((((x) & 0x000000fful) << 24) | \
        (((x) & 0x0000ff00ul) << 8)  | \
        (((x) & 0x00ff0000ul) >> 8)  | \
        (((x) & 0xff000000ul) >> 24))

#else
#define HTON16(x)
#define HTON32(x)
#endif

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
#include <netinet/in.h>
#elif defined(_WIN32)
// stupid stuff we need to do because raylib has naming conflicts with win32 api
// (disables the symbols which conflict)
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#define NOSOUND
#pragma comment(lib, "Ws2_32.lib")
#include <WS2tcpip.h>
#include <winsock2.h>
typedef SOCKET vos_SocketID;
typedef int socklen_t;
#define VOS_INVALID_SOCKET INVALID_SOCKET
#else
#error "unknown OS"
#endif

vos_NetError vos_NetInit();
vos_SocketID vos_Socket(int domain, int type, int protocol);
vos_NetError vos_SocketClose(vos_SocketID sok);

s32 vos_Send(vos_SocketID sok, const void* buf, int len, int flags);
s32 vos_Recv(vos_SocketID sok, void* buf, int len, int flags);

vos_NetError vos_Connect(vos_SocketID sok, const struct sockaddr* addr, int addrlen);
vos_NetError vos_ConnectWithTimeout(vos_SocketID sok, const struct sockaddr* addr, int addrlen, u32 timeout_ms);

vos_NetError vos_SocketSetBlocking(vos_SocketID sok, bool blocking);
vos_NetError vos_GetNetError();
int vos_GetIPv4FromHost(const char* host, unsigned int* ipaddr);