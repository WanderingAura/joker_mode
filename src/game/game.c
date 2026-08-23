#include <raylib.h>
#include <stdbool.h>
#include <string.h>

#include "based_basic.h"
#include "based_logging.h"
#include "core_game_memory.h"
#include "core_entity_template.h"
#include "core_menu_state.h"
#include "core_texture.h"
#include "render_font.h"
#include "state_transition.h"
#include "test_screen.h"
#include "demo_level.h"
#include "title_screen.h"
#include "gameover.h"
#include "input.h"

#if defined(__linux__)
  #define SOC_EXPORT
#elif defined(_WIN32)
  #define SOC_EXPORT __declspec(dllexport)
#else
  #error OS/Compiler unsupported
#endif

SOC_EXPORT void soc_GameModuleInit(soc_GameMemory* memory)
{
    core_GameMemorySet(memory);

    EntityTemplatesInit(&memory->entityTemplates, memory->textures);

    // efs_Entity proj = ProjectileEntityCreate(ProjectileNormal, (Vector2){(float)GetScreenWidth() / 2.0f,(float)GetScreenHeight()/ 2.0f}, (Vector2){1.0f, 0.0f});
    // efs_PoolAdd(memory->efs_entityPool, proj);

    InitialiseInputs();
}

SOC_EXPORT void soc_GameMemoryInit(soc_GameMemory* memory)
{
    // updates the library's pointer to game memory
    // this allows hot reloading to work
    core_GameMemorySet(memory);

    // TODO: change this to be dynamic based on something??
    bsd_SetLogLevel(bsd_LogLevel_Debug);

    memset(memory, 0, sizeof(soc_GameMemory));
    core_TexturesInit(memory->textures);
    rnd_FontInit(memory->fonts);
    memory->camera = (Camera2D){0};
    memory->gameoverData.usernameLen = 0;

    TransitionToState(memory, MenuState_Title);

#if 0
    memory->menuState = MenuState_GameOver;
    memory->levelTimer = 0.1f;
#endif

    BSD_INF("Game memory initialised!");
}

SOC_EXPORT void soc_GameUpdate(soc_GameMemory* memory)
{
    UpdateInputs();

    switch (memory->menuState)
    {
        case MenuState_Title:
        {
            TitleScreenUpdate(memory);
        } break;
        case MenuState_BulletHellTest:
        {
            BulletHellUpdate(memory);
        } break;
        case MenuState_MainGame:
        {
            MainGameUpdate(memory);
        } break;
        case MenuState_GameOver:
        {
            UpdateGameoverData(&memory->gameoverData);
        } break;

        default:
            DBG_ASSERT_MSG(false, "Unsupported state!");
    }

}
