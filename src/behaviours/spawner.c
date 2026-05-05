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

int handle_spawner(efs_Entity* entity, soc_GameMemory* memory) {
    if (efs_EntityHasProperty(entity, efs_prop_Spawner))
    {
        entity->timeSinceLastSpawn += GetFrameTime();
        if (entity->timeSinceLastSpawn >= entity->spawnTime)
        {
            entity->timeSinceLastSpawn = 0;
            efs_Entity spawned = {0};
            memcpy(&spawned, entity->childInfo.template, sizeof(efs_Entity));
            spawned.dir = entity->childInfo.initialDir;
            spawned.pos = Vector2Add(entity->pos, entity->childInfo.offset);
            efs_PoolAdd(&memory->efs_entityPool, spawned);
        }
    }
    return 0;
}