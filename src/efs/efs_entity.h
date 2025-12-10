#pragma once
#include "raylib.h"
#include "based_basic.h"
#include <assert.h>

#define PROPERTY_MAX_NUM 256
#define NUM_PROPS_IN_U64 64
static_assert(PROPERTY_MAX_NUM % NUM_PROPS_IN_U64 == 0, "needs to be a multiple of 64");
typedef u64 efs_Properties[PROPERTY_MAX_NUM / NUM_PROPS_IN_U64];

typedef enum {
    efs_prop_CanMove,
    efs_prop_HasHealth,
    efs_prop_FollowsOther,
    efs_prop_PlayerControlled,
} efs_PropertyType;

typedef struct efs_Entity {
    efs_Properties props;
    int next;
    int prev;
    Rectangle pos;
    Vector2 vel;
    int health;
    struct efs_Entity* following; // watch out for dangling references
    Texture2D texture;
} efs_Entity;


bool efs_EntityHasProperty(efs_Entity const* entity, efs_PropertyType prop);

void efs_SetEntityProperty(efs_Entity const* entity, efs_PropertyType prop);