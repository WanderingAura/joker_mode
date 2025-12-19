// NOTE: hot reload does not work if you change this file. please restart the exe after changing this file
#pragma once
#include <raylib.h>

#include "core_tilemap.h"

typedef struct {
    Rectangle lonelyRec;
    float timeSinceLastFrame;
    core_Tilemap tilemap;
    Texture2D textures[256];
} soc_GameMemory;

typedef void soc_FuncGameModuleInit(soc_GameMemory* memory);
typedef void soc_FuncGameMemoryInit(soc_GameMemory* memory);
typedef void soc_FuncGameUpdate(soc_GameMemory* memory);

soc_GameMemory* soc_GameMemoryGet();