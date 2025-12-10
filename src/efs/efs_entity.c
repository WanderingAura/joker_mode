#include "efs_entity.h"

#define entityPoolSize 100

int freeHead = 0;
int activeHead = -1;
    
int addToPool(efs_Entity* pool, efs_Entity entity) {
    int nextFreeHead = pool[freeHead].next;
    if(nextFreeHead < 0) {
        printf("Pools closed due to aids, infected line: %d\n", __LINE__);
        //ran out of space
        return -1;
    }
    pool[nextFreeHead].prev = -1;

    entity.next = activeHead;
    entity.prev = -1;
    pool[activeHead].prev = freeHead;
    pool[freeHead] = entity;

    activeHead = freeHead;
    freeHead = nextFreeHead;
}

void deleteFromPool(efs_Entity* pool, int index) {
    //collect relevant active entities
    efs_Entity* entity = &pool[index];
    memset(&entity->props, 0, sizeof(entity->props));
    if(entity->next != -1) {
        efs_Entity* next = &pool[entity->next];
        next->prev = entity->prev;
    }
    if(entity->prev != -1) {
        efs_Entity* prev = &pool[entity->prev];    
        prev->next = entity->next;
    } else {
        activeHead = entity->next;
    }
    
    //sort out free list
    pool[freeHead].prev = index;
    entity->next = freeHead;
    entity->prev = -1;
    freeHead = index;
}

efs_Entity* initPool() {
    efs_Entity* entityPool = MemAlloc(sizeof(efs_Entity)*entityPoolSize);
    entityPool[0] = (efs_Entity){ 0 };
    entityPool[0].next = 1;
    entityPool[0].prev = -1;
    for(int i = 1; i < entityPoolSize - 1; i ++) {
        entityPool[i] = (efs_Entity){ 0 };
        entityPool[i].next = i+1;
        entityPool[i].prev = i-1;
    }
    entityPool[entityPoolSize-1].prev = entityPoolSize-2;
    entityPool[entityPoolSize-1].next = -1;

    int poolNextFree = 1;
    int poolFirstUsed = 0;
}
      


bool efs_EntityHasProperty(efs_Entity const* entity, efs_PropertyType prop)
{
    u64 propLoc = entity->props[prop/NUM_PROPS_IN_U64];

    return propLoc & (1 << (prop % NUM_PROPS_IN_U64));
}

void efs_SetEntityProperty(efs_Entity *entity, efs_PropertyType prop) {
    entity->props[prop/NUM_PROPS_IN_U64] = 1;
}