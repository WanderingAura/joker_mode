#include "game_api.h"
#include "raylib.h"
#include "vos.h"
#include "vos_socket.h"
#include <assert.h>

#include "based_logging.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;


    const int screenWidth = 800;
    const int screenHeight = 600;
#ifdef WLIAS_DEBUG
    bsd_SetLogLevel(bsd_LogLevel_Debug);
#endif

    vos_NetError err = vos_NetInit();
    if (err != vos_NetErrorSuccess)
    {
        BSD_ERR("Failed to initialise net api err: %d", err);
        return 1;
    }

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);

    InitWindow(screenWidth, screenHeight, "Joker Mode");
    SetTargetFPS(60);

    soc_GameMemory* gameMemory = MemAlloc(sizeof(*gameMemory));

    soc_GameMemoryInit(gameMemory);
    soc_GameModuleInit(gameMemory);

    while (!WindowShouldClose())
    {
        soc_GameUpdate(gameMemory);
    }

    CloseWindow();

    return 0;
}