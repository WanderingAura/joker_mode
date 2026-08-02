#include "core_menu_state.h"
#include "core_game_memory.h"
#include "based_raylib.h"
#include "raylib.h"
#include "test_screen.h"

void TitleScreenUpdate(soc_GameMemory* memory)
{
    int key = GetKeyPressed();
    if (key != 0)
    {
        memory->menuState = MenuState_BulletHellTest;
        InitBulletHellTest(memory);
    }

    f32 time = GetTime();

    Font* fonts = memory->fonts;
    BeginDrawing();
        ClearBackground(BLACK);
        DrawTextEx(
            fonts[FontTypeTitle],
            "Joker Mode",
            (Vector2){245, 200},
            40, 4, DARKBLUE);
        Color flashingDarkBlue = PeriodicFade(DARKBLUE);
        DrawText("PRESS ANY KEY TO START", 250, 500, 20, flashingDarkBlue);
    EndDrawing();
}
