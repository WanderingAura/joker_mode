#pragma once

#include <raylib.h>
#include "based_basic.h"

typedef struct {
    Vector2 pos;
    u32 columns;
    u32 rows;
    u32 tileWidth;
    u32 tileHeight;
    Texture2D texture;
} core_Tilemap;

void core_TilemapInit(core_Tilemap* tilemap, Vector2 pos, u32 columns, u32 rows, Texture2D texture);
void core_TilemapDraw(core_Tilemap* tilemap);