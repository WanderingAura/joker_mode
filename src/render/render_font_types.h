#pragma once
typedef enum {
#   define _FontType(type) FontType##type,
#   include "fonts.def"
#   undef _FontType
    FontTypeCount,
} rnd_FontType;

const char* FontTypeToString(rnd_FontType type);