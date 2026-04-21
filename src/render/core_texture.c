#include "core_texture.h"
#include "based_basic.h"
#include "based_logging.h"
#include "core_game_memory.h"
#include "raylib.h"

void core_TexturesInit(Texture2D* textures)
{
    // Unload all textures that are currently on the GPU, otherwise we'll have a GPU memory leak
    for (u32 i = 0; i < TextureTypeCount; i++)
    {
        if (IsTextureValid(textures[i]))
        {
            BSD_DBG("Unloading texture ID %d (%s)", textures[i].id, TextureTypeToString(i));
            UnloadTexture(textures[i]);
        }
    }

    Image spawnerImg = GenImageColor(32, 32, ORANGE);
    Image projImg = GenImageColor(16, 16, RED);
    Image wallImg = GenImageColor(16, 16, BLACK);
    textures[TextureGrass] = LoadTexture("assets/grass.png");
    textures[TextureGuy] = LoadTexture("assets/bloke.png");
    textures[TextureProjectile] = LoadTextureFromImage(projImg);
    textures[TextureProjectileSpawner] = LoadTextureFromImage(spawnerImg);
    textures[TextureWall] = LoadTextureFromImage(wallImg);
    SetTextureWrap(textures[TextureWall], TEXTURE_WRAP_REPEAT);

    UnloadImage(spawnerImg);
    UnloadImage(projImg);
    UnloadImage(wallImg);
}