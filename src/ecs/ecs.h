#if 0
#pragma once

#include "based_basic.h"

typedef struct
{
    u32 id;
    u32 gen;
} ECS_EntityID;

#define BIT(x) (1 << (x))

typedef enum
{
    ECS_BehavioursMoving = BIT(0),
    ECS_BehavioursPlayerControlled = BIT(1),
    ECS_BehavioursPhysics = BIT(2)
} ECS_Behaviours;

typedef u64 ECS_BehaviourFlags

typedef enum
{
    ECS_BoundaryTypeSphere,
    ECS_BoundaryTypeRect,
    ECS_BoundaryTypeCapsule,
} ECS_BoundaryType;

typedef struct ECS_Entity
{
    ECS_BehaviourFlags flags;

    f32 x;
    f32 y;
    f32 speed;
    f32 dir;

    struct ECS_Entity* next;
    struct ECS_Entity* prev;
    struct ECS_Entity* firstChild;
    struct ECS_Entity* lastChild;

    ECS_BoundaryType geometryType;

} ECS_Entity;
#endif