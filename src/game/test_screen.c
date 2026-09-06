#include "bullet.h"
#include "core_game_memory.h"
#include "core_menu_state.h"
#include "state_transition.h"
#include "demo_level.h"
#include "input.h"
#include "based_raylib.h"
#include "efs_entity.h"
#include "prop_behaviours.h"

void InitBulletHellTest(soc_GameMemory* memory)
{
    efs_PoolInit(&memory->efs_entityPool);
}

void BulletHellUpdate(soc_GameMemory* memory)
{
    static int frame_count = 0;
    if (IsButtonsPressed(NextButton))
    {
        TransitionToState(memory, MenuState_MainGame);
    }
    if (IsButtonsPressed(ResetButton))
    {
        InitBulletHellTest(memory); // restart the screen
    }

    if (frame_count % (2*60) == 0)
    {
        bullet_spawn_linear_spew(&memory->efs_entityPool, (Vector2){400,300}, 20, 60.0f, 2.0f, RED);
    }

    if (frame_count % (3*60) == 0)
    {
        bullet_spawn_inward_spiral(&memory->efs_entityPool, (Vector2){600,300}, 2, 200, 10, 10, 3, GREEN);
    }

    BeginDrawing();
        ClearBackground(BLUE);
        Color flashingBlue = PeriodicFade(SKYBLUE);
        DrawText("PRESS SPACE TO EXIT TEST SCREEN", 250, 500, 20, flashingBlue);

        efs_entity_list_for_each(&memory->efs_entityPool.active_list, b)
        {
            handle_parametricMovement(b);
            bullet_draw(b);
        }
        DrawFPS(10, 10);
    EndDrawing();
    frame_count++;
}