#include "efs_entity.h"
#include <stdio.h>
#include <string.h>
#include "based_basic.h"


int efs_PoolAdd(efs_EntityPool* entityPool, efs_Entity entity) {
    int nextFreeHead = entityPool->entities[entityPool->freeHead].next;
    if(nextFreeHead < 0) {
        printf("Pools closed due to aids, infected line: %d\n", __LINE__);
        //ran out of space
        return -1;
    }
    
    DBG_ASSERT_MSG(nextFreeHead >= 0 && nextFreeHead <= ENTITY_POOL_SIZE, "Out of bounds");
    entityPool->entities[nextFreeHead].prev = -1;

    entity.next = entityPool->activeHead;
    entity.prev = -1;
    if (entityPool->activeHead >= 0)
    {
        entityPool->entities[entityPool->activeHead].prev = entityPool->freeHead;
    }
    DBG_ASSERT_MSG(entityPool->freeHead >= 0 && entityPool->freeHead <= ENTITY_POOL_SIZE, "Out of bounds");
    entityPool->entities[entityPool->freeHead] = entity;

    entityPool->activeHead = entityPool->freeHead;
    entityPool->freeHead = nextFreeHead;

    return 0;
}

void efs_PoolDelete(efs_EntityPool* entityPool, int index) {
    //collect relevant active entities
    efs_Entity* entity = &entityPool->entities[index];
    memset(&entity->props, 0, sizeof(entity->props));
    if(entity->next != -1) {
        efs_Entity* next = &entityPool->entities[entity->next];
        next->prev = entity->prev;
    }
    if(entity->prev != -1) {
        efs_Entity* prev = &entityPool->entities[entity->prev];    
        prev->next = entity->next;
    } else {
        entityPool->activeHead = entity->next;
    }
    
    //sort out free list
    entityPool->entities[entityPool->freeHead].prev = index;
    entity->next = entityPool->freeHead;
    entity->prev = -1;
    entityPool->freeHead = index;
}

void efs_PoolInit(efs_EntityPool* pool) {
    memset(pool, 0, sizeof(*pool));

    pool->freeHead = 0;
    pool->activeHead = -1;
    
    pool->entities[0] = (efs_Entity){ 0 };
    pool->entities[0].next = 1;
    pool->entities[0].prev = -1;
    int poolSize = ArrayCount(pool->entities);
    for(int i = 1; i < poolSize - 1; i++) {
        pool->entities[i] = (efs_Entity){ 0 };
        pool->entities[i].next = i+1;
        pool->entities[i].prev = i-1;
    }
    pool->entities[poolSize-1].prev = poolSize-2;
    pool->entities[poolSize-1].next = -1;
}

bool efs_EntityHasProperty(efs_Entity const* entity, efs_PropertyType prop)
{
    assert(entity != NULL);
    u64 propLoc = entity->props[prop/NUM_PROPS_IN_U64];

    return propLoc & (1 << (prop % NUM_PROPS_IN_U64));
}

void efs_EntitySetProperty(efs_Entity *entity, efs_PropertyType prop) {
    assert(entity != NULL);
    entity->props[prop/NUM_PROPS_IN_U64] |= (1 << (prop % NUM_PROPS_IN_U64));
}

void efs_EntityUnsetProperty(efs_Entity *entity, efs_PropertyType prop) {
    assert(entity != NULL);
    entity->props[prop/NUM_PROPS_IN_U64] &= ~(1 << (prop % NUM_PROPS_IN_U64));
}

void efs_EntitySetProperties(efs_Entity* entity, efs_PropertyType* props, u32 numProps)
{
    assert(entity != NULL);
    assert(numProps <= PROPERTY_MAX_NUM);

    for (int i = 0; i < numProps; i++)
    {
        efs_EntitySetProperty(entity, props[i]);
    }
}