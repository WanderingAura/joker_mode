#include <raylib.h>
#include <stdio.h>
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
#include "http.h"

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
    guy.health = 10;
    guy.dir.x = 0.0f;
    guy.dir.y = 0.0f;
    guy.rect.x = (float)GetScreenWidth() / 2.0f;
    guy.rect.y = (float)GetScreenHeight() / 2.0f;
    guy.rect.height = 64.0f;
    guy.rect.width = 64.0f;
    guy.baseMoveSpeed = 300.0f;
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
    static int count = 0;
    if (count % (60*2) == 0)
    {
        Vector2 position = {GetRandomValue(-50, 850), GetRandomValue(-50, 650)};
        Vector2 direction = Vector2Rotate((Vector2){1.0f, 0.0f}, GetRandomValue(0, 360));
        efs_Entity spawner;
        SpawnedProjInfo spawnedInfo = {ProjectileCircle, {1, 0}, {1,0}};
        spawner = ProjectileSpawnerCreate(SpawnerNormal, position, direction, spawnedInfo);
        efs_PoolAdd(&memory->efs_entityPool, spawner);
    }
    else if (count % (60*3) == 0)
    {
        Vector2 position = {GetRandomValue(100, 500), GetRandomValue(100, 400)};
        Vector2 direction = Vector2Rotate((Vector2){1.0f, 0.0f}, GetRandomValue(0, 360));
        efs_Entity spawner;
        SpawnedProjInfo spawnedInfo = {ProjectileNormal, {1, 0}, {1,0}};
        spawner = ProjectileSpawnerCreate(SpawnerNormal, position, direction, spawnedInfo);
        efs_PoolAdd(&memory->efs_entityPool, spawner);
    }
    count++;
    //Entity updates
    {
        efs_Entity* player = memory->player;
        int index = memory->efs_entityPool.activeHead;
        while(index >= 0) {
            efs_Entity* entity = &memory->efs_entityPool.entities[index];
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
                    efs_Entity spawned = {};
                    memcpy(&spawned, entity->childInfo.template, sizeof(efs_Entity));
                    spawned.dir = entity->childInfo.initialDir;
                    spawned.pos = Vector2Add(entity->pos, entity->childInfo.offset);
                    efs_PoolAdd(&memory->efs_entityPool, spawned);
                }
            }

            // update for damaging player
            if (!efs_EntityHasProperty(player, efs_prop_TempInvincible)
                && efs_EntityHasProperty(entity, efs_prop_DamagesPlayer))
            {
                if (player && CheckCollisionRecs(entity->rect, player->rect))
                {
                    // this projectile has collided with player
                    efs_EntitySetProperty(player, efs_prop_TempInvincible);
                    player->invincibleTimer = 1.0f;
                    player->health -= 3;
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

    BeginDrawing();
        DrawRectangle(0, 0, screenWidth, screenHeight, GREEN);
        DrawText("TITLE SCREEN", 20, 20, 40, DARKGREEN);
        DrawText("PRESS ANY KEY TO START", 120, 220, 20, DARKGREEN);
    EndDrawing();
}

static bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

s32 ParseScoresLine(ScoreInfo* info, char* buf, u32 len)
{
    char* comma = memchr(buf, ',', len);
    if (!comma)
    {
        return 1;
    }

    u32 usernameLen = comma - buf;

    if (usernameLen >= sizeof(info->username))
    {
        BSD_ERR("Username too long!");
        return 1;
    }

    char* curPos = comma+1;
    u32 score = 0;
    while (IsDigit(*curPos))
    {
        score *= 10;
        score += *curPos++ - '0';
    }
    memcpy(info->username, buf, usernameLen);
    info->score = score;
    return 0;
}

void GameoverLoadScores(GameoverData* data)
{
    data->gotScores = false;
    http_Connection* conn = {}; // fix this mem leak
    http_Error err = http_ConnectionCreate(&conn);
    DBG_ASSERT_MSG(err == http_Success, "Connection setup failed");
    http_Request req = {};
    req.method = http_MethodGET;
    req.body.str = NULL;
    req.port = 8080;
    strcpy(req.hostName, "192.168.1.7");
    strcpy(req.hostURL, "/v1/hiscores");
    http_Response resp = {};
    err = http_ReqAndWaitForResp(conn, &req, &resp);
    if (err)
    {
        BSD_ERR("HTTP req failed with error %d", err);
    }
    else if (resp.status != 200)
    {
        BSD_ERR("HTTP response returned non-success status code: %d", resp.status);
    }
    else
    {
        // got response, will parse csv body into ScoreInfo

        char* curPos = resp.content.str;
        u32 remaining = resp.content.len;
        
        u32 scoreIndex = 0;
        while (curPos < curPos + resp.content.len && scoreIndex < 10)
        {
            char* nextLine = memchr(curPos, '\n', remaining);
            if (!nextLine)
            {
                BSD_WARN("Possibility of truncated line");
                break;
            }

            // move to start of next line
            nextLine += 1;

            u32 lineLen = nextLine - curPos;

            ParseScoresLine(&data->topScores[scoreIndex], curPos, lineLen);

            scoreIndex++;
            curPos = nextLine;
            data->gotScores = true;
        }
        http_ResponseFree(&resp);
    }
    http_ConnectionClose(conn);

    if (data->gotScores)
    {
        data->state = GameoverState_ShowScores;
    }
    else
    {
        data->state = GameoverState_NoScores;
    }
}

void GameoverShowScores(GameoverData* data)
{
    int key = GetKeyPressed();
    if (key != 0)
    {
        core_GameMemoryGet()->menuState = MenuState_Title;
    }
    BeginDrawing();
        ClearBackground(BLUE);
        const int startX = 100;
        const int startY = 120;
        const int rowHeight = 32;

        const int nameX = startX;
        const int scoreX = startX + 300;

        DrawText("Game Over!", startX, 60, 40, RAYWHITE);

        for (u32 i = 0; i < ArrayCount(data->topScores); i++)
        {
            int y = startY + i * rowHeight;

            // username
            DrawText(data->topScores[i].username, nameX, y, 24, WHITE);

            // score
            DrawText(TextFormat("%u", data->topScores[i].score), scoreX, y, 24, YELLOW);
        }
        const int userScoreY = startY + ArrayCount(data->topScores) * rowHeight;
        DrawText(data->userScore.username, nameX, userScoreY, 24, ORANGE);
        DrawText(TextFormat("%u", data->userScore.score), scoreX, userScoreY, 24, YELLOW);

    EndDrawing();
}

void GameoverScreenNoScores(GameoverData* data)
{
    int key = GetKeyPressed();
    if (key != 0)
    {
        core_GameMemoryGet()->menuState = MenuState_Title;
    }
    BeginDrawing();
        ClearBackground(BLUE);
        DrawText("GAME OVER", 300, 200, 60, DARKBLUE);
        DrawText(TextFormat("Score: %d seconds", data->userScore.score), 200, 300, 40, GREEN);
        DrawText("PRESS ANY KEY TO RETURN TO TITLE SCREEN", 120, 350, 20, DARKBLUE);
    EndDrawing();
}

#define MAX_INPUT_CHARS 10
void GameoverInputScore(GameoverData* data)
{
    static int framesCounter = 0;
    int key = GetCharPressed();
    char* name = data->userScore.username;
    int screenWidth = GetScreenWidth();
    Rectangle textBox = { screenWidth/2.0f - 100, 180, 225, 50 };

    while (key > 0)
    {
        if (key >= 32 && key <= 125 && data->usernameLen < MAX_INPUT_CHARS)
        {
            name[data->usernameLen] = (char)key;
            name[data->usernameLen+1] = 0;
            data->usernameLen++;
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE))
    {
        data->usernameLen--;
        if (data->usernameLen < 0) data->usernameLen = 0;
        name[data->usernameLen] = 0;
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        // username should already be in the userScore struct
        data->userScore.score = (int)core_GameMemoryGet()->levelTimer * 100;
        data->state = GameoverState_LoadingScore;
    }

    BeginDrawing();
        ClearBackground(BLUE);
        DrawText("Game Over!", 100, 60, 40, RAYWHITE);
        DrawText("Enter your username to record your score!", 200, 150, 20, RAYWHITE);
        DrawRectangleRec(textBox, LIGHTGRAY);
        DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
        DrawText(name, (int)textBox.x + 5, (int)textBox.y + 8, 40, MAROON);
        if (data->usernameLen < MAX_INPUT_CHARS)
        {
            // Draw blinking underscore char
            if (((framesCounter/20)%2) == 0) DrawText("_", (int)textBox.x + 8 + MeasureText(name, 40), (int)textBox.y + 12, 40, MAROON);
        }
    EndDrawing();
}

void UpdateGameoverData(GameoverData* data)
{
    switch(data->state)
    {
        case GameoverState_InputScore:
        {
            GameoverInputScore(data);
        } break;
        case GameoverState_LoadingScore:
        {
            GameoverLoadScores(data);
        } break;
        case GameoverState_ShowScores:
        {
            GameoverShowScores(data);
        } break;
        case GameoverState_NoScores:
            GameoverScreenNoScores(data);
        break;
    }
}

void GameoverLoadingUpdate(soc_GameMemory* memory)
{
    (void)memory;
    // BeginDrawing();
    //     DrawRectangle(0, 0, screenWidth, screenHeight, BLUE);
    //     DrawText("GAME OVER", 300, 200, 60, DARKBLUE);
    //     DrawText(TextFormat("Time survived: %.1f seconds", memory->levelTimer), 200, 300, 40, GREEN);
    //     DrawText("PRESS ANY KEY TO RETURN TO TITLE SCREEN", 120, 350, 20, DARKBLUE);
    // EndDrawing();
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
        case MenuState_GameOverLoading:
        {
            GameoverLoadingUpdate(memory);
        } break;
        case MenuState_GameOver:
        {
            UpdateGameoverData(&memory->gameoverData);
        } break;
    }

}