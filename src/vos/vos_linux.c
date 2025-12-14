#ifndef __linux__
  #error "This file should only be compiled on linux
#endif
#include <dlfcn.h>
#include <raylib.h>
#include "vos.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define COPY_COMMAND_LEN_MAX (FILE_NAME_LEN_MAX * 3)
#define COPY_COMMAND_BYTES 4

vos_DLLHandle vos_DLLLoad(const char* file)
{
    const char* fileName = GetFileNameWithoutExt(file);
    assert(strlen(fileName) < FILE_NAME_LEN_MAX);

    char tmpFile[FILE_NAME_LEN_MAX];
    int n = sprintf(tmpFile, "%s" vos_DLL_ACTIVE_SUFFIX, fileName);
    assert((size_t)n == strlen(fileName) + strlen(vos_DLL_ACTIVE_SUFFIX));

    char copyCommand[COPY_COMMAND_LEN_MAX];
    n = sprintf(copyCommand, "cp %s %s", file, tmpFile);
    assert((size_t)n == strlen(file) + strlen(tmpFile) + COPY_COMMAND_BYTES);

    int ret = system(copyCommand);
    if (ret == -1)
    {
        int err = errno;
        printf("Failed to copy DLL: %s\n", strerror(err));
        return NULL;
    }
    else if (ret != 0)
    {
        printf("Copy command failed. Child exited with status %d\n", ret);
        return NULL;
    }

    // TODO: should we use RTLD_NOW or RTLD_LAZY? i'm thinking it doesn't make a difference
    // because we will only be resolving like 5 symbols, but RTLD_NOW will crash early if
    // there's a problem so i'm using that for now.
    vos_DLLHandle handle = dlopen(tmpFile, RTLD_NOW);
    if (handle == NULL)
    {
        printf("An error occured while loading the %s library: %s\n", file, dlerror());
    }
    return handle;
}

s64 vos_DLLUnload(vos_DLLHandle handle)
{
    int ret = dlclose(handle);
    if (ret != 0)
    {
        printf("An error occurred while unloading library: %s\n", dlerror());
    }
    return ret;
}

vos_DLLFuncPtr vos_DLLGetFunc(vos_DLLHandle handle, const char* funcName)
{
    vos_DLLFuncPtr ptr = dlsym(handle, funcName);
    if (ptr == NULL)
    {
        printf("Getting library function %s failed: %s\n", funcName, dlerror());
    }
    return ptr;
}