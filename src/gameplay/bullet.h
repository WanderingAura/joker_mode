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

// Shared setup for a freshly-allocated (efs_PoolAlloc'd) entity: fills in the
// fields a spawner would otherwise need to set itself, and marks it for
// per-frame parametric updates.
void bullet_init_common(efs_Entity* bullet, Vector2 pos, f32 radius, Color color);

// Example parametric spawn patterns - both built from only Pattern_Linear and
// Pattern_Sinusoidal.
void bullet_spawn_linear_spew(efs_EntityPool* pool, Vector2 origin, u32 count, f32 speed, f32 radius, Color color);
void bullet_spawn_inward_spiral(efs_EntityPool* pool, Vector2 target, u32 count, f32 orbit_radius, f32 orbit_freq, f32 collapse_duration, f32 radius, Color color);

void bullet_update(efs_Entity* bullet);
void bullet_draw(efs_Entity* bullet);
