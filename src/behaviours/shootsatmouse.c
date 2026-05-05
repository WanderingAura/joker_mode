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

int handle_shootAtMouse(efs_Entity* entity, soc_GameMemory* memory) {
    if(efs_EntityHasProperty(entity, efs_prop_ShootsAtMouse)) {
        entity->attackCoolDown -= GetFrameTime();
        if(entity->attackCoolDown <= 0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            
            entity->attackCoolDown = entity->attackSpeed;
            efs_Entity bullet = {0};
            memcpy(&bullet, entity->childInfo.template, sizeof(efs_Entity));
            bullet.canDamage = EnemyGroup;
            bullet.dir = (Vector2){1, 0};
            bullet.pos = entity->pos;
            efs_PoolAdd(&memory->efs_entityPool, bullet);
        }
    }
    return 0;
}