#include "core_game_memory.h"
static soc_GameMemory* GAME_MEMORY = NULL;

soc_GameMemory* core_GameMemoryGet()
{
    return GAME_MEMORY;
}

void core_GameMemorySet(soc_GameMemory* memory)
{
    BSD_INF("Setting game memory");
    GAME_MEMORY = memory;
}