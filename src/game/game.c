#include <raylib.h>
#include <string.h>

#include "based_basic.h"
#include "core_tilemap.h"
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

static soc_GameMemory* GAME_MEMORY = NULL;

soc_GameMemory* soc_GameMemoryGet()
{
    return GAME_MEMORY;
}

enum {
    TextureGrass = 0,
};

SOC_EXPORT void soc_GameModuleInit(soc_GameMemory* memory)
{
    GAME_MEMORY = memory;
    colorSet[0] = WHITE;
    colorSet[1] = ORANGE;
    colorSet[2] = BLUE;
    core_TilemapInit(&memory->tilemap, (Vector2){0,0}, 15, 15, memory->textures[TextureGrass]);
}

SOC_EXPORT void soc_GameMemoryInit(soc_GameMemory* memory)
{
    memset(memory, 0, sizeof(soc_GameMemory));
    memory->lonelyRec = (Rectangle){300,300, 100, 100};
    memory->textures[TextureGrass] = LoadTexture("assets/grass.png");
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
        core_TilemapDraw(&memory->tilemap);
    }
    EndDrawing();

    memory->timeSinceLastFrame += GetFrameTime();
}