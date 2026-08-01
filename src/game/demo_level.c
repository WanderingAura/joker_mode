#include "efs_entity.h"
#include "core_game_memory.h"
#include "core_entity_template.h"
#include "prop_behaviours.h"

void ClampIfPlayer(efs_Entity* entity, BoundingRect bounds)
{
    if (efs_EntityHasProperty(entity, efs_prop_PlayerControlled))
    {
        // clamp player movement within the level's bounds
        Vector2 max = Vector2Subtract(bounds.max, (Vector2){entity->rect.width, entity->rect.height});
        entity->pos = Vector2Clamp(entity->pos, bounds.min, max);
    }
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


void InitEntities(soc_GameMemory* memory)
{
    //DEFINE guy
    efs_Entity guy = { 0 };
    efs_EntitySetProperty(&guy, efs_prop_CanMove);
    efs_EntitySetProperty(&guy, efs_prop_PlayerControlled);
    efs_EntitySetProperty(&guy, efs_prop_HasHealth);
    efs_EntitySetProperty(&guy, efs_prop_ShootsAtMouse);
    efs_EntitySetProperty(&guy, efs_prop_CanDodge);
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
    guy.texture = memory->textures[TextureVGolfer];
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

    // efs_Entity wall = CreateWall((Rectangle){100.0f, 100.0f, 100.0f, 100.0f}, memory->textures[TextureWall]);
    // efs_PoolAdd(&memory->efs_entityPool, wall);
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

bool RectCollidesWall(Rectangle rect, efs_EntityPool* entityPool, Vector2* collideDir)
{
    collideDir->x = 0;
    collideDir->y = 0;
    int index = entityPool->activeHead;
    while (index >= 0)
    {
        efs_Entity* wallEntity = &entityPool->entities[index];
        if (efs_EntityHasProperty(wallEntity, efs_prop_Solid))
        {
            if (CheckCollisionRecs(rect, wallEntity->rect))
            {
                return true;
            }
        }
        index = wallEntity->next;
    }
    return true;
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
            //using loop control so not in a function
            if(efs_EntityHasProperty(entity, efs_prop_HasHealth) && entity->health <= 0) {
                index = nextIndex;
                continue;
            }
            handle_playerControlled(entity, memory);
            handle_hasRotation(entity);
            handle_canMove(entity, memory);
            handle_solid(entity, player);
            handle_lifetime(entity, memory, index);
            handle_spawner(entity, memory);
            handle_shootAtMouse(entity, memory);
            handle_canDamage(entity, memory, index);
            handle_tempInvincible(entity);
            handle_despawnWhenFarFromPlayer(entity, memory, player, index);
            handle_dodge(entity);
            index = nextIndex;
        }
    }

    core_TilemapUpdate(&memory->tilemap, &memory->camera);

    if (memory->player && memory->player->health <= 0)
    {
        memory->menuState = MenuState_GameOver;
        memory->gameoverData.state = GameoverState_InputScore;
    }
    memory->camera.target = memory->player->pos;
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
