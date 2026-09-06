#pragma once
#include "raylib.h"
#include "efs_entity_props.h"
#include <assert.h>

#define ENTITY_POOL_SIZE 1024

#define DODGE_SPEED_SCALE 1.5
#define DODGE_COOLDOWN 2
#define DODGE_DURATION 1

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

    // Intrusive free/active list links - see BulletPool in bullet.h for the
    // pattern this mirrors. Kept last since these are the fields planned to
    // survive a future merge of Bullet into efs_Entity.
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

bool efs_EntityHasProperty(efs_Entity const* entity, efs_PropertyType prop);
void efs_EntitySetProperty(efs_Entity* entity, efs_PropertyType prop);
void efs_EntityUnsetProperty(efs_Entity *entity, efs_PropertyType prop);