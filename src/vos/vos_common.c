#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "vos.h"
#include "raylib.h"

void GetTmpDLLName(char* dest, const char* file)
{
    const char* fileName = GetFileNameWithoutExt(file);
    assert(strlen(fileName) < FILE_NAME_LEN_MAX);

    int n = sprintf(dest, "%s" vos_DLL_ACTIVE_SUFFIX, fileName);
    assert(n < FILE_NAME_LEN_MAX);
    assert((size_t)n == strlen(fileName) + strlen(vos_DLL_ACTIVE_SUFFIX));
}