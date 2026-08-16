#include "core_menu_state.h"
#include "core_game_memory.h"
#include "based_raylib.h"
#include "raylib.h"
#include "state_transition.h"
#include "test_screen.h"
#include "ui_config.h"

void TitleScreenUpdate(soc_GameMemory* memory)
{
    int key = GetKeyPressed();
    if (key != 0)
    {
        TransitionToState(memory, MenuState_BulletHellTest);
    }

    Font* fonts = memory->fonts;
    BeginDrawing();
        ClearBackground(BLACK);
        DrawTextEx(
            fonts[FontTypeTitle],
            "Joker Mode",
            (Vector2){245, 200},
            40, 4, DARKBLUE);
        Color flashingDarkBlue = PeriodicFade(DARKBLUE);
        DrawHCentreScreenText("PRESS ANY KEY TO START", 500, FONT_BODY_SIZE, flashingDarkBlue);
    EndDrawing();
}
