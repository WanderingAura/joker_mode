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
#include "bullet.h"

// Player aims at the mouse; anything else (e.g. an enemy) aims at its `target` entity,
// set at spawn time (see EnemyEntityCreate). Both share one cooldown-gated firing path.
int handle_shootAtTarget(efs_Entity* entity, soc_GameMemory* memory) {
    if (!efs_EntityHasProperty(entity, efs_prop_ShootsAtTarget)) {
        return 0;
    }

    entity->curAttackCooldown -= GetFrameTime();
    if (entity->curAttackCooldown > 0) {
        return 0;
    }

    bool isPlayer = efs_EntityHasProperty(entity, efs_prop_PlayerControlled);
    Vector2 targetPos;
    if (isPlayer) {
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            return 0;
        }
        targetPos = GetScreenToWorld2D(GetMousePosition(), memory->camera);
    } else if (entity->target != NULL) {
        targetPos = efs_EntityCenter(entity->target);
    } else {
        return 0;
    }

    entity->curAttackCooldown = entity->attackCooldown;

    // stats/appearance/move-pattern shape come from the template; only where it's fired
    // from and which way it's aimed vary per shot.
    efs_Entity bullet = *entity->childInfo.template;
    bullet.canDamage = (entity->damageGroup == PlayerGroup) ? EnemyGroup : PlayerGroup;

    Vector2 shooterCenter = efs_EntityCenter(entity);
    // spawn so the bullet's own center (not its top-left corner) starts at the shooter's center
    Vector2 origin = Vector2Subtract(shooterCenter, (Vector2){bullet.rect.width / 2.0f, bullet.rect.height / 2.0f});
    bullet_orient(&bullet, origin, bullet_rotation_to(shooterCenter, targetPos));
    efs_PoolAdd(&memory->efs_entityPool, bullet);

    return 0;
}