#include <asm-generic/errno.h>
#include <raylib.h>
#include <string.h>
#include <raymath.h>

#include "based_basic.h"
#include "based_logging.h"
#include "core_game_memory.h"
#include "core_menu_state.h"
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

SOC_EXPORT void soc_GameModuleInit(soc_GameMemory* memory)
{
    core_GameMemorySet(memory);

    ProjectileSystemInit(memory);

    // efs_Entity proj = ProjectileEntityCreate(ProjectileNormal, (Vector2){GetScreenWidth() / 2.0f,GetScreenHeight()/ 2.0f}, (Vector2){1.0f, 0.0f});
    // efs_PoolAdd(memory->efs_entityPool, proj);


    core_TilemapInit(&memory->tilemap, (Vector2){0,0}, 15, 10, memory->textures[TextureGrass]);
}

void InitDemoLevel(soc_GameMemory* memory)
{
    //DEFINE guy
    efs_Entity guy = { 0 };
    efs_EntitySetProperty(&guy, efs_prop_CanMove);
    efs_EntitySetProperty(&guy, efs_prop_PlayerControlled);
    efs_EntitySetProperty(&guy, efs_prop_HasHealth);
    guy.health = 100;
    guy.dir.x = 0.0f;
    guy.dir.y = 0.0f;
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

    // store a pointer to the player so that it's easily accessed
    memory->player = &memory->efs_entityPool->entities[memory->efs_entityPool->activeHead];

    ProjectileSystemInit(memory);
    Vector2 middleOfScreen = {GetScreenWidth()/2.0f, GetScreenHeight()/2.0f};
    efs_Entity spawner = ProjectileSpawnerCreate(SpawnerNormal, middleOfScreen, (Vector2){1.0f, 0.0f}, ProjectileNormal);
    efs_PoolAdd(memory->efs_entityPool, spawner);
}

SOC_EXPORT void soc_GameMemoryInit(soc_GameMemory* memory)
{
    core_GameMemorySet(memory);
    // TODO: change this to be dynamic based on something??
    bsd_SetLogLevel(bsd_LogLevel_Debug);
    memset(memory, 0, sizeof(soc_GameMemory));
    memory->lonelyRec = (Rectangle){300,300, 100, 100};
    core_TexturesInit(memory);
    memory->camera = (Camera2D){0};
    memory->efs_entityPool = efs_PoolInit();

    memory->menuState = MenuState_Title;

    BSD_INF("Game memory initialised!");
}

void MainGameUpdate(soc_GameMemory* memory)
{
    //Entity updates
    {
        int index = memory->efs_entityPool->activeHead;
        while(index >= 0) {
            efs_Entity* entity = &memory->efs_entityPool->entities[index];
            int nextIndex = entity->next;
            if(efs_EntityHasProperty(entity, efs_prop_PlayerControlled)) {
                entity->dir.x = 0.0f;
                entity->dir.y = 0.0f;
                if(IsKeyDown(KEY_W)) {
                    entity->dir.y -= 1.0f;
                };
                if(IsKeyDown(KEY_S)) {
                    entity->dir.y += 1.0f;
                }
                if(IsKeyDown(KEY_A)) {
                    entity->dir.x -= 1.0f;
                }
                if(IsKeyDown(KEY_D)) {
                    entity->dir.x += 1.0f;
                }
                entity->dir = Vector2Normalize(entity->dir);
                memory->camera.target = entity->pos;
            }
            if (efs_EntityHasProperty(entity, efs_prop_HasRotation))
            {
                entity->dir = Vector2Rotate(entity->dir, entity->rotationSpeed * GetFrameTime());
            }
            if(efs_EntityHasProperty(entity, efs_prop_CanMove)) {
                Vector2 entityStep = Vector2Scale(entity->dir, entity->moveSpeed * GetFrameTime());
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
            if (efs_EntityHasProperty(entity, efs_prop_Spawner))
            {
                entity->timeSinceLastSpawn += GetFrameTime();
                if (entity->timeSinceLastSpawn >= entity->spawnTime)
                {
                    entity->timeSinceLastSpawn = 0;
                    efs_Entity spawned = {};
                    memcpy(&spawned, entity->entityToSpawn, sizeof(efs_Entity));
                    spawned.dir = entity->spawnedEntityDir;
                    // TODO: here we need an extra value stored in the entity to indicate spawn offset
                    // TODO: we really need a mechanism to set a rotation value so that the entity
                    // can be at different orientations...
                    spawned.pos = entity->pos;
                    efs_PoolAdd(memory->efs_entityPool, spawned);
                }
            }

            // update for damaging player
            if (!efs_EntityHasProperty(memory->player, efs_prop_TempInvincible)
                && efs_EntityHasProperty(entity, efs_prop_DamagesPlayer))
            {
                if (memory->player && CheckCollisionRecs(entity->rect, memory->player->rect))
                {
                    // this projectile has collided with player
                    efs_EntitySetProperty(memory->player, efs_prop_TempInvincible);
                    memory->player->invincibleTimer = 1.0f;
                    memory->player->health -= 3;
                }
            }

            if (efs_EntityHasProperty(entity, efs_prop_TempInvincible))
            {
                entity->invincibleTimer -= GetFrameTime();
                if (entity->invincibleTimer < 0)
                {
                    efs_EntityUnsetProperty(entity, efs_prop_TempInvincible);
                }
            }
            index = nextIndex;
        }
    }

    BeginDrawing();
    {
        ClearBackground(BLACK);
        BeginMode2D(memory->camera);
        {
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

        DrawText(TextFormat("Player Health: %d", memory->player->health), 10, 10, 10, RED);
    }
    EndDrawing();
}

void TitleScreenUpdate(soc_GameMemory* memory)
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    int key;
    while (key = GetKeyPressed())
    {
        if (key != 0)
        {
            memory->menuState = MenuState_MainGame;
            InitDemoLevel(memory);
        }
    }

    BeginDrawing();
        DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
        DrawText("TITLE SCREEN", 20, 20, 40, DARKGREEN);
        DrawText("PRESS ANY KEY TO START", 120, 220, 20, DARKGREEN);
    EndDrawing();
}

void GameoverScreenUpdate(soc_GameMemory* memory)
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    int key = GetKeyPressed();
    if (key != 0)
    {
        memory->menuState = MenuState_Title;
    }

    BeginDrawing();
        DrawRectangle(0, 0, screenWidth, screenHeight, BLUE);
        DrawText("ENDING SCREEN", 20, 20, 40, DARKBLUE);
        DrawText("PRESS ANY KEY TO RETURN TO TITLE SCREEN", 120, 220, 20, DARKBLUE);
    EndDrawing();
}

SOC_EXPORT void soc_GameUpdate(soc_GameMemory* memory)
{
    switch (memory->menuState)
    {
        case MenuState_Title:
        {
            TitleScreenUpdate(memory);
        } break;
        case MenuState_MainGame:
        {
            MainGameUpdate(memory);
        } break;
        case MenuState_GameOver:
        {
            GameoverScreenUpdate(memory);
        } break;
    }

}