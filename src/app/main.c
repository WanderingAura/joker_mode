#include "game_api.h"
#include "raylib.h"
#include "vos.h"
#include "vos_socket.h"
#include <assert.h>
#include <stdio.h>

#include "based_logging.h"

#define GAME_DLL_NAME "soc"
#define GAME_DLL_FILE_NAME (vos_DLL_PREFIX GAME_DLL_NAME "." vos_DLL_EXTENSION)

// stubs to use if the dynamic loading of the game functions fail
static void GameModuleInitStub(soc_GameMemory* memory)
{
    (void)memory;
    return;
}

static void GameMemoryInitStub(soc_GameMemory* memory)
{
    (void)memory;
    return;
}
static void GameUpdateStub(soc_GameMemory* memory)
{
    (void)memory;
    return;
}

typedef struct {
    soc_FuncGameModuleInit* ModuleInit;
    soc_FuncGameMemoryInit* MemoryInit;
    soc_FuncGameUpdate* Update;
} GameFuncs;

GameFuncs GetGameFuncs(vos_DLLHandle dll)
{
    assert(dll != NULL);

    GameFuncs gameFuncs = {0};
    vos_DLLFuncPtr fnModuleInit = vos_DLLGetFunc(dll, "soc_GameModuleInit");
    if (fnModuleInit)
    {
        gameFuncs.ModuleInit = (soc_FuncGameModuleInit*)fnModuleInit;
    }

    vos_DLLFuncPtr fnMemoryInit = vos_DLLGetFunc(dll, "soc_GameMemoryInit");
    if (fnMemoryInit)
    {
        gameFuncs.MemoryInit = (soc_FuncGameMemoryInit*)fnMemoryInit;
    }

    vos_DLLFuncPtr fnUpdate = vos_DLLGetFunc(dll, "soc_GameUpdate");
    if (fnUpdate)
    {
        gameFuncs.Update = (soc_FuncGameUpdate*)fnUpdate;
    }

    // if we failed to get either of the functions then we use the stubs instead
    if (!fnModuleInit || !fnMemoryInit || !fnUpdate)
    {
        gameFuncs.ModuleInit = GameModuleInitStub;
        gameFuncs.MemoryInit = GameMemoryInitStub;
        gameFuncs.Update = GameUpdateStub;
    }

    return gameFuncs;
}

int main(void)
{

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

    InitWindow(screenWidth, screenHeight, "Joker Mode");
    SetTargetFPS(60);

    vos_DLLHandle gameDLL = vos_DLLLoad(GAME_DLL_FILE_NAME);
    if (gameDLL == NULL)
    {
        return 1;
    }
    GameFuncs gameFuncs = GetGameFuncs(gameDLL);

    soc_GameMemory* gameMemory = MemAlloc(sizeof(*gameMemory));
    gameFuncs.MemoryInit(gameMemory);
    gameFuncs.ModuleInit(gameMemory);

    long dllLastModTime = GetFileModTime(GAME_DLL_FILE_NAME);
    bool dllWasModifying = false;
    bool doHardReset = false;

    while (!WindowShouldClose())
    {
        long dllCurModTime = GetFileModTime(GAME_DLL_FILE_NAME);

        bool dllModifying = false;

        // NOTE: this is used for a hard reset of the game memory but if the game memory's structure changes between
        // reloads it might cause a crash before we have the opportunity to hard reset. we may need another mechanism:
        // - add a keybind to pause all game updates so that we can do a hard reset while the game is paused
        // - change the build and hot-reload logic so that hot-reload only happens if a certain build flag is set.
        //   we can have a dummy file for this where this file only updates when we want a hot-reload.
        if (IsKeyPressed(KEY_F5))
        {
            doHardReset = true;
        }

        // checks if DLL is being modified
        if (dllCurModTime > dllLastModTime)
        {
            dllModifying = true;
            dllWasModifying = true;
            dllLastModTime = dllCurModTime;
        }

        if ((dllWasModifying && !dllModifying) || doHardReset)
        {
            BSD_INF("Performing hot reload...");
            dllWasModifying = false;
            s64 ret = vos_DLLUnload(gameDLL);
            if (ret != 0)
            {
                BSD_ERR("Failed to unload DLL");
                goto cleanup;
            }
            if (doHardReset)
            {
                MemFree(gameMemory);
            }
            gameDLL = vos_DLLLoad(GAME_DLL_FILE_NAME);
            if (!gameDLL)
            {
                BSD_ERR("Failed to load DLL");
                goto cleanup;
            }
            gameFuncs = GetGameFuncs(gameDLL);
            if (doHardReset)
            {
                gameMemory = MemAlloc(sizeof(*gameMemory));
                gameFuncs.MemoryInit(gameMemory);
            }
            gameFuncs.ModuleInit(gameMemory);
            BSD_INF("Hot reload successful");
            doHardReset = false;
        }
        
        gameFuncs.Update(gameMemory);
    }

cleanup:
    CloseWindow();

    return 0;
}

