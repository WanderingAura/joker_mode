#ifndef _WIN32
  #error This file should only be compiled on Windows
#endif

#include <windows.h>
#include <stdbool.h>

#include "vos.h"
#include "based_logging.h"

#pragma comment(lib, "Advapi32.lib")

#define WINDOWS_ERROR_MESSAGE_LEN_MAX 256

static void WIN32_GetLastErrorString(char* errMsg)
{
  DWORD errCode = GetLastError();
  DWORD ret = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
      errCode, 0, errMsg, WINDOWS_ERROR_MESSAGE_LEN_MAX, NULL);
  
  if (ret == 0)
  {
    BSD_ERR("Formatting message for error code %d failed\n", errCode);
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
    BSD_DBG("trying to copy file\n");
    Sleep(5);
  } while (!success && err == ERROR_SHARING_VIOLATION);
  if (!success)
  {
    WIN32_GetLastErrorString(errMsg);
    BSD_ERR("CopyFile failed: %s %d\n", errMsg, err);
    return NULL;
  }

  vos_DLLHandle handle = LoadLibraryA(tmpFile);
  if (!handle)
  {
    WIN32_GetLastErrorString(errMsg);
    BSD_ERR("LoadLibrary failed: %s\n", errMsg);
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
    BSD_ERR("An error occurred while unloading library: %s\n", errMsg);
    return 1;
  }
  return 0;
}

vos_DLLFuncPtr vos_DLLGetFunc(vos_DLLHandle handle, const char* funcName)
{
  char errMsg[WINDOWS_ERROR_MESSAGE_LEN_MAX] = {0};
  vos_DLLFuncPtr ptr = (vos_DLLFuncPtr)GetProcAddress(handle, funcName);

  if (!ptr)
  {
    WIN32_GetLastErrorString(errMsg);
    BSD_ERR("Getting library function %s failed: %s\n", funcName, errMsg);
  }
  return ptr;
}

// large pages require the process token to hold SeLockMemoryPrivilege, which is
// disabled by default even when the account has been granted the "Lock pages in
// memory" right. try to enable it once; if it fails, large-page allocation will
// fail too and callers fall back the same way they would on a failed mmap(MAP_HUGETLB).
static bool WIN32_TryEnableLockMemoryPrivilege()
{
  static bool tried = false;
  static bool enabled = false;
  if (tried)
  {
    return enabled;
  }
  tried = true;

  HANDLE token;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
  {
    return false;
  }

  TOKEN_PRIVILEGES priv = {0};
  priv.PrivilegeCount = 1;
  priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  if (!LookupPrivilegeValueA(NULL, SE_LOCK_MEMORY_NAME, &priv.Privileges[0].Luid))
  {
    CloseHandle(token);
    return false;
  }

  bool success = AdjustTokenPrivileges(token, false, &priv, 0, NULL, NULL) && GetLastError() == ERROR_SUCCESS;
  CloseHandle(token);

  enabled = success;
  return enabled;
}

vos_SystemInfo* vos_GetSystemInfo()
{
  static vos_SystemInfo* infoptr;
  static vos_SystemInfo info;
  if (infoptr == NULL)
  {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    infoptr = &info;
    infoptr->logical_processor_count = sysInfo.dwNumberOfProcessors;
    infoptr->page_size = sysInfo.dwPageSize;
    infoptr->large_page_size = GetLargePageMinimum();
    infoptr->allocation_granularity = sysInfo.dwAllocationGranularity;
  }
  return infoptr;
}

void* vos_ReserveMemoryLarge(u64 size)
{
  char errMsg[WINDOWS_ERROR_MESSAGE_LEN_MAX] = {0};

  if (!WIN32_TryEnableLockMemoryPrivilege())
  {
    BSD_ERR("Failed to enable SeLockMemoryPrivilege; large pages require the "
        "'Lock pages in memory' user right\n");
    return NULL;
  }

  // large pages must be reserved and committed in a single call on Windows,
  // unlike regular pages which can be reserved and committed separately.
  void* mem = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
  if (mem == NULL)
  {
    WIN32_GetLastErrorString(errMsg);
    BSD_ERR("VirtualAlloc (large pages) failed: %s\n", errMsg);
  }
  return mem;
}

void* vos_ReserveMemory(u64 size)
{
  char errMsg[WINDOWS_ERROR_MESSAGE_LEN_MAX] = {0};

  void* mem = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
  if (mem == NULL)
  {
    WIN32_GetLastErrorString(errMsg);
    BSD_ERR("VirtualAlloc (reserve) failed: %s\n", errMsg);
  }
  return mem;
}

b32 vos_CommitMemory(void* ptr, u64 size)
{
  char errMsg[WINDOWS_ERROR_MESSAGE_LEN_MAX] = {0};

  void* result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
  if (result == NULL)
  {
    WIN32_GetLastErrorString(errMsg);
    BSD_ERR("VirtualAlloc (commit) failed: %s\n", errMsg);
    return false;
  }
  return true;
}

b32 vos_CommitMemoryLarge(void* ptr, u64 size)
{
  // no-op: vos_ReserveMemoryLarge already reserves and commits the full
  // region up front, since Windows requires MEM_LARGE_PAGES allocations to
  // be committed at reserve time.
  (void)ptr;
  (void)size;
  return true;
}

void vos_ReleaseMemory(void* ptr, u64 size)
{
  char errMsg[WINDOWS_ERROR_MESSAGE_LEN_MAX] = {0};

  // MEM_RELEASE requires size to be 0; it always frees the entire region
  // that was originally reserved.
  (void)size;
  if (!VirtualFree(ptr, 0, MEM_RELEASE))
  {
    WIN32_GetLastErrorString(errMsg);
    BSD_ERR("VirtualFree failed: %s\n", errMsg);
  }
}