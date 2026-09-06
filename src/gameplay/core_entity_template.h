#pragma once
#include <raylib.h>
#include "efs_entity.h"
#include "core_game_memory.h"
#include "core_entity_types.h"

typedef struct
{
    ProjectileType type;
    Vector2 offset;
    Vector2 dir;
} SpawnedProjInfo;

void EntityTemplatesInit(EntityTemplateTables* templates, const Texture2D textures[TextureTypeCount]);
efs_Entity ProjectileEntityCreate(ProjectileType type, Vector2 pos, Vector2 dir);
efs_Entity ProjectileSpawnerCreate(SpawnerType type, Vector2 pos, Vector2 dir, SpawnedProjInfo spawnedInfo);
// Spawns an enemy at `pos`, aimed at (and targeting) the current player - see soc_GameMemory::player.
efs_Entity EnemyEntityCreate(EnemyType type, Vector2 pos);