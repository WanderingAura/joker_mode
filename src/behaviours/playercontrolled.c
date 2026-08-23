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
#include "core_texture_types.h"
#include "core_tilemap.h"
#include "core_entity_template.h"
#include "efs_entity.h"
#include "efs_entity_props.h"
#include "gameover.h"
#include "render_font.h"
#include "lvl_collision.h"
#include "core_wall.h"
#include "input.h"

#include "prop_behaviours.h"
#include "efs_entity.h"
#include "core_game_memory.h"

void MoveAndResolveCollisions(efs_Entity* player, efs_EntityPool* pool)
{
    float stepAmount = player->baseMoveSpeed * GetFrameTime();
    Vector2 entityStep = Vector2Scale(player->dir, stepAmount);

    player->pos.x += entityStep.x;

    int entityIdx = pool->activeHead;
    while(entityIdx >= 0) {
        efs_Entity* wall = &pool->entities[entityIdx];
        if (efs_EntityHasProperty(wall, efs_prop_Solid))
        {
            if (CheckCollisionRecs(wall->rect, player->rect))
            {
                player->pos.x += lvl_CollisionAdjust(player->pos.x, player->rect.width, wall->pos.x, wall->rect.width);
            }
        }
        entityIdx = wall->next;
    }
    player->pos.y += entityStep.y;
    while(entityIdx >= 0) {
        efs_Entity* wall = &pool->entities[entityIdx];
        if (efs_EntityHasProperty(wall, efs_prop_Solid))
        {
            if (CheckCollisionRecs(wall->rect, player->rect))
            {
                player->pos.y += lvl_CollisionAdjust(player->pos.y, player->rect.height, wall->pos.y, wall->rect.height);
            }
        }
        entityIdx = wall->next;
    }
}

bool AbleToDodge(efs_Entity* entity)
{
    return IsButtonsPressed(DodgeButton) &&
        entity->dodgeCooldown <= 0 &&
        Vector2LengthSqr(entity->dir) != 0;
}

int handle_playerControlled(efs_Entity* entity, soc_GameMemory* memory) {
    if(efs_EntityHasProperty(entity, efs_prop_PlayerControlled)) {
        if(entity->dodgeTimer >= 0) {
            //prevent move while in roll
            return 0;
        }
        entity->dir = GetInputDir();
        if(AbleToDodge(entity)) {
            entity->dodgeDirection = entity->dir;
            entity->dodgeTimer = DODGE_DURATION;
        } else {
            MoveAndResolveCollisions(entity, &memory->efs_entityPool);
        }
    }
    return 0;
}