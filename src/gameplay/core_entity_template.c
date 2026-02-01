#include "core_entity_types.h"
#include "core_game_memory.h"
#include "efs_entity.h"
#include "raylib.h"
#include "core_entity_template.h"
#include "based_logging.h"

#include <string.h>

#define PROJ_SIZE 16

static void ProjectileTemplatesInit(efs_Entity template_table[ProjectileTypeCount], const Texture2D textures[TextureTypeCount])
{
    memset(template_table, 0, sizeof(efs_Entity)*ProjectileTypeCount);

    efs_EntitySetProperty(&template_table[ProjectileNormal], efs_prop_CanMove);
    efs_EntitySetProperty(&template_table[ProjectileNormal], efs_prop_HasLifetime);
    efs_EntitySetProperty(&template_table[ProjectileNormal], efs_prop_Collidable);
    efs_EntitySetProperty(&template_table[ProjectileNormal], efs_prop_DamagesPlayer);
    template_table[ProjectileNormal].lifetime = 5.0f;
    template_table[ProjectileNormal].moveSpeed = 350.0f;
    template_table[ProjectileNormal].texture = textures[TextureProjectile];
    template_table[ProjectileNormal].rect.width = PROJ_SIZE;
    template_table[ProjectileNormal].rect.height = PROJ_SIZE;

    efs_EntitySetProperty(&template_table[ProjectileCircle], efs_prop_CanMove);
    efs_EntitySetProperty(&template_table[ProjectileCircle], efs_prop_HasLifetime);
    efs_EntitySetProperty(&template_table[ProjectileCircle], efs_prop_Collidable);
    efs_EntitySetProperty(&template_table[ProjectileCircle], efs_prop_HasRotation);
    efs_EntitySetProperty(&template_table[ProjectileCircle], efs_prop_DamagesPlayer);
    template_table[ProjectileCircle].lifetime = 5.0f;
    template_table[ProjectileCircle].moveSpeed = 300.0f;
    template_table[ProjectileCircle].rotationSpeed = 1.0f;
    template_table[ProjectileCircle].texture = textures[TextureProjectile];
    template_table[ProjectileCircle].rect.width = PROJ_SIZE;
    template_table[ProjectileCircle].rect.height = PROJ_SIZE;
}

static void SpawnerTemplatesInit(efs_Entity template_table[SpawnerTypeCount], const Texture2D textures[TextureTypeCount])
{
    memset(template_table, 0, sizeof(template_table[0])*SpawnerTypeCount);
    efs_EntitySetProperty(&template_table[SpawnerNormal], efs_prop_Spawner);
    template_table[SpawnerNormal].texture = textures[TextureProjectileSpawner];
    template_table[SpawnerNormal].spawnTime = 0.5f;
    template_table[SpawnerNormal].timeSinceLastSpawn = 0.5f;
    template_table[SpawnerNormal].rect.width = 32;
    template_table[SpawnerNormal].rect.height = 32;

    efs_EntitySetProperty(&template_table[SpawnerTwoPoints], efs_prop_Spawner);
    efs_EntitySetProperty(&template_table[SpawnerTwoPoints], efs_prop_MovesBetweenTwoPoints);
    template_table[SpawnerTwoPoints].texture = textures[TextureProjectileSpawner];
    template_table[SpawnerTwoPoints].spawnTime = 1.0f;
    template_table[SpawnerTwoPoints].timeSinceLastSpawn = 1.0f;
    template_table[SpawnerTwoPoints].rect.width = 32;
    template_table[SpawnerTwoPoints].rect.height = 32;
}

// must be initialised after the textures have been initialised
void EntityTemplatesInit(EntityTemplateTables* templates, const Texture2D textures[TextureTypeCount])
{
    ProjectileTemplatesInit(templates->projectile, textures);

    SpawnerTemplatesInit(templates->spawner, textures);

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