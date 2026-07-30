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

int handle_canDamage(efs_Entity* entity, soc_GameMemory* memory, int index) {
    int j = memory->efs_entityPool.activeHead;
    while(j >= 0) {
        efs_Entity* target = &memory->efs_entityPool.entities[j];
        if(j == index) {
            j = target->next;
        }
        if (efs_EntityHasProperty(target, efs_prop_HasHealth)
            && !efs_EntityHasProperty(target, efs_prop_TempInvincible)
            && efs_EntityHasProperty(entity, efs_prop_CanDamage)                
            && entity->canDamage == target->damageGroup) {
            if (target && CheckCollisionRecs(entity->rect, target->rect)) {
                // this projectile has collided with player
                efs_EntitySetProperty(target, efs_prop_TempInvincible);
                target->invincibleTimer = 3.0f;
                target->health -= entity->damage;

            }
        }
        j = target->next;
    }
    return 0;
}