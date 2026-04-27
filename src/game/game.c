#include <raylib.h>
#include <stdbool.h>
#include <string.h>
#include <raymath.h>

#include "based_basic.h"
#include "based_logging.h"
#include "core_entity_types.h"
#include "core_game_memory.h"
#include "core_menu_state.h"
#include "core_texture.h"
#include "core_tilemap.h"
#include "core_entity_template.h"
#include "efs_entity.h"
#include "gameover.h"
#include "vos_socket.h"

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

    EntityTemplatesInit(&memory->entityTemplates, memory->textures);

    // efs_Entity proj = ProjectileEntityCreate(ProjectileNormal, (Vector2){(float)GetScreenWidth() / 2.0f,(float)GetScreenHeight()/ 2.0f}, (Vector2){1.0f, 0.0f});
    // efs_PoolAdd(memory->efs_entityPool, proj);


}

void ClampIfPlayer(efs_Entity* entity, BoundingRect bounds)
{
    if (efs_EntityHasProperty(entity, efs_prop_PlayerControlled))
    {
        // clamp player movement within the level's bounds
        Vector2 max = Vector2Subtract(bounds.max, (Vector2){entity->rect.width, entity->rect.height});
        entity->pos = Vector2Clamp(entity->pos, bounds.min, max);
    }
}

void InitEntities(soc_GameMemory* memory)
{
    //DEFINE guy
    efs_Entity guy = { 0 };
    efs_EntitySetProperty(&guy, efs_prop_CanMove);
    efs_EntitySetProperty(&guy, efs_prop_PlayerControlled);
    efs_EntitySetProperty(&guy, efs_prop_HasHealth);
    efs_EntitySetProperty(&guy, efs_prop_ShootsAtMouse);
    guy.health = 10;
    guy.damageGroup = PlayerGroup;
    guy.dir.x = 0.0f;
    guy.dir.y = 0.0f;
    guy.rect.x = (float)GetScreenWidth() / 2.0f;
    guy.rect.y = (float)GetScreenHeight() / 2.0f;
    guy.rect.height = 64.0f;
    guy.rect.width = 64.0f;
    guy.baseMoveSpeed = 300.0f;
    guy.childInfo.template = &memory->entityTemplates.projectile[ProjectileNormal];
    guy.attackCoolDown = 0.0f;
    guy.attackSpeed = 2.0f;
    guy.texture = memory->textures[TextureGuy];
    efs_PoolAdd(&memory->efs_entityPool, guy);

    // // store a pointer to the player so that it's easily accessed
    memory->player = &memory->efs_entityPool.entities[memory->efs_entityPool.activeHead];

    Vector2 middleOfScreen = {(float)GetScreenWidth()/2.0f, (float)GetScreenHeight()/2.0f};

    SpawnedProjInfo spawnedInfo = {
        ProjectileCircle,
        .offset = {100, 0},
        .dir = {0,1}};
    efs_Entity spawner = ProjectileSpawnerCreate(SpawnerNormal, middleOfScreen, (Vector2){1.0f, 0.0f}, spawnedInfo);
    efs_PoolAdd(&memory->efs_entityPool, spawner);
}

void DrawBounds(BoundingRect bounds)
{
    Vector2 topLeft = bounds.min;
    Vector2 bottomRight = bounds.max;
    Vector2 topRight = {bounds.max.x, bounds.min.y};
    Vector2 bottomLeft = {bounds.min.x, bounds.max.y};
    DrawLineV(topLeft, topRight, RED);
    DrawLineV(bottomLeft, bottomRight, RED);
    DrawLineV(topLeft, bottomLeft, RED);
    DrawLineV(topRight, bottomRight, RED);
}

void DrawEntities(soc_GameMemory* memory)
{
    // render entities
    int index = memory->efs_entityPool.activeHead;
    while(index >= 0) {
        efs_Entity* entity = &memory->efs_entityPool.entities[index];
        if(efs_EntityHasProperty(entity, efs_prop_HasHealth) && entity->health <= 0) {
            index = entity->next;
            continue;
        }
        DrawTexturePro(entity->texture, (Rectangle){0.0f, 0.0f, entity->rect.width, entity->rect.height}, entity->rect, (Vector2){0.0f, 0.0f}, 0, WHITE);
        index = entity->next;
    }
}

void InitDemoLevel(soc_GameMemory* memory)
{
    efs_PoolInit(&memory->efs_entityPool);

    core_TilemapInit(&memory->tilemap, (Vector2){0,0}, 16, 12, memory->textures[TextureGrass]);
    InitEntities(memory);
    //Add guy to pool
    memory->camera.target = (Vector2){(float)GetScreenWidth()/2.0f, (float)GetScreenHeight()/2.0f};
    memory->camera.zoom = 1.0f;
    memory->camera.offset = (Vector2){(float)GetScreenWidth()/2.0f, (float)GetScreenHeight()/2.0f};
    memory->levelBounds = (BoundingRect){{0,0}, {800,600}};
    memory->levelTimer = 0.0f;

    EntityTemplatesInit(&memory->entityTemplates, memory->textures);
}

SOC_EXPORT void soc_GameMemoryInit(soc_GameMemory* memory)
{
    // Updates the library's pointer to game memory
    core_GameMemorySet(memory);

    // TODO: change this to be dynamic based on something??
    bsd_SetLogLevel(bsd_LogLevel_Debug);

    memset(memory, 0, sizeof(soc_GameMemory));
    core_TexturesInit(memory);
    memory->camera = (Camera2D){0};
    memory->gameoverData.usernameLen = 0;

    memory->menuState = MenuState_Title;

// #if 1
//     memory->menuState = MenuState_GameOver;
//     memory->levelTimer = 0.1f;
// #endif

    BSD_INF("Game memory initialised!");
}

void MainGameUpdate(soc_GameMemory* memory)
{
    memory->levelTimer += GetFrameTime();
    static int frameCount = 0;
    if (frameCount % (60*2) == 0)
    {
        Vector2 position = {GetRandomValue(-50, 850), GetRandomValue(-50, 650)};
        Vector2 direction = Vector2Rotate((Vector2){1.0f, 0.0f}, GetRandomValue(0, 360));
        efs_Entity spawner;
        SpawnedProjInfo spawnedInfo = {ProjectileCircle, {1, 0}, {1,0}};
        spawner = ProjectileSpawnerCreate(SpawnerNormal, position, direction, spawnedInfo);
        efs_PoolAdd(&memory->efs_entityPool, spawner);
    }
    else if (frameCount % (60*3) == 0)
    {
        Vector2 position = {GetRandomValue(100, 500), GetRandomValue(100, 400)};
        Vector2 direction = Vector2Rotate((Vector2){1.0f, 0.0f}, GetRandomValue(0, 360));
        efs_Entity spawner;
        SpawnedProjInfo spawnedInfo = {ProjectileNormal, {1, 0}, {1,0}};
        spawner = ProjectileSpawnerCreate(SpawnerNormal, position, direction, spawnedInfo);
        efs_PoolAdd(&memory->efs_entityPool, spawner);
    }
    frameCount++;
    //Entity updates
    {
        efs_Entity* player = memory->player;
        int index = memory->efs_entityPool.activeHead;
        while(index >= 0) {
            efs_Entity* entity = &memory->efs_entityPool.entities[index];
            int nextIndex = entity->next;

            if(efs_EntityHasProperty(entity, efs_prop_HasHealth) && entity->health <= 0) {
                index = nextIndex;
                continue;
            }

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
                entity->dir = Vector2Rotate(entity->dir, entity->baseRotationSpeed * GetFrameTime());
            }
            if(efs_EntityHasProperty(entity, efs_prop_CanMove)) {
                float stepAmount = entity->baseMoveSpeed * GetFrameTime();
                if (efs_EntityHasProperty(entity, efs_prop_ScalesWithDifficulty))
                {
                    stepAmount *= 1 + memory->levelTimer/10;
                }
                Vector2 entityStep = Vector2Scale(entity->dir, stepAmount);
                entity->pos.x += entityStep.x;
                entity->pos.y += entityStep.y;
            }
            if (efs_EntityHasProperty(entity, efs_prop_HasLifetime))
            {
                entity->lifetime -= GetFrameTime();
                if (entity->lifetime < 0)
                {
                    efs_PoolDelete(&memory->efs_entityPool, index);
                }
            }
            if (efs_EntityHasProperty(entity, efs_prop_Spawner))
            {
                entity->timeSinceLastSpawn += GetFrameTime();
                if (entity->timeSinceLastSpawn >= entity->spawnTime)
                {
                    entity->timeSinceLastSpawn = 0;
                    efs_Entity spawned = {0};
                    memcpy(&spawned, entity->childInfo.template, sizeof(efs_Entity));
                    spawned.dir = entity->childInfo.initialDir;
                    spawned.pos = Vector2Add(entity->pos, entity->childInfo.offset);
                    efs_PoolAdd(&memory->efs_entityPool, spawned);
                }
            }
            if(efs_EntityHasProperty(entity, efs_prop_ShootsAtMouse)) {
                entity->attackCoolDown -= GetFrameTime();
                if(entity->attackCoolDown <= 0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    
                    entity->attackCoolDown = entity->attackSpeed;
                    efs_Entity bullet = {0};
                    memcpy(&bullet, entity->childInfo.template, sizeof(efs_Entity));
                    bullet.canDamage = EnemyGroup;
                    bullet.dir = (Vector2){1, 0};
                    bullet.pos = entity->pos;
                    efs_PoolAdd(&memory->efs_entityPool, bullet);
                }
            }

            // update for damaging player
            int j = memory->efs_entityPool.activeHead;
            while(j >= 0) {
                efs_Entity* target = &memory->efs_entityPool.entities[j];
                if(j == index) {
                    j = target->next;
                }
                if (efs_EntityHasProperty(target, efs_prop_HasHealth)
                    && !efs_EntityHasProperty(target, efs_prop_TempInvincible)
                    && efs_EntityHasProperty(entity, efs_prop_CanDamage)                
                    && entity->canDamage == entity->damageGroup) {
                    if (target && CheckCollisionRecs(entity->rect, target->rect)) {
                        // this projectile has collided with player
                        efs_EntitySetProperty(target, efs_prop_TempInvincible);
                        target->invincibleTimer = 3.0f;
                        target->health -= entity->damage;

                    }
                }
                j = target->next;
            }

            if (efs_EntityHasProperty(entity, efs_prop_TempInvincible))
            {
                entity->invincibleTimer -= GetFrameTime();
                if (entity->invincibleTimer < 0)
                {
                    efs_EntityUnsetProperty(entity, efs_prop_TempInvincible);
                }
            }

            if (efs_EntityHasProperty(entity, efs_prop_DespawnWhenFarFromPlayer))
            {
                DBG_ASSERT_MSG(entity->despawnDistance > 0, "Got %f despawn distance. Entity with this property should have >0 despawn distance");
                float distanceToPlayer = Vector2Distance(entity->pos, player->pos);
                if (distanceToPlayer > entity->despawnDistance)
                {
                    efs_PoolDelete(&memory->efs_entityPool, index);
                }
            }
            index = nextIndex;
        }
    }

    core_TilemapUpdate(&memory->tilemap, &memory->camera);

    if (memory->player && memory->player->health <= 0)
    {
        memory->menuState = MenuState_GameOver;
        memory->gameoverData.state = GameoverState_InputScore;
    }

    BeginDrawing();
    {
        ClearBackground(BLACK);
        BeginMode2D(memory->camera);
        {
            core_TilemapDraw(&memory->tilemap);
            DrawEntities(memory);
            DrawBounds(memory->levelBounds);
        }
        EndMode2D();

        DrawText(TextFormat("Player Health: %d", memory->player->health), 10, 10, 20, RED);
        DrawText(TextFormat("Time survived: %.1f", memory->levelTimer), 10, 40, 20, GREEN);

        DrawFPS((float)GetScreenWidth()-20, 0);
    }
    EndDrawing();
}

void TitleScreenUpdate(soc_GameMemory* memory)
{
    int screenWidth = (float)GetScreenWidth();
    int screenHeight = (float)GetScreenHeight();

    int key = GetKeyPressed();
    if (key != 0)
    {
        memory->menuState = MenuState_MainGame;
        InitDemoLevel(memory);
    }
    static int alphaCount = 0;
    float alpha = ( (sinf((float)alphaCount / 10.0f) + 1.0f )* 0.5f );
    BeginDrawing();
        DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
        DrawText("JOKER MODE", 250, 200, 40, DARKGREEN);
        DrawText("PRESS ANY KEY TO START", 250, 500, 20, Fade(DARKGREEN, alpha));
    EndDrawing();
    alphaCount++;
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
            UpdateGameoverData(&memory->gameoverData);
        } break;

        default:
            DBG_ASSERT_MSG(false, "Unsupported state!");
    }

}