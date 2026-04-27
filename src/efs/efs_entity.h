#pragma once
#include "raylib.h"
#include "efs_entity_props.h"
#include <assert.h>

#define ENTITY_POOL_SIZE 1024

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
    float attackSpeed;
    float attackCoolDown;
    Vector2 offsetFromParent;
    int health;
    DamageGroup damageGroup;
    int damage;
    DamageGroup canDamage;
    float despawnDistance;
    efs_Child childInfo; // contains template for entities
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