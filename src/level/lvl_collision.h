#pragma once
#include <based_basic.h>

// Returns how far to move a rect A (top-left corner `aPos`, extent `aSize` along this axis)
// so it no longer overlaps rect B (`bPos`/`bSize`) on this axis; 0 if they don't actually
// overlap on this axis. Magnitude is always bounded by the real overlap depth on this axis,
// regardless of how large either rect is along the OTHER axis - call once per axis.
f32 lvl_CollisionAdjust(f32 aPos, f32 aSize, f32 bPos, f32 bSize);
