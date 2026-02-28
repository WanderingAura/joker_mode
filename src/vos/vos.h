#pragma once
#include "based_basic.h"

typedef void* vos_DLLFuncPtr;
typedef void* vos_DLLHandle;

#if defined(_WIN32)
  #define vos_DLL_EXTENSION "dll"
  #define vos_DLL_PREFIX ""
#elif defined(__linux__)
  #define vos_DLL_EXTENSION "so"
  #define vos_DLL_PREFIX "lib"
#else
  #error "You are building for an unsupported operating system!"
#endif

#define vos_DLL_ACTIVE_SUFFIX "_active." vos_DLL_EXTENSION
#define FILE_NAME_LEN_MAX 128

// copies the DLL to a temporary file and loads it. this is to allow the original DLL to be replaced
// while the game is running, allowing hot reloading.
vos_DLLHandle vos_DLLLoad(const char* path);

s64 vos_DLLUnload(vos_DLLHandle handle);

vos_DLLFuncPtr vos_DLLGetFunc(vos_DLLHandle handle, const char* funcName);

void GetTmpDLLName(char* dest, const char* file);

#if defined(__BYTE_ORDER__)
  #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define VOS_BIG_ENDIAN
  #else
    #define VOS_LITTLE_ENDIAN
  #endif
#else
  #error "Cannot determine endianess"
#endif

#define HTON16(x) \
  (u16)((((x) & 0x00ffu) << 8) | \
        (((x) & 0xff00u) >> 8) )

#define HTON32(x) \
  (u32)((((x) & 0x000000fful) << 24) | \
        (((x) & 0x0000ff00ul) << 8)  | \
        (((x) & 0x00ff0000ul) >> 8)  | \
        (((x) & 0xff000000ul) >> 24))

// typedef enum
// {
//   vos_SocketTCP,
//   vos_SocketUDP
// } vos_SocketType;

// typedef enum
// {
//   vos_IPv4,
//   vos_IPv6,
// } vos_IPVersion;

// typedef enum
// {
//   vos_SocketModeBlocking,
//   vos_SocketModeNonBlocking,
// } vos_SocketMode;

// typedef struct
// {
//   vos_SocketType type;
//   vos_IPVersion ipver;
//   vos_SocketMode mode;
// } vos_SocketInfo;

// vos_SocketHandle vos_Socket(vos_SocketInfo* info);
// int vos_Connect(vos_SocketHandle sok, u32 ipaddr, u16 port);
// int vos_Send(vos_SocketHandle sok, void* buf, size_t len);
// int vos_Receive(vos_SocketHandle sok, void* buf, size_t maxlen);
// int vos_GetIPv4FromHost(const char* host, unsigned int* ipaddr);
// int vos_SocketClose(vos_SocketHandle sok);