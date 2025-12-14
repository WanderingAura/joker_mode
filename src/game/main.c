#include "game.h"
#include "raylib.h"
#include "vos.h"
#include <assert.h>
#include <stdio.h>

// stubs to use if the dynamic loading of the game functions fail
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
    soc_FuncGameMemoryInit* MemoryInit;
    soc_FuncGameUpdate* Update;
} GameFuncs;

#define GAME_DLL_NAME "soc"
#define GAME_DLL_FILE_NAME (vos_DLL_PREFIX GAME_DLL_NAME "." vos_DLL_EXTENSION)

GameFuncs GetGameFuncs(vos_DLLHandle dll)
{
    assert(dll != NULL);

    GameFuncs gameFuncs = {};
    vos_DLLFuncPtr fnInit = vos_DLLGetFunc(dll, "soc_GameMemoryInit");
    if (fnInit)
    {
        gameFuncs.MemoryInit = fnInit;
    }


    vos_DLLFuncPtr fnUpdate = vos_DLLGetFunc(dll, "soc_GameUpdate");
    if (fnUpdate)
    {
        gameFuncs.Update = fnUpdate;
    }

    // if we failed to get either of the functions then we use the stubs instead
    if (!fnInit || !fnUpdate)
    {
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
            dllWasModifying = false;
            s64 ret = vos_DLLUnload(gameDLL);
            if (ret != 0)
            {
                printf("Failed to unload DLL\n");
                goto cleanup;
            }
            gameDLL = vos_DLLLoad(GAME_DLL_FILE_NAME);
            if (!gameDLL)
            {
                printf("Failed to load DLL\n");
                goto cleanup;
            }
            gameFuncs = GetGameFuncs(gameDLL);
        }

        gameFuncs.Update(&gameMemory);
    }

cleanup:
    CloseWindow();

    return 0;
}

