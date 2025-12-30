#include "raylib.h"
#include <assert.h>
#include "based_basic.h"
#include "based_arena.h"

#define MEGABYTE (1024*1024)
#define DEFAULT_ARENA_CAPACITY (2*MEGABYTE)

void bsd_ArenaInit(bsd_Arena* arena, u64 capacity)
{
    DBG_ASSERT_MSG(arena->data == NULL, "You are initialising an arena with pre-existing data!");

    arena->data = MemAlloc(capacity);
    arena->capacity = capacity;
    arena->used = 0;
}

void* bsd_ArenaAlloc(bsd_Arena* arena, u64 size)
{
    // zero is initialisation
    if (arena->data == NULL)
    {
        arena->data = MemAlloc(DEFAULT_ARENA_CAPACITY);
        arena->capacity = DEFAULT_ARENA_CAPACITY;
    }

    assert(arena->used + size <= arena->capacity);

    void* pdata = (unsigned char*)arena->data + arena->used;
    arena->used += size;
    return pdata;
}

void bsd_ArenaPop(bsd_Arena* arena, u64 size)
{
    if (arena->used < size)
    {
        arena->used = 0;
    }
    else
    {
        arena->used -= size;
    }
}

void bsd_ArenaRelease(bsd_Arena* arena)
{
    arena->used = 0;
}

void bsd_ArenaDestroy(bsd_Arena* arena)
{
    if (arena->data)
    {
        MemFree(arena->data);
    }
    arena->used = 0;
    arena->capacity = 0;
}