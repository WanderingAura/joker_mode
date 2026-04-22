#pragma once
#include "raylib.h"
#include "efs_entity.h"
#include "core_tilemap.h"
#include "core_texture_types.h"
#include "core_entity_types.h"
#include "core_menu_state.h"
#include "render_font_types.h"

typedef struct
{
    efs_Entity projectile[ProjectileTypeCount];
    efs_Entity spawner[SpawnerTypeCount];
} EntityTemplateTables;

typedef struct
{
    Vector2 min;
    Vector2 max;
} BoundingRect;

typedef struct
{
    char username[16];
    u32 score;
} ScoreInfo;

typedef struct
{
    ScoreInfo topScores[10];
    ScoreInfo userScore;
} Scoreboard;

typedef enum
{
    GameoverState_InputScore,
    GameoverState_LoadingScore,
    GameoverState_ShowScores,
    GameoverState_NoScores,
} GameoverState;

typedef struct
{
    Scoreboard scoreboard;
    int usernameLen;
    bool gotScores;

    GameoverState state;
} GameoverData;

typedef struct {
    core_Tilemap tilemap;
    Texture2D textures[TextureTypeCount];
    Font fonts[FontTypeCount];
    efs_EntityPool efs_entityPool;
    efs_Entity* player;
    Camera2D camera;
    BoundingRect levelBounds;
    float levelTimer;
    EntityTemplateTables entityTemplates;
    GameMenuState menuState;

    GameoverData gameoverData;
} soc_GameMemory;

soc_GameMemory* core_GameMemoryGet();
void core_GameMemorySet(soc_GameMemory* memory);