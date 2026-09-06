#include "efs_entity.h"
#include "efs_entity_props.h"
#include "raylib.h"

efs_Entity CreateWall(Rectangle rect, Texture2D texture)
{
    efs_Entity wall = {};
    efs_EntitySetProperty(&wall, efs_prop_Solid);
    wall.rect = rect;
    wall.texture = texture;
    return wall;
}

// rect.x/y is a top-left corner throughout the entity system (matches DrawTexturePro and
// CheckCollisionRecs), so each wall is laid out from its top-left corner, not its center.
void CreateRoomWalls(efs_EntityPool* pool, Vector2 min, Vector2 max, f32 thickness, Texture2D texture)
{
    f32 width = max.x - min.x;
    f32 height = max.y - min.y;

    Rectangle top = {min.x - thickness, min.y - thickness, width + 2.0f * thickness, thickness};
    Rectangle bottom = {min.x - thickness, max.y, width + 2.0f * thickness, thickness};
    Rectangle left = {min.x - thickness, min.y, thickness, height};
    Rectangle right = {max.x, min.y, thickness, height};

    efs_PoolAdd(pool, CreateWall(top, texture));
    efs_PoolAdd(pool, CreateWall(bottom, texture));
    efs_PoolAdd(pool, CreateWall(left, texture));
    efs_PoolAdd(pool, CreateWall(right, texture));
}