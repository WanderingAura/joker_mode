#include <raylib.h>
#include <math.h>
#include <raymath.h>

#include "based_basic.h"
#include "based_core.h"

#define MAX_SIMUL_PATTERNS 64

typedef enum
{
    MovementType_Parametric, // next position is defined by parametric equations
    MovementType_Velocity, // next position is defined by velocity
} MovementType;

typedef enum
{
    Pattern_None,
    Pattern_Sinusoidal,
    Pattern_Linear,
    Pattern_Constant,
    Pattern_CubicEaseIn,
} Pattern;

typedef enum
{
    TargetType_Direct, // directly move towards target
    TargetType_Accel, // accelerate towards target
} TargetType;

typedef struct
{
    Pattern pattern;
    union
    {
        struct // sine
        {
            f32 amp;
            f32 freq;
            f32 phase;
        };

        struct // linear
        {
            f32 speed;
        };

        struct
        {
            f32 constant;
        };

        struct // cubic ease in
        {
            f32 duration;
        };
    };
} MovementInfo;

struct Entity_;

typedef struct
{
    // position
    union
    {
        struct
        {
            f32 x;
            f32 y;
        };
        Vector2 pos;
    };

    // initial position (used for parametric calculations)
    union
    {
        struct
        {
            f32 init_x;
            f32 init_y;
        };
        Vector2 init_pos;
    };

    // velocity
    union
    {
        struct
        {
            f32 vel_x;
            f32 vel_y;
        };
        Vector2 vel;
    };

    f32 radius;
    f32 time_since_spawn;
    Color color;

    MovementType move_type;

    // Set of parametric patterns to follow when in parametric mode 
    // Can consider converting to SoA if this is slow
    MovementInfo move_info_x[MAX_SIMUL_PATTERNS];
    MovementInfo move_info_y[MAX_SIMUL_PATTERNS];

    struct
    {
        f32 parametric_speed; // REMARK: we can change this every frame to make the bullets speed up or slow down.
        MovementInfo speed_info;
    };

    struct Entity_* target;
    TargetType targeting_type;
} Bullet;

typedef enum
{
    EntityType_Bullet,
    EntityType_Player,
    EntityType_Enemy,
} EntityType;

typedef struct Entity_
{
    EntityType type;

    union
    {
        Bullet bullet;
    };
} Entity;
