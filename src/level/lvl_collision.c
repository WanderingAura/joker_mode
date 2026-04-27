#include <stdlib.h>
#include <raylib.h>

// returns the required movement to stop collision in a given dimension
int lvl_CollisionAdjust(int x1, int w1, int x2, int w2)
{
    int dist = x2 - x1;
    // half widths
    int hw1 = w1/2;
    int hw2 = w2/2;

    int x1High = x1 + hw1;
    int x1Low = x1 - hw1;

    int x2High = x2 + hw2;
    int x2Low = x2 - hw2;

    if (abs(dist) < hw1 + hw2)
    {
        // collided
        if (dist <= 0)
        {
            return x2High - x1Low;
        }
        else
        {
            return x2Low - x1High;
        }
    }
    return 0;
}