#include <math.h>
#include "based_basic.h"
#include "raylib.h"
#include "based_raylib.h"

static inline f32 AngularFreq(f32 freq)
{
    return 2.0f * PI * freq;
}

Color PeriodicFade_(Color c, PeriodicFadeParams params)
{
    DBG_ASSERT_MSG(params.low >= 0 && params.low <= 1, "invalid params");
    DBG_ASSERT_MSG(params.high >= 0 && params.high <= 1, "invalid params");
    DBG_ASSERT_MSG(params.low < params.high, "invalid params");

    f32 midpoint = (params.high + params.low) / 2.0f;
    f32 half_dist = (params.high - params.low) / 2.0f;

    f32 alpha = midpoint + (half_dist * cosf(AngularFreq(params.freq) * GetTime()));

    return Fade(c, alpha);
}

f32 CalculateCentredPosition(f32 low, f32 high, f32 size)
{
    DBG_ASSERT_MSG(high - low >= size, "the object size must be smaller than the bounds");

    f32 centre = (low + high) / 2;
    return centre - size / 2;
}

void DrawHCentreText(const char* text, f32 y, f32 fontSize, f32 lowX, f32 highX, Color color)
{
    int size = MeasureText(text, fontSize);
    f32 x = CalculateCentredPosition(lowX, highX, size);
    DrawText(text, x, y, fontSize, color);
}

void DrawHCentreScreenText(const char* text, f32 y, f32 fontSize, Color color)
{
    return DrawHCentreText(text, y, fontSize, 0, GetScreenWidth(), color);
}