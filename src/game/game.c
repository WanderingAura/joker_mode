#include <raylib.h>
#include <string.h>

#include "based_basic.h"
#include "game.h"

static Color colorSet[] = {MAGENTA, BLUE, RED}; // NOTE: temporary scuffed code

void soc_GameMemoryInit(soc_GameMemory* memory)
{
    memset(memory, 0, sizeof(soc_GameMemory));
    memory->lonelyRec = (Rectangle){300,300, 100, 100};
}

void soc_GameUpdate(soc_GameMemory* memory)
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
        DrawRectangleRec(memory->lonelyRec, colorSet[colorIdx%(ArrayCount(colorSet))]);
    }
    EndDrawing();

    memory->timeSinceLastFrame += GetFrameTime();
}