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

int handle_canDamage(efs_Entity* entity, soc_GameMemory* memory, int index);
int handle_canMove(efs_Entity* entity, soc_GameMemory* memory);
int handle_despawnWhenFarFromPlayer(efs_Entity* entity, soc_GameMemory* memory, efs_Entity* player, int index);
int handle_lifetime(efs_Entity* entity, soc_GameMemory* memory, int index);
int handle_hasRotation(efs_Entity* entity);
int handle_playerControlled(efs_Entity* entity, soc_GameMemory* memory);
int handle_shootAtMouse(efs_Entity* entity, soc_GameMemory* memory);
int handle_solid(efs_Entity* entity, efs_Entity* player);
int handle_spawner(efs_Entity* entity, soc_GameMemory* memory);
int handle_tempInvincible(efs_Entity* entity);