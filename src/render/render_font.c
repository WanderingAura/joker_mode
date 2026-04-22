#include <raylib.h>

#include "based_basic.h"

#include "render_font_types.h"

void rnd_FontInit(Font* fonts)
{
    // Unload all fonts that are currently on the GPU, otherwise we'll have a GPU memory leak
    for (u32 i = 0; i < FontTypeCount; i++)
    {
        if (IsFontValid(fonts[i]))
        {
            BSD_DBG("Unloading texture ID %d (%s)", fonts[i].texture.id, FontTypeToString(i));
            UnloadFont(fonts[i]);
        }
    }

    fonts[FontTypeTitle] = LoadFont("assets/fonts/Sekuya-Regular.ttf");
}