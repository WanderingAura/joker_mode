#include "ecs.h"

#define MAX_ENTITIES 65536
static ECS_EntityID ids[MAX_ENTITIES];

static int freeIds[MAX_ENTITIES];

static ECS_Entity entity[MAX_ENTITIES];

static void ecs_init()
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        freeIds[i] = i;
    }
}

static ECS_Entity* ecs_entity_create()
{
}