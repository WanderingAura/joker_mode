#include <stdbool.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#undef STB_DS_IMPLEMENTATION

bool bsd_IsDigit(char c)
{
    return c >= '0' && c <= '9';
}