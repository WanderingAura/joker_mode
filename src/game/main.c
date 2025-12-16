#include "game.h"
#include "raylib.h"
#include "vos.h"
#include <assert.h>
#include <stdio.h>

#include "based_logging.h"

#define GAME_DLL_NAME "soc"
#define GAME_DLL_FILE_NAME (vos_DLL_PREFIX GAME_DLL_NAME "." vos_DLL_EXTENSION)

// stubs to use if the dynamic loading of the game functions fail
static void GameModuleInitStub(void)
{
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


    InitWindow(screenWidth, screenHeight, "hot reload test");
    SetTargetFPS(60);

    vos_DLLHandle gameDLL = vos_DLLLoad(GAME_DLL_FILE_NAME);
    if (gameDLL == NULL)
    {
        return 1;
    }
    GameFuncs gameFuncs = GetGameFuncs(gameDLL);

    soc_GameMemory gameMemory = {};
    gameFuncs.MemoryInit(&gameMemory);
    gameFuncs.ModuleInit();

    long dllLastModTime = GetFileModTime(GAME_DLL_FILE_NAME);
    bool dllWasModifying = false;

    while (!WindowShouldClose())
    {
        long dllCurModTime = GetFileModTime(GAME_DLL_FILE_NAME);

        bool dllModifying = false;

        // checks if DLL is being modified
        if (dllCurModTime > dllLastModTime)
        {
            dllModifying = true;
            dllWasModifying = true;
            dllLastModTime = dllCurModTime;
        }

        // if the dll has done being modified by the compiler
        if (dllWasModifying && !dllModifying)
        {
            BSD_INF("Game DLL has been modified. Performing hot reload...");
            dllWasModifying = false;
            s64 ret = vos_DLLUnload(gameDLL);
            if (ret != 0)
            {
                BSD_ERR("Failed to unload DLL\n");
                goto cleanup;
            }
            gameDLL = vos_DLLLoad(GAME_DLL_FILE_NAME);
            if (!gameDLL)
            {
                BSD_ERR("Failed to load DLL\n");
                goto cleanup;
            }
            gameFuncs = GetGameFuncs(gameDLL);
            gameFuncs.ModuleInit();
            BSD_INF("Hot reload successful");
        }

        gameFuncs.Update(&gameMemory);
    }

cleanup:
    CloseWindow();

    return 0;
}

