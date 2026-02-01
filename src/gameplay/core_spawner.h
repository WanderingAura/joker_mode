#include "core_projectile.h"
#include "efs_entity.h"

typedef struct {
    efs_Entity* template;
    Vector2 offset;
    Vector2 dir;
} SpawnedEntity;

void SpawnerCreate(SpawnerType type, Vector2 pos, Vector2 dir, ProjectileType spawnedProjectileType);