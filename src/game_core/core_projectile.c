#include "core_game_memory.h"
#include "efs_entity.h"
#include "raylib.h"
#include "core_projectile.h"
#include "based_logging.h"

#include <string.h>

#define PROJ_SIZE 16

// must be initialised after the textures have been initialised
void ProjectileSystemInit(soc_GameMemory* memory)
{
    memset(&memory->entityTemplates.projectile, 0, sizeof(memory->entityTemplates.projectile));

    efs_EntitySetProperty(&memory->entityTemplates.projectile[ProjectileNormal], efs_prop_CanMove);
    efs_EntitySetProperty(&memory->entityTemplates.projectile[ProjectileNormal], efs_prop_HasLifetime);
    efs_EntitySetProperty(&memory->entityTemplates.projectile[ProjectileNormal], efs_prop_Collidable);
    efs_EntitySetProperty(&memory->entityTemplates.projectile[ProjectileNormal], efs_prop_DamagesPlayer);
    memory->entityTemplates.projectile[ProjectileNormal].lifetime = 10.0f;
    memory->entityTemplates.projectile[ProjectileNormal].moveSpeed = 150.0f;
    memory->entityTemplates.projectile[ProjectileNormal].texture = memory->textures[TextureProjectile];
    memory->entityTemplates.projectile[ProjectileNormal].rect.width = PROJ_SIZE;
    memory->entityTemplates.projectile[ProjectileNormal].rect.height = PROJ_SIZE;


    efs_EntitySetProperty(&memory->entityTemplates.projectile[ProjectileCircle], efs_prop_CanMove);
    efs_EntitySetProperty(&memory->entityTemplates.projectile[ProjectileCircle], efs_prop_HasLifetime);
    efs_EntitySetProperty(&memory->entityTemplates.projectile[ProjectileCircle], efs_prop_Collidable);
    efs_EntitySetProperty(&memory->entityTemplates.projectile[ProjectileCircle], efs_prop_HasRotation);
    memory->entityTemplates.projectile[ProjectileCircle].lifetime = 20.0f;
    memory->entityTemplates.projectile[ProjectileCircle].moveSpeed = 150.0f;
    memory->entityTemplates.projectile[ProjectileCircle].rotationSpeed = 10.0f;
    memory->entityTemplates.projectile[ProjectileCircle].texture = memory->textures[TextureProjectile];
    memory->entityTemplates.projectile[ProjectileCircle].rect.width = PROJ_SIZE;
    memory->entityTemplates.projectile[ProjectileCircle].rect.height = PROJ_SIZE;

    memset(&memory->entityTemplates.spawner, 0, sizeof(memory->entityTemplates.spawner));
    efs_EntitySetProperty(&memory->entityTemplates.spawner[SpawnerNormal], efs_prop_Spawner);
    memory->entityTemplates.spawner[SpawnerNormal].texture = memory->textures[TextureProjectileSpawner];
    memory->entityTemplates.spawner[SpawnerNormal].spawnTime = 1.0f;
    memory->entityTemplates.spawner[SpawnerNormal].timeSinceLastSpawn = 1.0f;
    memory->entityTemplates.spawner[SpawnerNormal].rect.width = 32;
    memory->entityTemplates.spawner[SpawnerNormal].rect.height = 32;

    BSD_INF("projectile system initialised");
}

efs_Entity ProjectileEntityCreate(ProjectileType type, Vector2 pos, Vector2 dir)
{
    efs_Entity proj = core_GameMemoryGet()->entityTemplates.projectile[type];
    proj.dir = dir;
    proj.pos = pos;
    return proj;
}

efs_Entity ProjectileSpawnerCreate(SpawnerType type, Vector2 pos, Vector2 dir, ProjectileType spawnedProjectileType)
{
    EntityTemplateTables* templates = &core_GameMemoryGet()->entityTemplates;
    efs_Entity spawner = templates->spawner[type];
    spawner.dir = dir;
    spawner.pos = pos;
    // TODO: this is a bit scuffed currently. if we want the spawner to move and the spawned
    // spawned projectile have a relative position and velocity to the spawner, we need a struct
    // stored containing pos, dir and entityToSpawn (and possibly movespeed of spawner??)
    spawner.entityToSpawn = &templates->projectile[spawnedProjectileType];
    spawner.spawnedEntityDir = dir;
    return spawner;
}