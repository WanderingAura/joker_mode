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

void handle_dodge(efs_Entity* entity) {
  if(!efs_EntityHasProperty(entity, efs_prop_CanDodge)) {
    return;
  }
  if(entity->dodgeTimer >= 0) {
    efs_EntitySetProperty(entity, efs_prop_Invincible); //should be reduntant but could get unset mid dodge from some other logic
    float deltaTime = GetFrameTime();
    entity->pos = Vector2Add(entity->pos, Vector2Scale(entity->dodgeDirection, entity->baseMoveSpeed*DODGE_SPEED_SCALE*deltaTime));
    entity->dodgeTimer -= deltaTime;
    if(entity->dodgeTimer <= 0) {
      //should we be removing this? might be getting set from somewhere else?
      efs_EntityUnsetProperty(entity, efs_prop_Invincible);
      entity->dodgeCooldown = DODGE_COOLDOWN;
    }
    return;
  }
  if(entity->dodgeCooldown >= 0) {
    entity->dodgeCooldown -= GetFrameTime();
  }
}