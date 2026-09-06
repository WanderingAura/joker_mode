#pragma once
#include "efs_entity.h"

efs_Entity CreateWall(Rectangle rect, Texture2D texture);
// Encloses [min, max] in four solid wall entities (one per side, corners overlapped so
// there are no gaps) and adds them to `pool`.
void CreateRoomWalls(efs_EntityPool* pool, Vector2 min, Vector2 max, f32 thickness, Texture2D texture);