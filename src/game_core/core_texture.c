#include "core_texture.h"
#include "based_basic.h"
#include "based_logging.h"
#include "core_game_memory.h"
#include "raylib.h"

void core_TexturesInit(soc_GameMemory* memory)
{
    // Unload all textures that are currently on the GPU, otherwise we'll have a GPU memory leak
    for (u32 i = 0; i < ArrayCount(memory->textures); i++)
    {
        if (IsTextureValid(memory->textures[i]))
        {
            BSD_DBG("Unloading texture ID %d (%s)", memory->textures[i].id, TextureTypeToString(i));
            UnloadTexture(memory->textures[i]);
        }
    }

    Image spawnerImg = GenImageColor(32, 32, ORANGE);
    Image projImg = GenImageColor(16, 16, RED);
    memory->textures[TextureGrass] = LoadTexture("assets/grass.png");
    memory->textures[TextureGuy] = LoadTexture("assets/bloke.png");
    memory->textures[TextureProjectile] = LoadTextureFromImage(projImg);
    memory->textures[TextureProjectileSpawner] = LoadTextureFromImage(spawnerImg);

    UnloadImage(spawnerImg);
    UnloadImage(projImg);
}