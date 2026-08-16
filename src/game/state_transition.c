#include "based_core.h"
#include "core_menu_state.h"
#include "demo_level.h"
#include "title_screen.h"
#include "test_screen.h"
#include "gameover.h"
#include "state_transition.h"

static StateTransition state_transitions[] =
{
    [MenuState_Title] = 
    {
        .entry = NULL,
        .exit = NULL,
    },
    [MenuState_MainGame] =
    {
        .entry = InitDemoLevel,
        .exit = NULL,
    },
    [MenuState_BulletHellTest] =
    {
        .entry = InitBulletHellTest,
        .exit = NULL,
    },
    [MenuState_GameOver] =
    {
        .entry = InitGameOver,
        .exit = NULL,
    },
};

static_assert(ArrayCount(state_transitions) == MenuStateCount, "every state should have transitions");

void TransitionToState(soc_GameMemory* memory, GameMenuState target_state)
{
    if (state_transitions[memory->menuState].exit)
    {
        state_transitions[memory->menuState].exit(memory);
    }

    memory->menuState = target_state;
    if (state_transitions[target_state].entry)
    {
        state_transitions[target_state].entry(memory);
    }
}