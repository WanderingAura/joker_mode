#pragma once
#include <raylib.h>
#include "efs_entity.h"
#include "core_game_memory.h"
#include "core_entity_types.h"

void EntityTemplatesInit(EntityTemplateTables* templates, const Texture2D textures[TextureTypeCount]);
efs_Entity ProjectileEntityCreate(ProjectileType type, Vector2 pos, Vector2 dir);
efs_Entity ProjectileSpawnerCreate(SpawnerType type, Vector2 pos, Vector2 dir, ProjectileType spawnedProjectileType);