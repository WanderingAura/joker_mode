#pragma once
#include <raylib.h>
#include "efs_entity.h"
#include "core_game_memory.h"

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

    SpawnerTypeCount,
} SpawnerType;

void ProjectileSystemInit(soc_GameMemory* memory);
efs_Entity ProjectileEntityCreate(ProjectileType type, Vector2 pos, Vector2 dir);
efs_Entity ProjectileSpawnerCreate(SpawnerType type, Vector2 pos, Vector2 dir, ProjectileType spawnedProjectileType);