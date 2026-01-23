#pragma once
#include "raylib.h"
#include "efs_entity.h"
#include "core_tilemap.h"
#include "core_texture_types.h"
#include "core_entity_types.h"

typedef struct
{
    efs_Entity projectile[ProjectileTypeCount];
    efs_Entity spawner[SpawnerTypeCount];
} EntityTemplateTables;

typedef struct {
    Rectangle lonelyRec;
    float timeSinceLastFrame;
    core_Tilemap tilemap;
    Texture2D textures[TextureTypeCount];
    efs_EntityPool* efs_entityPool;
    efs_Entity* player;
    Camera2D camera;
    EntityTemplateTables entityTemplates;
} soc_GameMemory;

soc_GameMemory* core_GameMemoryGet();
void core_GameMemorySet(soc_GameMemory* memory);