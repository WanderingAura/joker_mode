#include <raylib.h>
#include <string.h>

#include "based_basic.h"
#include "game.h"

#if defined(__linux__)
  #define SOC_EXPORT
#elif defined(_WIN32)
  #define SOC_EXPORT __declspec(dllexport)
#else
#error OS/Compiler unsupported
#endif

#define COLOR_SET_SIZE 3
static Color colorSet[COLOR_SET_SIZE];

SOC_EXPORT void soc_GameModuleInit()
{
    colorSet[0] = WHITE;
    colorSet[1] = ORANGE;
    colorSet[2] = BLUE;
}

SOC_EXPORT void soc_GameMemoryInit(soc_GameMemory* memory)
{
    memset(memory, 0, sizeof(soc_GameMemory));
    memory->lonelyRec = (Rectangle){300,300, 100, 100};
}

SOC_EXPORT void soc_GameUpdate(soc_GameMemory* memory)
{
    static u32 colorIdx = 0; // NOTE: temporary scuffed code
    if (memory->timeSinceLastFrame > 1.0f)
    {
        colorIdx++;
        memory->timeSinceLastFrame = 0.0f;
    }

    memory->lonelyRec.height += 1;
    if (memory->lonelyRec.height > 100)
    {
        memory->lonelyRec.height = 50;
    }

    BeginDrawing();
    {
        ClearBackground(BLACK);
        DrawRectangleRec(memory->lonelyRec, colorSet[colorIdx%COLOR_SET_SIZE]);
    }
    EndDrawing();

    memory->timeSinceLastFrame += GetFrameTime();
}