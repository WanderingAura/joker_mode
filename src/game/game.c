#include <raylib.h>
#include <string.h>
#include <raymath.h>

#include "based_basic.h"
#include "based_logging.h"
#include "core_texture.h"
#include "core_tilemap.h"
#include "core_projectile.h"
#include "efs_entity.h"

#if defined(__linux__)
  #define SOC_EXPORT
#elif defined(_WIN32)
  #define SOC_EXPORT __declspec(dllexport)
#else
  #error OS/Compiler unsupported
#endif

#define COLOR_SET_SIZE 3
static Color colorSet[COLOR_SET_SIZE];

SOC_EXPORT void soc_GameModuleInit(soc_GameMemory* memory)
{
    core_GameMemorySet(memory);
    colorSet[0] = WHITE;
    colorSet[1] = ORANGE;
    colorSet[2] = BLUE;

    ProjectileSystemInit(memory);

    efs_Entity proj = ProjectileEntityCreate(ProjectileNormal, (Vector2){GetScreenWidth() / 2.0f,GetScreenHeight()/ 2.0f}, (Vector2){1.0f, 0.0f});
    efs_PoolAdd(memory->efs_entityPool, proj);

    proj = ProjectileEntityCreate(ProjectileCircle, (Vector2){GetScreenWidth() / 2.0f,GetScreenHeight()/ 2.0f}, (Vector2){1.0f, 0.0f});
    efs_PoolAdd(memory->efs_entityPool, proj);

    core_TilemapInit(&memory->tilemap, (Vector2){0,0}, 15, 10, memory->textures[TextureGrass]);
}

SOC_EXPORT void soc_GameMemoryInit(soc_GameMemory* memory)
{
    // TODO: change this to be dynamic based on something??
    bsd_SetLogLevel(bsd_LogLevel_Debug);
    memset(memory, 0, sizeof(soc_GameMemory));
    memory->lonelyRec = (Rectangle){300,300, 100, 100};
    core_TexturesInit(memory);
    memory->camera = (Camera2D){0};
    memory->efs_entityPool = efs_PoolInit();


    //DEFINE guy
    efs_Entity guy = { 0 };
    efs_EntitySetProperty(&guy, efs_prop_CanMove);
    efs_EntitySetProperty(&guy, efs_prop_PlayerControlled);
    efs_EntitySetProperty(&guy, efs_prop_HasHealth);
    guy.health = 100;
    guy.vel.x = 0.0f;
    guy.vel.y = 0.0f;
    guy.rect.x = GetScreenWidth() / 2.0f;
    guy.rect.y = GetScreenHeight() / 2.0f;
    guy.rect.height = 64.0f;
    guy.rect.width = 64.0f;
    guy.moveSpeed = 100.0f;
    guy.texture = memory->textures[TextureGuy];
    //Add guy to pool
    memory->camera.target = (Vector2){guy.pos.x, guy.pos.y};
    memory->camera.zoom = 1.0f;
    memory->camera.offset = (Vector2){GetScreenWidth()/2.0f, GetScreenHeight()/2.0f};

    efs_PoolAdd(memory->efs_entityPool, guy);


    BSD_INF("Game memory initialised!");
}

SOC_EXPORT void soc_GameUpdate(soc_GameMemory* memory)
{

    //Entitiy updates
    {
        int index = memory->efs_entityPool->activeHead;
        while(index >= 0) {
            efs_Entity* entity = &memory->efs_entityPool->entities[index];
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
                memory->camera.target = entity->pos;
            }
            if (efs_EntityHasProperty(entity, efs_prop_HasRotation))
            {
                entity->vel = Vector2Rotate(entity->vel, entity->rotationSpeed * GetFrameTime());
            }
            if(efs_EntityHasProperty(entity, efs_prop_CanMove)) {
                Vector2 entityStep = Vector2Scale(entity->vel, entity->moveSpeed * GetFrameTime());
                entity->pos.x += entityStep.x;
                entity->pos.y += entityStep.y;
            }
            if (efs_EntityHasProperty(entity, efs_prop_HasLifetime))
            {
                entity->lifetime -= GetFrameTime();
                if (entity->lifetime < 0)
                {
                    efs_PoolDelete(memory->efs_entityPool, index);
                }
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
            int index = memory->efs_entityPool->activeHead;
            while(index >= 0) {
                efs_Entity* entity = &memory->efs_entityPool->entities[index];
                DrawTexturePro(entity->texture, (Rectangle){0.0f, 0.0f, entity->rect.width, entity->rect.height}, entity->rect, (Vector2){0.0f, 0.0f}, 0, WHITE);
                index = entity->next;
            }
        }
        EndMode2D();
    }
    EndDrawing();

    memory->timeSinceLastFrame += GetFrameTime();
}