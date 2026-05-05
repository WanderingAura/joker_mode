#pragma once
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

#include "prop_behaviours.h"
#include "efs_entity.h"
#include "core_game_memory.h"

int handle_solid(efs_Entity* entity, efs_Entity* player) {
    if (efs_EntityHasProperty(entity, efs_prop_Solid) && player)
    {
        if (CheckCollisionRecs(entity->rect, player->rect))
        {
            player->pos.x += lvl_CollisionAdjust(player->pos.x, player->rect.width, entity->pos.x, entity->rect.width);
            player->pos.y += lvl_CollisionAdjust(player->pos.y, player->rect.height, entity->pos.y, entity->rect.height);
        }
    }
    return 0;
}