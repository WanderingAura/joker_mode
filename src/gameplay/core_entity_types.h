#pragma once
#include <raylib.h>
#include <efs_entity.h>

typedef enum ProjectileType
{
    ProjectileNormal,
    ProjectileCircle,
    ProjectileNormalSpeedsUp,
    
    ProjectileTypeCount,
} ProjectileType;

typedef enum SpawnerType
{
    SpawnerNormal,
    SpawnerTwoPoints,

    SpawnerTypeCount,
} SpawnerType;

typedef enum EnemyType
{
    EnemyChaser, // walks straight at the player and fires at them on cooldown

    EnemyTypeCount,
} EnemyType;

typedef struct {
    efs_Entity* template;
    Vector2 offset;
    Vector2 dir;
} SpawnedEntity;