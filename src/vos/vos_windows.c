#ifndef _WIN32
  #error This file should only be compiled on Windows
#endif

#include <windows.h>
#include <stdbool.h>

#include "vos.h"

#define WINDOWS_ERROR_MESSAGE_LEN_MAX 256

static void WIN32_GetLastErrorString(char* errMsg)
{
  DWORD errCode = GetLastError();
  DWORD ret = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
      errCode, 0, errMsg, WINDOWS_ERROR_MESSAGE_LEN_MAX, NULL);
  
  if (ret == 0)
  {
    printf("Formatting message for error code %d failed\n", errCode);
    errMsg[0] = 0;
  }
}

vos_DLLHandle vos_DLLLoad(const char* file)
{
  char errMsg[WINDOWS_ERROR_MESSAGE_LEN_MAX] = {0};

  char tmpFile[FILE_NAME_LEN_MAX];
  GetTmpDLLName(tmpFile, file);

  DWORD err;
  bool success;
  // TODO: this do while is a crappy workaround for windows still holding the lock
  // to the DLL for a while (~20ms) even after the compiler has finished modifying it.
  // so we just keep copying until it's successful. Note that this seems to only
  // happen with the Ninja generator.
  // a possible better fix would be to create a function IsDLLReady() and only call DLLLoad() when
  // the DLL has 1. been modified and 2. is ready to be copied.
  do {
    success = CopyFileA(file, tmpFile, false);
    err = GetLastError();
    printf("trying to copy file\n");
    Sleep(5);
  } while (!success && err == ERROR_SHARING_VIOLATION);
  if (!success)
  {
    WIN32_GetLastErrorString(errMsg);
    printf("CopyFile failed: %s %d\n", errMsg, err);
    return NULL;
  }

  vos_DLLHandle handle = LoadLibraryA(tmpFile);
  if (!handle)
  {
    WIN32_GetLastErrorString(errMsg);
    printf("LoadLibrary failed: %s\n", errMsg);
  }

  return handle;
}

s64 vos_DLLUnload(vos_DLLHandle handle)
{
  char errMsg[WINDOWS_ERROR_MESSAGE_LEN_MAX] = {0};
  bool success = FreeLibrary(handle);
  if (!success)
  {
    WIN32_GetLastErrorString(errMsg);
    printf("An error occurred while unloading library: %s\n", errMsg);
    return 1;
  }
  return 0;
}

vos_DLLFuncPtr vos_DLLGetFunc(vos_DLLHandle handle, const char* funcName)
{
  char errMsg[WINDOWS_ERROR_MESSAGE_LEN_MAX] = {0};
  vos_DLLFuncPtr ptr = GetProcAddress(handle, funcName);

  if (!ptr)
  {
    WIN32_GetLastErrorString(errMsg);
    printf("Getting library function %s failed: %s\n", funcName, errMsg);
  }
  return ptr;
}