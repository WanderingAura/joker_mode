#include "efs_entity.h"
#include <stdio.h>
#include <string.h>
#include "based_basic.h"
#include "based_core.h"


void efs_entity_list_init(efs_Entity* sentinel) {
    sentinel->next = sentinel;
    sentinel->prev = sentinel;
}

bool efs_entity_list_is_empty(const efs_Entity* sentinel) {
    return sentinel->next == sentinel;
}

// Inserts `node` immediately before `position` (position may be the sentinel
// itself, which inserts at the back of the list).
void efs_entity_list_insert_before(efs_Entity* position, efs_Entity* node) {
    node->prev = position->prev;
    node->next = position;
    position->prev->next = node;
    position->prev = node;
}

void efs_entity_list_push_back(efs_Entity* sentinel, efs_Entity* node) {
    efs_entity_list_insert_before(sentinel, node);
}

void efs_entity_list_push_front(efs_Entity* sentinel, efs_Entity* node) {
    efs_entity_list_insert_before(sentinel->next, node);
}

// Unlinks `node` from whatever list it's currently in. `node` must be
// re-inserted into a list before its next/prev fields are used again.
void efs_entity_list_remove(efs_Entity* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

// Unlinks and returns a node from the free list, or NULL if the pool is exhausted.
static efs_Entity* efs_PoolTakeFree(efs_EntityPool* entityPool) {
    if (efs_entity_list_is_empty(&entityPool->free_list)) {
        printf("Pools closed due to aids, infected line: %d\n", __LINE__);
        //ran out of space
        return NULL;
    }

    efs_Entity* node = entityPool->free_list.next;
    efs_entity_list_remove(node);
    return node;
}

efs_Entity* efs_PoolAdd(efs_EntityPool* entityPool, efs_Entity entity) {
    efs_Entity* node = efs_PoolTakeFree(entityPool);
    if (node == NULL) {
        return NULL;
    }

    *node = entity;
    efs_entity_list_push_back(&entityPool->active_list, node);

    return node;
}

efs_Entity* efs_PoolAlloc(efs_EntityPool* entityPool) {
    efs_Entity* node = efs_PoolTakeFree(entityPool);
    if (node == NULL) {
        return NULL;
    }

    *node = (efs_Entity){ 0 };
    efs_entity_list_push_back(&entityPool->active_list, node);

    return node;
}

void efs_PoolDelete(efs_EntityPool* entityPool, efs_Entity* entity) {
    memset(&entity->props, 0, sizeof(entity->props));
    efs_entity_list_remove(entity);
    efs_entity_list_push_back(&entityPool->free_list, entity);
}

void efs_PoolInit(efs_EntityPool* pool) {
    memset(pool, 0, sizeof(*pool));

    efs_entity_list_init(&pool->free_list);
    efs_entity_list_init(&pool->active_list);

    int poolSize = ArrayCount(pool->storage);
    for (int i = 0; i < poolSize; i++) {
        pool->storage[i] = (efs_Entity){ 0 };
        efs_entity_list_push_back(&pool->free_list, &pool->storage[i]);
    }
}

bool efs_EntityHasProperty(efs_Entity const* entity, efs_PropertyType prop)
{
    assert(entity != NULL);
    u64 propLoc = entity->props[prop/NUM_PROPS_IN_U64];

    return propLoc & (1ULL << (prop % NUM_PROPS_IN_U64));
}

void efs_EntitySetProperty(efs_Entity *entity, efs_PropertyType prop) {
    assert(entity != NULL);
    entity->props[prop/NUM_PROPS_IN_U64] |= (1ULL << (prop % NUM_PROPS_IN_U64));
}

void efs_EntityUnsetProperty(efs_Entity *entity, efs_PropertyType prop) {
    assert(entity != NULL);
    entity->props[prop/NUM_PROPS_IN_U64] &= ~(1ULL << (prop % NUM_PROPS_IN_U64));
}

void efs_EntitySetProperties(efs_Entity* entity, efs_PropertyType* props, u32 numProps)
{
    assert(entity != NULL);
    assert(numProps <= PROPERTY_MAX_NUM);

    for (u32 i = 0; i < numProps; i++)
    {
        efs_EntitySetProperty(entity, props[i]);
    }
}