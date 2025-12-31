#include "core_texture_types.h"
#include "based_basic.h"
#include <assert.h>

static char* texture_type_strs[TextureTypeCount] =
{
    // Stringified names for the texture enum e.g. "GrassTexture"
#   define _TextureType(type) STRINGIFY(type##Texture),
#   include "textures.def"
#   undef _TextureType
};

const char* TextureTypeToString(TextureType type)
{
    assert((u32)type < TextureTypeCount);
    return texture_type_strs[type];
}