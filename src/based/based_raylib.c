#include <math.h>
#include "based_raylib.h"

// it shouldn't be up to the consumer to call this only once per frame.
// TODO: add a flag to this function so that it only calls sine once per frame
float GetPeriodicTime(float start, float end, float periodSeconds)
{
    static float counter = 0;
    counter += GetFrameTime() * (2.0f * PI) / periodSeconds;
    float midpoint = (end + start) / 2.0f;
    float halfDist = (end - start) / 2.0f;

    return midpoint + (halfDist * sinf(counter));
}