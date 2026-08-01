#include "core_menu_state.h"
#include "core_game_memory.h"
#include "test_screen.h"

void TitleScreenUpdate(soc_GameMemory* memory)
{
    int key = GetKeyPressed();
    if (key != 0)
    {
        memory->menuState = MenuState_BulletHellTest;
        InitBulletHellTest(memory);
    }

    Font* fonts = memory->fonts;
    static int alphaCount = 0;
    float alpha = ( (sinf((float)alphaCount / 10.0f) + 1.0f )* 0.5f );
    BeginDrawing();
        ClearBackground(BLACK);
        DrawTextEx(
            fonts[FontTypeTitle],
            "Joker Mode",
            (Vector2){245, 200},
            40, 4, DARKBLUE);
        DrawText("PRESS ANY KEY TO START", 250, 500, 20, Fade(DARKBLUE, alpha));
    EndDrawing();
    alphaCount++;
}
