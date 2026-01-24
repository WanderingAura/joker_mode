#include <raylib.h>

#include "core_tilemap.h"
#include "based_basic.h"

void core_TilemapInit(core_Tilemap* tilemap, Vector2 pos, u32 columns, u32 rows, Texture2D texture)
{
    tilemap->texture = texture;
    tilemap->rows = rows;
    tilemap->columns = columns;
    tilemap->pos = pos;
    tilemap->tileHeight = tilemap->texture.height;
    tilemap->tileWidth = tilemap->texture.width;
    SetTextureWrap(tilemap->texture, TEXTURE_WRAP_REPEAT);
}

void core_TilemapUpdate(core_Tilemap* tilemap, const Camera2D* cam)
{
    int screenHeight = GetScreenHeight();
    int screenWidth = GetScreenWidth();

    // TODO: 16 should not be hard coded, this should be the tilewidth maybe?
    int tileMapX = (((int)cam->target.x - screenWidth/2 )/16) * 16 - 32;
    int tilemapY = (((int)cam->target.y - screenHeight/2)/16) * 16 - 32;
    tilemap->pos.x = tileMapX;
    tilemap->pos.y = tilemapY;
}

void core_TilemapDraw(core_Tilemap* tilemap)
{
    DBG_ASSERT_MSG(tilemap, "The tilemap being drawn is NULL!");

    // DrawTextureV(tilemap->texture, tilemap->pos, WHITE);
    Rectangle tileRect = {tilemap->pos.x, tilemap->pos.y, tilemap->tileWidth * tilemap->columns, tilemap->tileHeight * tilemap->rows};
    DrawTexturePro(tilemap->texture, tileRect, tileRect, (Vector2){0,0}, 0.0f, WHITE);
}
