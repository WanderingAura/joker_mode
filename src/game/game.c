#include <raylib.h>
#include <string.h>
#include <raymath.h>

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
    TextureGuy = 1,
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
    memory->textures[TextureGuy] = LoadTexture("assets/bloke.png");
    memory->efs_entitiyPool = initPool();
    memory->camera = (Camera2D){0};


    //DEFINE guy
    efs_Entity guy = { 0 };
    efs_SetEntityProperty(&guy, efs_prop_CanMove);
    efs_SetEntityProperty(&guy, efs_prop_PlayerControlled);
    efs_SetEntityProperty(&guy, efs_prop_HasHealth);
    guy.health = 100;
    guy.vel.x = 0.0f;
    guy.vel.y = 0.0f;
    guy.pos.x = GetScreenWidth() / 2.0f;
    guy.pos.y = GetScreenHeight() / 2.0f;
    guy.pos.height = 64.0f;
    guy.pos.width = 64.0f;
    guy.moveSpeed = 10;
    guy.texture = memory->textures[TextureGuy];
    //Add guy to pool
    addToPool(memory->efs_entitiyPool, guy);

    memory->camera.target = (Vector2){guy.pos.x, guy.pos.y};
    memory->camera.zoom = 1.0f;
    memory->camera.offset = (Vector2){GetScreenWidth()/2.0f, GetScreenHeight()/2.0f};

}

SOC_EXPORT void soc_GameUpdate(soc_GameMemory* memory)
{

    //Entitiy updates
    {
        int index = memory->efs_entitiyPool->activeHead;
        while(index >= 0) {
            efs_Entity* entity = &memory->efs_entitiyPool->entities[index];
            if(efs_EntityHasProperty(entity, efs_prop_PlayerControlled)) {
                entity->vel.x = 0.0f;
                entity->vel.y = 0.0f;
                if(IsKeyDown(KEY_W)) {
                    entity->vel.y -= 1.0f;
                };
                if(IsKeyDown(KEY_S)) {
                    entity->vel.y += 1.0f;
                }
                if(IsKeyDown(KEY_A)) {
                    entity->vel.x -= 1.0f;
                }
                if(IsKeyDown(KEY_D)) {
                    entity->vel.x += 1.0f;
                }
                entity->vel = Vector2Normalize(entity->vel);
                entity->vel = Vector2Scale(entity->vel, entity->moveSpeed);
                memory->camera.target = (Vector2){entity->pos.x, entity->pos.y};

            }
            if(efs_EntityHasProperty(entity, efs_prop_CanMove)) {
                entity->pos.x += entity->vel.x;
                entity->pos.y += entity->vel.y;
            }
            index = entity->next;
        }
    }

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
        BeginMode2D(memory->camera);
        {
            DrawRectangleRec(memory->lonelyRec, colorSet[colorIdx%COLOR_SET_SIZE]);
            core_TilemapDraw(&memory->tilemap);
            //render entities
            int index = memory->efs_entitiyPool->activeHead;
            while(index >= 0) {
                efs_Entity* entity = &memory->efs_entitiyPool->entities[index];
                DrawTexturePro(entity->texture, (Rectangle){0.0f, 0.0f, 64.0f, 64.0f}, entity->pos, (Vector2){0.0f, 0.0f}, 0, WHITE);
                index = entity->next;
            }
        }
        EndMode2D();
    }
    EndDrawing();

    memory->timeSinceLastFrame += GetFrameTime();
}