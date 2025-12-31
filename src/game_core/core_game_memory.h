#pragma once
#include "raylib.h"
#include "efs_entity.h"
#include "core_tilemap.h"
#include "core_texture_types.h"

typedef struct {
    Rectangle lonelyRec;
    float timeSinceLastFrame;
    core_Tilemap tilemap;
    Texture2D textures[TextureTypeCount];
    efs_EntityPool* efs_entityPool;
    Camera2D camera;
} soc_GameMemory;

soc_GameMemory* core_GameMemoryGet();
void core_GameMemorySet(soc_GameMemory* memory);