#include <raylib.h>

#include "bullet.h"
#include "efs_entity.h"
#include "efs_entity_props.h"

#include "prop_behaviours.h"

int handle_parametricMovement(efs_Entity* entity, soc_GameMemory* memory) {
    if (efs_EntityHasProperty(entity, efs_prop_ParametricMovement))
    {
        f32 speed_multiplier = 1.0f;
        if (efs_EntityHasProperty(entity, efs_prop_ScalesWithDifficulty))
        {
            speed_multiplier = 1.0f + memory->levelTimer / 20.0f;
        }
        bullet_update(entity, speed_multiplier);
    }
    return 0;
}
