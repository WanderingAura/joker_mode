#pragma once
#include "core_game_memory.h"

typedef void soc_FuncGameModuleInit(soc_GameMemory* memory);
typedef void soc_FuncGameMemoryInit(soc_GameMemory* memory);
typedef void soc_FuncGameUpdate(soc_GameMemory* memory);

// if platform is android we need these
void soc_GameModuleInit(soc_GameMemory* memory);
void soc_GameMemoryInit(soc_GameMemory* memory);
void soc_GameUpdate(soc_GameMemory* memory);