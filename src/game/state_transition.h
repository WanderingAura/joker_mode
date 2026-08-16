#pragma once
#include "core_game_memory.h"

typedef struct
{
    void (*entry)(soc_GameMemory*);
    void (*exit)(soc_GameMemory*);
} StateTransition;

void TransitionToState(soc_GameMemory* memory, GameMenuState target_state);