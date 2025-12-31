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

    memory->textures[TextureGrass] = LoadTexture("assets/grass.png");
    memory->textures[TextureGuy] = LoadTexture("assets/bloke.png");
    memory->textures[TextureProjectile] = memory->textures[TextureGuy]; // TODO: make an actual texture for the projectile
    memory->textures[TextureProjectileSpawner] = memory->textures[TextureGuy];
}