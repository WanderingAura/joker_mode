#pragma once
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
            f32 decay_duration; // amp ramps linearly to 0 over [0, decay_duration]; <= 0 means no decay (constant amp)
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

typedef struct Bullet_
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
    f32 rotation;
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

    struct Bullet_* next;
    struct Bullet_* prev;
} Bullet;


// Bullet Pool Allocator
#define BULLET_POOL_SIZE 65536

#define bullet_list_for_each(sentinel, it) \
    for (Bullet* (it) = (sentinel)->next; (it) != (sentinel); (it) = (it)->next)

// It's safe to free nodes during iteration here
#define bullet_list_for_each_safe(sentinel, it, tmp) \
    for (Bullet* (it) = (sentinel)->next, *(tmp) = (it)->next; \
         (it) != (sentinel); \
         (it) = (tmp), (tmp) = (it)->next)

typedef struct BulletPool
{
    Bullet storage[BULLET_POOL_SIZE];

    // Sentinels for the free/active intrusive lists. These are not real
    // bullets - only their next/prev fields are ever touched.
    Bullet free_list;
    Bullet active_list;
} BulletPool;

void bullet_list_init(Bullet* sentinel);
bool bullet_list_is_empty(const Bullet* sentinel);
void bullet_list_insert_before(Bullet* position, Bullet* node);
void bullet_list_push_back(Bullet* sentinel, Bullet* node);
void bullet_list_push_front(Bullet* sentinel, Bullet* node);
void bullet_list_remove(Bullet* node);

void bullet_pool_init(BulletPool* pool);
Bullet* bullet_pool_alloc(BulletPool* pool);
void bullet_pool_free(BulletPool* pool, Bullet* bullet);

// Example parametric spawn patterns - both built from only Pattern_Linear and
// Pattern_Sinusoidal.
void bullet_spawn_linear_spew(BulletPool* pool, Vector2 origin, u32 count, f32 speed, f32 radius, Color color);
void bullet_spawn_inward_spiral(BulletPool* pool, Vector2 target, u32 count, f32 orbit_radius, f32 orbit_freq, f32 collapse_duration, f32 radius, Color color);

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

void bullet_update(Bullet* bullet);
void bullet_draw(Bullet* bullet);