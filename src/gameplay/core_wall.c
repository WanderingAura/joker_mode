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