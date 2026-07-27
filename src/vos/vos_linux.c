#include <stdbool.h>
#include "based_basic.h"
#include "based_core.h"
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
#include <sys/sysinfo.h>
#include <sys/mman.h>
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

vos_SystemInfo* vos_GetSystemInfo()
{
    static vos_SystemInfo* infoptr;
    static vos_SystemInfo info;
    if (infoptr == NULL)
    {
        infoptr = &info;
        infoptr->logical_processor_count = get_nprocs();
        infoptr->page_size = getpagesize();
        infoptr->large_page_size = MB(2);
        infoptr->allocation_granularity = infoptr->page_size;
    }
    return infoptr;
}

void* vos_ReserveMemoryLarge(u64 size)
{
    void* mem = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    if (mem == MAP_FAILED)
    {
        mem = 0;
    }
    return mem;
}

void* vos_ReserveMemory(u64 size)
{
    void* mem = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED)
    {
        mem = 0;
    }
    return mem;
}

b32 vos_CommitMemory(void* ptr, u64 size)
{
    mprotect(ptr, size, PROT_READ | PROT_WRITE);
    return true;
}

b32 vos_CommitMemoryLarge(void* ptr, u64 size)
{
    mprotect(ptr, size, PROT_READ | PROT_WRITE);
    return true;
}

void vos_ReleaseMemory(void *ptr, u64 size)
{
    munmap(ptr, size);
}

