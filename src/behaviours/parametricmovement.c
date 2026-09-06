#include <raylib.h>

#include "bullet.h"
#include "efs_entity.h"
#include "efs_entity_props.h"

#include "prop_behaviours.h"

int handle_parametricMovement(efs_Entity* entity) {
    if (efs_EntityHasProperty(entity, efs_prop_ParametricMovement))
    {
        bullet_update(entity);
    }
    return 0;
}
