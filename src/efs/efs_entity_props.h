#pragma once
#include <assert.h>
#include <based_basic.h>

#define PROPERTY_MAX_NUM 256
#define NUM_PROPS_IN_U64 64

static_assert(PROPERTY_MAX_NUM % NUM_PROPS_IN_U64 == 0, "needs to be a multiple of 64");
typedef u64 efs_Properties[PROPERTY_MAX_NUM / NUM_PROPS_IN_U64];

typedef enum {
    efs_prop_CanMove, // simple dir*speed movement; unused by current templates but kept as a generic mover
    efs_prop_HasHealth,
    efs_prop_PlayerControlled, // also updates movement of player
    efs_prop_HasLifetime,
    efs_prop_HasRotation,
    efs_prop_Collidable,
    efs_prop_CanDamage,
    efs_prop_Spawner,
    efs_prop_CanDodge,
    efs_prop_TempInvincible, //removes efs_prop_Invincible after time passes
    efs_prop_Invincible,
    efs_prop_DamageFlash, // purely cosmetic "just got hit" marker - distinct from efs_prop_Invincible so sources of invincibility that aren't a hit (e.g. dodging) don't flash
    efs_prop_DespawnWhenFarFromPlayer,
    efs_prop_ScalesWithDifficulty,
    efs_prop_Solid, // stops the player from moving through it
    efs_prop_ShootsAtTarget, // player: aims at the mouse; anything else: aims at `target`
    efs_prop_ParametricMovement, // per-frame position update via MovementType_Parametric/Velocity (bullet.c)
} efs_PropertyType;