#include "core_entity_types.h"
#include "core_game_memory.h"
#include "efs_entity.h"
#include "raylib.h"
#include "core_entity_template.h"
#include "based_logging.h"
#include "bullet.h"

#include <string.h>

#define PROJ_SIZE 16

static void ProjectileTemplatesInit(efs_Entity template_table[ProjectileTypeCount], const Texture2D textures[TextureTypeCount])
{
    memset(template_table, 0, sizeof(efs_Entity)*ProjectileTypeCount);

    efs_EntitySetProperty(&template_table[ProjectileNormal], efs_prop_HasLifetime);
    efs_EntitySetProperty(&template_table[ProjectileNormal], efs_prop_Collidable);
    efs_EntitySetProperty(&template_table[ProjectileNormal], efs_prop_CanDamage);
    efs_EntitySetProperty(&template_table[ProjectileNormal], efs_prop_DespawnWhenFarFromPlayer);
    efs_EntitySetProperty(&template_table[ProjectileNormal], efs_prop_ScalesWithDifficulty);
    template_table[ProjectileNormal].lifetime = 5.0f;
    template_table[ProjectileNormal].damage = 3;
    template_table[ProjectileNormal].canDamage = PlayerGroup;
    template_table[ProjectileNormal].despawnDistance = 1000.0f;
    template_table[ProjectileNormal].texture = textures[TextureProjectile];
    template_table[ProjectileNormal].rect.width = PROJ_SIZE;
    template_table[ProjectileNormal].rect.height = PROJ_SIZE;
    // straight line along local +x; a spawn call only needs to set pos/rotation (see bullet_orient)
    template_table[ProjectileNormal].move_info_x[0] = movement_linear(350.0f);
    template_table[ProjectileNormal].move_info_y[0] = movement_linear(0.0f);

    memcpy(&template_table[ProjectileCircle], &template_table[ProjectileNormal], sizeof(efs_Entity));
    template_table[ProjectileCircle].lifetime = 5.0f;
    // same forward travel, plus a lateral wave - replaces the old spin-the-dir-vector spiral
    template_table[ProjectileCircle].move_info_x[0] = movement_linear(300.0f);
    template_table[ProjectileCircle].move_info_y[0] = movement_sinusoidal(40.0f, 4.0f, 0.0f, 0.0f);
}

static void SpawnerTemplatesInit(efs_Entity template_table[SpawnerTypeCount], const Texture2D textures[TextureTypeCount])
{
    memset(template_table, 0, sizeof(template_table[0])*SpawnerTypeCount);
    efs_EntitySetProperty(&template_table[SpawnerNormal], efs_prop_Spawner);
    efs_EntitySetProperty(&template_table[SpawnerNormal], efs_prop_HasHealth);
    template_table[SpawnerNormal].texture = textures[TextureProjectileSpawner];
    template_table[SpawnerNormal].health = 5;
    template_table[SpawnerNormal].damageGroup = EnemyGroup;
    template_table[SpawnerNormal].spawnTime = 0.5f;
    template_table[SpawnerNormal].timeSinceLastSpawn = 0.5f;
    template_table[SpawnerNormal].rect.width = 32;
    template_table[SpawnerNormal].rect.height = 32;

    // TODO: used to patrol between two points via the since-removed efs_prop_MovesBetweenTwoPoints,
    // which was declared but never implemented. For now this behaves like SpawnerNormal with a
    // slower fire rate; re-add patrol movement as a parametric pattern (e.g. a sinusoid between
    // the two points) rather than a bespoke movement mode if/when it's needed.
    efs_EntitySetProperty(&template_table[SpawnerTwoPoints], efs_prop_Spawner);
    efs_EntitySetProperty(&template_table[SpawnerTwoPoints], efs_prop_HasHealth);
    template_table[SpawnerTwoPoints].texture = textures[TextureProjectileSpawner];
    template_table[SpawnerTwoPoints].health = 5;
    template_table[SpawnerTwoPoints].damageGroup = EnemyGroup;
    template_table[SpawnerTwoPoints].spawnTime = 1.0f;
    template_table[SpawnerTwoPoints].timeSinceLastSpawn = 1.0f;
    template_table[SpawnerTwoPoints].rect.width = 32;
    template_table[SpawnerTwoPoints].rect.height = 32;
}

#define ENEMY_SIZE 32

// Must run after ProjectileTemplatesInit - EnemyChaser's shot references templates->projectile.
static void EnemyTemplatesInit(EntityTemplateTables* templates, const Texture2D textures[TextureTypeCount])
{
    efs_Entity template_table[EnemyTypeCount];
    memset(template_table, 0, sizeof(template_table));

    efs_Entity* chaser = &template_table[EnemyChaser];
    efs_EntitySetProperty(chaser, efs_prop_HasHealth);
    efs_EntitySetProperty(chaser, efs_prop_CanDamage);
    efs_EntitySetProperty(chaser, efs_prop_ParametricMovement);
    efs_EntitySetProperty(chaser, efs_prop_ShootsAtTarget);
    efs_EntitySetProperty(chaser, efs_prop_DespawnWhenFarFromPlayer);
    chaser->health = 10;
    chaser->damage = 1;
    chaser->damageGroup = EnemyGroup;
    chaser->canDamage = PlayerGroup; // contact damage against the player
    chaser->move_type = MovementType_Velocity;
    chaser->targeting_type = TargetType_Direct;
    chaser->parametric_speed = 120.0f; // chase speed; not overwritten each frame in Velocity mode
    chaser->attackSpeed = 1.5f;
    chaser->texture = textures[TextureProjectileSpawner]; // placeholder art - no dedicated enemy sprite yet
    chaser->rect.width = ENEMY_SIZE;
    chaser->rect.height = ENEMY_SIZE;
    chaser->despawnDistance = 1500.0f;
    chaser->childInfo.template = &templates->projectile[ProjectileNormal];

    memcpy(templates->enemy, template_table, sizeof(template_table));
}

// must be initialised after the textures have been initialised
void EntityTemplatesInit(EntityTemplateTables* templates, const Texture2D textures[TextureTypeCount])
{
    ProjectileTemplatesInit(templates->projectile, textures);

    SpawnerTemplatesInit(templates->spawner, textures);

    EnemyTemplatesInit(templates, textures);

    BSD_INF("projectile system initialised");
}

efs_Entity ProjectileEntityCreate(ProjectileType type, Vector2 pos, Vector2 dir)
{
    efs_Entity proj = core_GameMemoryGet()->entityTemplates.projectile[type];
    proj.dir = dir;
    proj.pos = pos;
    return proj;
}

efs_Entity ProjectileSpawnerCreate(SpawnerType type, Vector2 pos, Vector2 dir, SpawnedProjInfo spawnedInfo)
{
    EntityTemplateTables* templates = &core_GameMemoryGet()->entityTemplates;
    efs_Entity spawner = templates->spawner[type];
    spawner.dir = dir;
    spawner.pos = pos;
    spawner.childInfo.template = &templates->projectile[spawnedInfo.type];
    spawner.childInfo.initialDir = spawnedInfo.dir;
    spawner.childInfo.offset = spawnedInfo.offset;
    return spawner;
}

efs_Entity EnemyEntityCreate(EnemyType type, Vector2 pos)
{
    soc_GameMemory* memory = core_GameMemoryGet();
    efs_Entity enemy = memory->entityTemplates.enemy[type];
    // Not bullet_orient() - that forces MovementType_Parametric, which would stomp the
    // MovementType_Velocity chase baked into the enemy template.
    enemy.pos = pos;
    enemy.init_pos = pos;
    enemy.target = memory->player;
    return enemy;
}