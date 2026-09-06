#include <math.h>

#include "core_menu_state.h"
#include "input.h"
#include "state_transition.h"
#include "efs_entity.h"
#include "core_game_memory.h"
#include "core_entity_template.h"
#include "core_wall.h"
#include "prop_behaviours.h"

#define WALL_THICKNESS 32.0f

// Flash rate (cycles/sec) for the white<->red damage-flash tint.
#define DMG_FLASH_HZ 3.0f

// A random top-left position on the perimeter of `bounds` for an entity of `size`, chosen so
// its rect (top-left based, like everywhere else in the entity system) stays entirely inside
// `bounds` - flush with the edge rather than spilling into the walls just beyond it.
static Vector2 RandomPointOnBoundsEdge(BoundingRect bounds, Vector2 size)
{
    switch (GetRandomValue(0, 3))
    {
        case 0: return (Vector2){(f32)GetRandomValue((int)bounds.min.x, (int)(bounds.max.x - size.x)), bounds.min.y}; // top
        case 1: return (Vector2){(f32)GetRandomValue((int)bounds.min.x, (int)(bounds.max.x - size.x)), bounds.max.y - size.y}; // bottom
        case 2: return (Vector2){bounds.min.x, (f32)GetRandomValue((int)bounds.min.y, (int)(bounds.max.y - size.y))}; // left
        default: return (Vector2){bounds.max.x - size.x, (f32)GetRandomValue((int)bounds.min.y, (int)(bounds.max.y - size.y))}; // right
    }
}

void DrawEntities(soc_GameMemory* memory)
{
    // render entities
    efs_entity_list_for_each(&memory->efs_entityPool.active_list, entity) {
        if(efs_EntityHasProperty(entity, efs_prop_HasHealth) && entity->health <= 0) {
            continue;
        }

        Color tint = WHITE;
        if (efs_EntityHasProperty(entity, efs_prop_DamageFlash))
        {
            f32 blend = (sinf(entity->invincibleTimer * 2.0f * PI * DMG_FLASH_HZ) + 1.0f) / 2.0f; // 0..1
            tint = ColorLerp(WHITE, RED, blend);
        }

        DrawTexturePro(entity->texture, (Rectangle){0.0f, 0.0f, entity->rect.width, entity->rect.height}, entity->rect, (Vector2){0.0f, 0.0f}, 0, tint);
    }
}


void InitEntities(soc_GameMemory* memory)
{
    //DEFINE guy
    efs_Entity guy = { 0 };
    efs_EntitySetProperty(&guy, efs_prop_PlayerControlled);
    efs_EntitySetProperty(&guy, efs_prop_HasHealth);
    efs_EntitySetProperty(&guy, efs_prop_ShootsAtTarget);
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
    guy.curAttackCooldown = 0.0f;
    guy.attackCooldown = 0.5f;
    guy.texture = memory->textures[TextureVGolfer];

    // // store a pointer to the player so that it's easily accessed
    memory->player = efs_PoolAdd(&memory->efs_entityPool, guy);
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
    memory->levelBounds = (BoundingRect){{0,0}, {800,600}};
    InitEntities(memory);
    CreateRoomWalls(&memory->efs_entityPool, memory->levelBounds.min, memory->levelBounds.max, WALL_THICKNESS, memory->textures[TextureWall]);
    //Add guy to pool
    memory->camera.target = (Vector2){(float)GetScreenWidth()/2.0f, (float)GetScreenHeight()/2.0f};
    memory->camera.zoom = 1.0f;
    memory->camera.offset = (Vector2){(float)GetScreenWidth()/2.0f, (float)GetScreenHeight()/2.0f};
    memory->levelTimer = 0.0f;

    EntityTemplatesInit(&memory->entityTemplates, memory->textures);
}

void MainGameUpdate(soc_GameMemory* memory)
{
    memory->levelTimer += GetFrameTime();
    static int frameCount = 0;
    // Rectangle spawnerRect = memory->entityTemplates.spawner[SpawnerNormal].rect;
    // Vector2 spawnerSize = {spawnerRect.width, spawnerRect.height};
    // if (frameCount % (60*2) == 0)
    // {
    //     Vector2 position = RandomPointOnBoundsEdge(memory->levelBounds, spawnerSize);
    //     Vector2 direction = Vector2Rotate((Vector2){1.0f, 0.0f}, GetRandomValue(0, 360));
    //     efs_Entity spawner;
    //     SpawnedProjInfo spawnedInfo = {ProjectileCircle, {1, 0}, {1,0}};
    //     spawner = ProjectileSpawnerCreate(SpawnerNormal, position, direction, spawnedInfo);
    //     efs_PoolAdd(&memory->efs_entityPool, spawner);
    // }
    // else if (frameCount % (60*3) == 0)
    // {
    //     Vector2 position = RandomPointOnBoundsEdge(memory->levelBounds, spawnerSize);
    //     Vector2 direction = Vector2Rotate((Vector2){1.0f, 0.0f}, GetRandomValue(0, 360));
    //     efs_Entity spawner;
    //     SpawnedProjInfo spawnedInfo = {ProjectileNormal, {1, 0}, {1,0}};
    //     spawner = ProjectileSpawnerCreate(SpawnerNormal, position, direction, spawnedInfo);
    //     efs_PoolAdd(&memory->efs_entityPool, spawner);
    // }
    if (frameCount % (60*4) == 0)
    {
        Rectangle enemyRect = memory->entityTemplates.enemy[EnemyChaser].rect;
        Vector2 position = RandomPointOnBoundsEdge(memory->levelBounds, (Vector2){enemyRect.width, enemyRect.height});
        efs_Entity enemy = EnemyEntityCreate(EnemyChaser, position);
        efs_PoolAdd(&memory->efs_entityPool, enemy);
    }
    frameCount++;
    //Entity updates
    {
        efs_Entity* player = memory->player;
        efs_entity_list_for_each_safe(&memory->efs_entityPool.active_list, entity, nextEntity)
        {
            //using loop control so not in a function
            if(efs_EntityHasProperty(entity, efs_prop_HasHealth) && entity->health <= 0) {
                continue;
            }
            handle_playerControlled(entity, memory);
            handle_hasRotation(entity);
            handle_canMove(entity, memory);
            handle_parametricMovement(entity, memory);
            handle_lifetime(entity, memory);
            handle_spawner(entity, memory);
            handle_shootAtTarget(entity, memory);
            handle_canDamage(entity, memory);
            handle_tempInvincible(entity);
            handle_despawnWhenFarFromPlayer(entity, memory, player);
            handle_dodge(entity);
        }
    }

    core_TilemapUpdate(&memory->tilemap, &memory->camera);

    if (memory->player && memory->player->health <= 0)
    {
        TransitionToState(memory, MenuState_GameOver);
    }
    memory->camera.target = efs_EntityCenter(memory->player);
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

        DrawJoystick();

        DrawFPS((float)GetScreenWidth()-20, 0);
    }
    EndDrawing();
}
