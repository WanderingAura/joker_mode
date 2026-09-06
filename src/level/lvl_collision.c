#include <math.h>

#include "lvl_collision.h"

f32 lvl_CollisionAdjust(f32 aPos, f32 aSize, f32 bPos, f32 bSize)
{
    f32 overlap = fminf(aPos + aSize, bPos + bSize) - fmaxf(aPos, bPos);
    if (overlap <= 0.0f)
    {
        return 0.0f;
    }

    f32 aCenter = aPos + aSize / 2.0f;
    f32 bCenter = bPos + bSize / 2.0f;
    return (aCenter < bCenter) ? -overlap : overlap;
}
