#include "core_game_memory.h"
#include "efs_entity.h"
#include "raylib.h"
#include "core_projectile.h"
#include "based_logging.h"

#include <string.h>

#define PROJ_SIZE 16


// a table storing templates for all the different types of projectile entities
static efs_Entity projectile_template_table[ProjectileTypeCount];

// must be initialised after the textures have been initialised
void ProjectileSystemInit(soc_GameMemory* memory)
{
    memset(&projectile_template_table, 0, sizeof(projectile_template_table));

    efs_EntitySetProperty(&projectile_template_table[ProjectileNormal], efs_prop_CanMove);
    efs_EntitySetProperty(&projectile_template_table[ProjectileNormal], efs_prop_HasLifetime);
    efs_EntitySetProperty(&projectile_template_table[ProjectileNormal], efs_prop_Collidable);
    projectile_template_table[ProjectileNormal].lifetime = 10.0f;
    projectile_template_table[ProjectileNormal].moveSpeed = 150.0f;
    projectile_template_table[ProjectileNormal].texture = memory->textures[TextureProjectile];
    projectile_template_table[ProjectileNormal].rect.width = PROJ_SIZE;
    projectile_template_table[ProjectileNormal].rect.height = PROJ_SIZE;


    efs_EntitySetProperty(&projectile_template_table[ProjectileCircle], efs_prop_CanMove);
    efs_EntitySetProperty(&projectile_template_table[ProjectileCircle], efs_prop_HasLifetime);
    efs_EntitySetProperty(&projectile_template_table[ProjectileCircle], efs_prop_Collidable);
    efs_EntitySetProperty(&projectile_template_table[ProjectileCircle], efs_prop_HasRotation);
    projectile_template_table[ProjectileCircle].lifetime = 20.0f;
    projectile_template_table[ProjectileCircle].moveSpeed = 150.0f;
    projectile_template_table[ProjectileCircle].rotationSpeed = 10.0f;
    projectile_template_table[ProjectileCircle].texture = memory->textures[TextureProjectile];
    projectile_template_table[ProjectileCircle].rect.width = PROJ_SIZE;
    projectile_template_table[ProjectileCircle].rect.height = PROJ_SIZE;

    BSD_INF("projectile system initialised");
}

efs_Entity ProjectileEntityCreate(ProjectileType type, Vector2 pos, Vector2 vel)
{
    efs_Entity proj = projectile_template_table[type];
    proj.vel = vel;
    proj.pos = pos;
    return proj;
}
