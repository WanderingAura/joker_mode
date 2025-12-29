#include "efs_entity.h"
#include <stdio.h>
#include <string.h>


int efs_PoolAdd(efs_EntityPool* entityPool, efs_Entity entity) {
    int nextFreeHead = entityPool->entities[entityPool->freeHead].next;
    if(nextFreeHead < 0) {
        printf("Pools closed due to aids, infected line: %d\n", __LINE__);
        //ran out of space
        return -1;
    }
    
    entityPool->entities[nextFreeHead].prev = -1;

    entity.next = entityPool->activeHead;
    entity.prev = -1;
    entityPool->entities[entityPool->activeHead].prev = entityPool->freeHead;
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

efs_EntityPool* efs_PoolInit() {
    efs_Entity* entitites = MemAlloc(sizeof(efs_Entity)*entityPoolSize);

    efs_EntityPool* entityPool = MemAlloc(sizeof(efs_EntityPool));
    *entityPool = (efs_EntityPool){ .freeHead = 0, .activeHead = -1, .entities = entitites };
    
    entitites[0] = (efs_Entity){ 0 };
    entitites[0].next = 1;
    entitites[0].prev = -1;
    for(int i = 1; i < entityPoolSize - 1; i ++) {
        entitites[i] = (efs_Entity){ 0 };
        entitites[i].next = i+1;
        entitites[i].prev = i-1;
    }
    entitites[entityPoolSize-1].prev = entityPoolSize-2;
    entitites[entityPoolSize-1].next = -1;

    return entityPool;
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

void efs_EntitySetProperties(efs_Entity* entity, efs_PropertyType* props, u32 numProps)
{
    assert(entity != NULL);
    assert(numProps <= PROPERTY_MAX_NUM);

    for (int i = 0; i < numProps; i++)
    {
        efs_EntitySetProperty(entity, props[i]);
    }
}