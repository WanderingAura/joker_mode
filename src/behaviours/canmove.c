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

void EntityMove(efs_Entity* entity, float modifier)
{
    float stepAmount = entity->baseMoveSpeed * GetFrameTime();
    if (efs_EntityHasProperty(entity, efs_prop_ScalesWithDifficulty))
    {
        stepAmount *= 1 + modifier;
    }
    Vector2 entityStep = Vector2Scale(entity->dir, stepAmount);
    entity->pos.x += entityStep.x;
    entity->pos.y += entityStep.y;
}

int handle_canMove(efs_Entity* entity, soc_GameMemory* memory) {
    // if not a player we update it normally
    if(efs_EntityHasProperty(entity, efs_prop_CanMove) && !efs_EntityHasProperty(entity, efs_prop_PlayerControlled)) {
        EntityMove(entity, memory->levelTimer/20.0f);
    }
    return 0;
}