#include "based_basic.h"

typedef struct {
    void* data;
    u64 capacity;
    u64 used;
} bsd_Arena;

void bsd_ArenaInit(bsd_Arena* arena, u64 capacity);
void* bsd_ArenaAlloc(bsd_Arena* arena, u64 size);
void bsd_ArenaPop(bsd_Arena* arena, u64 size);
void bsd_ArenaRelease(bsd_Arena* arena);
void bsd_ArenaDestroy(bsd_Arena* arena);