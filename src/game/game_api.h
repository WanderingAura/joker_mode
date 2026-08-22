#pragma once
#include "core_game_memory.h"

// on android we statically compile everything into one .so so we don't
// support hot reloading and use the functions directly.
#ifdef GAME_PLATFORM_ANDROID
void soc_GameModuleInit(soc_GameMemory* memory);
void soc_GameMemoryInit(soc_GameMemory* memory);
void soc_GameUpdate(soc_GameMemory* memory);
#else
typedef void soc_FuncGameModuleInit(soc_GameMemory* memory);
typedef void soc_FuncGameMemoryInit(soc_GameMemory* memory);
typedef void soc_FuncGameUpdate(soc_GameMemory* memory);
#endif