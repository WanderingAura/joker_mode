#pragma once
#include <raylib.h>
#include <math.h>
#include <raymath.h>

#include "based_basic.h"
#include "based_core.h"
#include "efs_entity.h"

// Pattern/MovementInfo/MovementType/TargetType and the pooled efs_Entity
// itself live in efs_entity.h - Bullet was merged into efs_Entity, gated
// behind efs_prop_ParametricMovement (see handle_parametricMovement in
// behaviours/).

MovementInfo movement_linear(f32 speed);
// decay_duration <= 0 keeps amp constant; otherwise amp ramps linearly to 0 by t = decay_duration.
MovementInfo movement_sinusoidal(f32 amp, f32 freq, f32 phase, f32 decay_duration);
MovementInfo movement_constant(f32 value);

Vector2 calc_pos_from_patterns(MovementInfo* infos_x, MovementInfo* infos_y, Vector2 init_pos, f32 t, f32 rotation);
Vector2 vel_from_target(TargetType type, Vector2 target, Vector2 pos, f32 speed, Vector2 cur_vel);

// Angle (radians) from `from` to `to`, for aiming a bullet's `rotation` at a live point.
// Returns 0 for a zero-length delta rather than atan2f's undefined-ish 0/0 case.
f32 bullet_rotation_to(Vector2 from, Vector2 to);
// Angle (radians) of a direction vector itself, e.g. for a spawner's fixed fire direction.
f32 bullet_rotation_of_dir(Vector2 dir);

// Anchors + aims an entity for parametric movement using whatever move_info_x/y pattern
// it already carries (e.g. baked into a projectile template by core_entity_template.c) -
// only `pos` (the pattern's local origin) and `rotation` (aim direction) vary per spawn.
// Does not touch appearance (texture/rect/radius/color) or gameplay stats (damage,
// lifetime, ...) - callers fill those in from a template before/after calling this.
void bullet_orient(efs_Entity* bullet, Vector2 pos, f32 rotation);

// Shared setup for a freshly-allocated (efs_PoolAlloc'd) entity: fills in the
// fields a spawner would otherwise need to set itself, and marks it for
// per-frame parametric updates.
void bullet_init_common(efs_Entity* bullet, Vector2 pos, f32 radius, Color color);

// Example parametric spawn patterns - both built from only Pattern_Linear and
// Pattern_Sinusoidal.
void bullet_spawn_linear_spew(efs_EntityPool* pool, Vector2 origin, u32 count, f32 speed, f32 radius, Color color);
void bullet_spawn_inward_spiral(efs_EntityPool* pool, Vector2 target, u32 count, f32 orbit_radius, f32 orbit_freq, f32 collapse_duration, f32 radius, Color color);
// A single bullet fired from `origin` straight at `target_pos` (aimed once at spawn time,
// not homing).
efs_Entity* bullet_spawn_aimed(efs_EntityPool* pool, Vector2 origin, Vector2 target_pos, f32 speed, f32 radius, Color color);

// speed_multiplier scales this frame's effective speed (e.g. difficulty scaling) - pass 1.0f for none.
void bullet_update(efs_Entity* bullet, f32 speed_multiplier);
void bullet_draw(efs_Entity* bullet);
