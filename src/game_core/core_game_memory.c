#include "core_game_memory.h"
static soc_GameMemory* GAME_MEMORY = NULL;

soc_GameMemory* core_GameMemoryGet()
{
    return GAME_MEMORY;
}

void core_GameMemorySet(soc_GameMemory* memory)
{
    assert(GAME_MEMORY == NULL); // if this assertion fails there's likely a memory leak
    GAME_MEMORY = memory;
}