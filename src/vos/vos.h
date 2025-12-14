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