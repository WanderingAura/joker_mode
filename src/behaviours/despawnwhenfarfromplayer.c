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

int handle_despawnWhenFarFromPlayer(efs_Entity* entity, soc_GameMemory* memory, efs_Entity* player, int index) {
    if (efs_EntityHasProperty(entity, efs_prop_DespawnWhenFarFromPlayer))
    {
        DBG_ASSERT_MSG(entity->despawnDistance > 0, "Got %f despawn distance. Entity with this property should have >0 despawn distance");
        float distanceToPlayer = Vector2Distance(entity->pos, player->pos);
        if (distanceToPlayer > entity->despawnDistance)
        {
            efs_PoolDelete(&memory->efs_entityPool, index);
        }
    }
    return 0;
}