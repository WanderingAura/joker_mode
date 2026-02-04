#pragma once
#include "raylib.h"
#include "based_basic.h"
#include <assert.h>

#define ENTITY_POOL_SIZE 1024
#define PROPERTY_MAX_NUM 256
#define NUM_PROPS_IN_U64 64
static_assert(PROPERTY_MAX_NUM % NUM_PROPS_IN_U64 == 0, "needs to be a multiple of 64");
typedef u64 efs_Properties[PROPERTY_MAX_NUM / NUM_PROPS_IN_U64];

typedef enum {
    efs_prop_CanMove,
    efs_prop_HasHealth,
    efs_prop_FollowsOther,
    efs_prop_PlayerControlled,
    efs_prop_HasLifetime,
    efs_prop_HasRotation,
    efs_prop_Collidable,
    efs_prop_DamagesPlayer,
    efs_prop_Spawner,
    efs_prop_MovesBetweenTwoPoints,
    efs_prop_TempInvincible,
    efs_prop_DespawnWhenFarFromPlayer,
    efs_prop_ScalesWithDifficulty,
} efs_PropertyType;

typedef struct efs_Entity {
    efs_Properties props;
    int next;
    int prev;
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
    int health;
    float despawnDistance;
    struct efs_Entity* entityToSpawn;
    Vector2 spawnedEntityDir;
    struct efs_Entity* following; // watch out for dangling references
    Texture2D texture;
} efs_Entity;

typedef struct efs_EntityPool {
    int freeHead;
    int activeHead;
    efs_Entity entities[ENTITY_POOL_SIZE];
} efs_EntityPool;

void efs_PoolInit(efs_EntityPool* pool);
void efs_PoolDelete(efs_EntityPool* pool, int index);
int efs_PoolAdd(efs_EntityPool* pool, efs_Entity entity);

bool efs_EntityHasProperty(efs_Entity const* entity, efs_PropertyType prop);
void efs_EntitySetProperty(efs_Entity* entity, efs_PropertyType prop);
void efs_EntityUnsetProperty(efs_Entity *entity, efs_PropertyType prop);