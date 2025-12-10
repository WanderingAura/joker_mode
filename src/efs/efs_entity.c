#include "efs_entity.h"

bool efs_EntityHasProperty(efs_Entity const* entity, efs_PropertyType prop)
{
    u64 propLoc = entity->props[prop/NUM_PROPS_IN_U64];

    return propLoc & (1 << (prop % NUM_PROPS_IN_U64));
}
