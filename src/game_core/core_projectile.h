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

void ProjectileSystemInit(soc_GameMemory* memory);
efs_Entity ProjectileEntityCreate(ProjectileType type, Vector2 pos, Vector2 vel);