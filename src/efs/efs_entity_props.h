#pragma once
#include <assert.h>
#include <based_basic.h>

#define PROPERTY_MAX_NUM 256
#define NUM_PROPS_IN_U64 64

static_assert(PROPERTY_MAX_NUM % NUM_PROPS_IN_U64 == 0, "needs to be a multiple of 64");
typedef u64 efs_Properties[PROPERTY_MAX_NUM / NUM_PROPS_IN_U64];

typedef enum {
    efs_prop_CanMove, // for movement of non-player entities
    efs_prop_HasHealth,
    efs_prop_FollowsOther,
    efs_prop_PlayerControlled, // also updates movement of player
    efs_prop_HasLifetime,
    efs_prop_HasRotation,
    efs_prop_Collidable,
    efs_prop_DamagesPlayer,
    efs_prop_Spawner,
    efs_prop_MovesBetweenTwoPoints,
    efs_prop_TempInvincible,
    efs_prop_DespawnWhenFarFromPlayer,
    efs_prop_ScalesWithDifficulty,
    efs_prop_SolidWall,
} efs_PropertyType;