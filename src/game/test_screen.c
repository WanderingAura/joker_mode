#include "bullet.h"
#include "core_game_memory.h"
#include "demo_level.h"
#include "based_raylib.h"

void InitBulletHellTest(soc_GameMemory* memory)
{
    bullet_pool_init(&memory->bullet_pool);
}

void BulletHellUpdate(soc_GameMemory* memory)
{
    static int frame_count = 0;
    int key = GetKeyPressed();
    if (key == KEY_SPACE)
    {
        memory->menuState = MenuState_MainGame;
        InitDemoLevel(memory);
    }
    if (key == KEY_R)
    {
        InitBulletHellTest(memory); // restart the screen
    }

    if (frame_count % (2*60) == 0)
    {
        bullet_spawn_linear_spew(&memory->bullet_pool, (Vector2){400,300}, 20, 60.0f, 2.0f, RED);
    }

    if (frame_count % (3*60) == 0)
    {
        bullet_spawn_inward_spiral(&memory->bullet_pool, (Vector2){600,300}, 2, 200, 10, 10, 3, GREEN);
    }

    BeginDrawing();
        ClearBackground(BLUE);
        Color flashingBlue = PeriodicFade(SKYBLUE);
        DrawText("PRESS SPACE TO EXIT TEST SCREEN", 250, 500, 20, flashingBlue);

        bullet_list_for_each(&memory->bullet_pool.active_list, b)
        {
            bullet_update(b);
            bullet_draw(b);
        }
        DrawFPS(10, 10);
    EndDrawing();
    frame_count++;
}