#include "render_font_types.h"
#include "based_basic.h"
#include <assert.h>

static char* font_type_strs[FontTypeCount] =
{
    // Stringified names for the texture enum e.g. "GrassTexture"
#   define _FontType(type) STRINGIFY(type##FontType),
#   include "fonts.def"
#   undef _FontType
};

const char* FontTypeToString(rnd_FontType type)
{
    assert((u32)type < FontTypeCount);
    return font_type_strs[type];
}