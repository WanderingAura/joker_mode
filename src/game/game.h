// NOTE: hot reload does not work if you change this file. please restart the exe after changing this file
#pragma once
#include <raylib.h>

typedef struct {
    Rectangle lonelyRec;
    float timeSinceLastFrame;
} soc_GameMemory;

typedef void soc_FuncGameMemoryInit(soc_GameMemory* memory);
typedef void soc_FuncGameUpdate(soc_GameMemory* memory);