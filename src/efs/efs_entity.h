#pragma once
#include "raylib.h"
#include "efs_entity_props.h"
#include <assert.h>

#define ENTITY_POOL_SIZE 16384

#define DODGE_SPEED_SCALE 1.5
#define DODGE_COOLDOWN 2
#define DODGE_DURATION 1

// i-frame duration after taking damage (efs_prop_TempInvincible); also drives the
// flashing-red damage animation while that invincibility is active - see DrawEntities.
#define DMG_INVINCIBLE_TIME 2.0f

// Max number of simultaneous per-axis patterns a parametrically-moving entity
// (see MovementType_Parametric) can sum together
#define MAX_SIMUL_PATTERNS 8

struct efs_Entity;

typedef enum DamageGroup {
    PlayerGroup,
    EnemyGroup,
    NeutralGroup,
} DamageGroup;

typedef struct {
    struct efs_Entity* template;
    Vector2 initialDir;
    Vector2 offset;
} efs_Child;

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

typedef struct efs_Entity {
    efs_Properties props;
    union {
        Vector2 pos;
        Rectangle rect;
    };
    Vector2 dir;
    float timeSinceLastSpawn;
    float lifetime;
    float spawnTime;
    float baseRotationSpeed;
    float baseMoveSpeed;
    float invincibleTimer;
    float attackCooldown;
    float curAttackCooldown;
    Vector2 offsetFromParent;
    int health;
    DamageGroup damageGroup;
    Vector2 dodgeDirection;
    float dodgeTimer;
    float dodgeCooldown;
    int damage;
    DamageGroup canDamage;
    float despawnDistance;
    efs_Child childInfo; // contains template for entities
    Vector2 spawnedEntityDir;
    struct efs_Entity* following; // this should really be a generational index so that we can solve dangling references.
    Texture2D texture;

    // -- Parametric bullet values gated behind efs_prop_ParametricMovement --
    Vector2 init_pos; // parametric anchor: origin patterns are evaluated relative to
    Vector2 vel; // used by MovementType_Velocity
    float radius;
    float time_since_spawn;
    float rotation; // rotates the parametric coordinate frame; set once at spawn
    Color color;
    MovementType move_type;
    // Set of parametric patterns to follow when in parametric mode.
    MovementInfo move_info_x[MAX_SIMUL_PATTERNS];
    MovementInfo move_info_y[MAX_SIMUL_PATTERNS];
    float parametric_speed; // can be changed every frame to make bullets speed up or slow down
    MovementInfo speed_info;
    struct efs_Entity* target; // used by MovementType_Velocity
    TargetType targeting_type;

    struct efs_Entity* next;
    struct efs_Entity* prev;
} efs_Entity;

#define efs_entity_list_for_each(sentinel, it) \
    for (efs_Entity* (it) = (sentinel)->next; (it) != (sentinel); (it) = (it)->next)

// It's safe to free nodes during iteration here
#define efs_entity_list_for_each_safe(sentinel, it, tmp) \
    for (efs_Entity* (it) = (sentinel)->next, *(tmp) = (it)->next; \
         (it) != (sentinel); \
         (it) = (tmp), (tmp) = (it)->next)

typedef struct efs_EntityPool {
    efs_Entity storage[ENTITY_POOL_SIZE];

    // Sentinels for the free/active intrusive lists. These are not real
    // entities - only their next/prev fields are ever touched.
    efs_Entity free_list;
    efs_Entity active_list;
} efs_EntityPool;

void efs_entity_list_init(efs_Entity* sentinel);
bool efs_entity_list_is_empty(const efs_Entity* sentinel);
void efs_entity_list_insert_before(efs_Entity* position, efs_Entity* node);
void efs_entity_list_push_back(efs_Entity* sentinel, efs_Entity* node);
void efs_entity_list_push_front(efs_Entity* sentinel, efs_Entity* node);
void efs_entity_list_remove(efs_Entity* node);

void efs_PoolInit(efs_EntityPool* pool);
void efs_PoolDelete(efs_EntityPool* pool, efs_Entity* entity);
efs_Entity* efs_PoolAdd(efs_EntityPool* pool, efs_Entity entity);
// Allocates a zeroed entity and splices it into the active list, without
// requiring a fully-constructed efs_Entity value up front. Useful for
// spawners (e.g. bullet.c) that fill in fields in place after allocating.
efs_Entity* efs_PoolAlloc(efs_EntityPool* pool);

bool efs_EntityHasProperty(efs_Entity const* entity, efs_PropertyType prop);
void efs_EntitySetProperty(efs_Entity* entity, efs_PropertyType prop);
void efs_EntityUnsetProperty(efs_Entity *entity, efs_PropertyType prop);

// entity->pos (== rect.x/y) is the entity's top-left corner everywhere in this codebase
// (matches DrawTexturePro/CheckCollisionRecs) - use this wherever "the entity's visual
// center" is actually the intended meaning (aiming, camera follow, chase targets, ...).
Vector2 efs_EntityCenter(const efs_Entity* entity);