#include <math.h>
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