#include "bullet.h"
#include "based_logging.h"
#include "raymath.h"

static inline f32 calc_linear_pattern(f32 t, f32 speed)
{
    f32 result = t * speed;
    return result;
}

inline f32 cubed(f32 x)
{
    return x * x * x;
}

inline f32 calc_cubic_easein_pattern(f32 t, f32 duration)
{
    f32 result = cubed(t / duration);
    return result;
}

// Linear ramp from 1 down to 0 over [0, decay_duration]; decay_duration <= 0 disables decay.
static inline f32 sinusoid_envelope(f32 t, f32 decay_duration)
{
    if (decay_duration <= 0.0f)
    {
        return 1.0f;
    }
    return ClampBot(1.0f - t / decay_duration, 0.0f);
}
static inline f32 calc_sin_pattern(f32 t, f32 amp, f32 freq, f32 phase)
{
    f32 result = amp * sinf(freq * t - phase);
    return result;
}


static inline f32 val_from_pattern(const MovementInfo* info, f32 t)
{
    f32 result = 0;
    switch (info->pattern)
    {
        case Pattern_Linear:
        {
            result += calc_linear_pattern(t, info->speed);
        } break;
        case Pattern_Sinusoidal:
        {
            f32 envelope = sinusoid_envelope(t, info->decay_duration);
            result += envelope * calc_sin_pattern(t, info->amp, info->freq, info->phase);
        } break;
        default:
        {
            BSD_ERR("Unhandled pattern %d", info->pattern);
        }
    }
    return result;
}

Vector2 calc_pos_from_patterns(MovementInfo* infos_x, MovementInfo* infos_y, Vector2 init_pos, f32 t, f32 rotation)
{
    Vector2 current_pos = {};

    for (u32 i = 0; i < MAX_SIMUL_PATTERNS; i++)
    {
        MovementInfo info_x = infos_x[i];
        MovementInfo info_y = infos_y[i];
        Pattern pat_x = info_x.pattern;
        Pattern pat_y = info_y.pattern;

        if (pat_x == Pattern_None || pat_y == Pattern_None)
        {
            Assert(pat_x == Pattern_None && pat_y == Pattern_None);
            break;
        }

        current_pos.x += val_from_pattern(&info_x, t);
        current_pos.y += val_from_pattern(&info_y, t);
    }

    // rotate and offset the final pattern
    current_pos = Vector2Rotate(current_pos, rotation);
    current_pos = Vector2Add(current_pos, init_pos);

    return current_pos;
}

Vector2 vel_from_target(TargetType type, Vector2 target, Vector2 pos, f32 speed, Vector2 cur_vel)
{
    Vector2 result = {};
    Vector2 target_dir = Vector2Normalize(Vector2Subtract(target, pos));
    switch (type)
    {
        case TargetType_Direct:
        {
            result = Vector2Scale(target_dir, speed);
        } break;
        case TargetType_Accel:
        {
            result = Vector2Add(Vector2Scale(target_dir, speed), cur_vel);
        } break;
        default:
        {
            BSD_CRIT("Unknown target type");
        } break;
    }
    return result;
}

void bullet_update(efs_Entity* bullet)
{
    f32 t = GetFrameTime();
    bullet->time_since_spawn += t * bullet->parametric_speed;
    if (bullet->move_type == MovementType_Parametric)
    {
        bullet->parametric_speed = val_from_pattern(&bullet->speed_info, bullet->time_since_spawn);

        bullet->pos = calc_pos_from_patterns(bullet->move_info_x, bullet->move_info_y, bullet->init_pos, bullet->time_since_spawn, bullet->rotation);
    }
    else if (bullet->move_type == MovementType_Velocity)
    {
        Vector2 target_pos = bullet->target->pos;
        bullet->vel = vel_from_target(bullet->targeting_type, target_pos, bullet->pos, bullet->parametric_speed, bullet->vel);
        bullet->pos = Vector2Add(bullet->pos, Vector2Scale(bullet->vel, t));
    }
    else
    {
        BSD_CRIT("Unknown bullet movement type");
    }
}

void bullet_draw(efs_Entity* bullet)
{
    DrawCircle(bullet->pos.x, bullet->pos.y, bullet->radius, bullet->color);
}

MovementInfo movement_linear(f32 speed)
{
    MovementInfo info = {0};
    info.pattern = Pattern_Linear;
    info.speed = speed;
    return info;
}

// decay_duration <= 0 keeps amp constant; otherwise amp ramps linearly to 0 by t = decay_duration.
MovementInfo movement_sinusoidal(f32 amp, f32 freq, f32 phase, f32 decay_duration)
{
    MovementInfo info = {0};
    info.pattern = Pattern_Sinusoidal;
    info.amp = amp;
    info.freq = freq;
    info.phase = phase;
    info.decay_duration = decay_duration;
    return info;
}

// Pattern_Sinusoidal with freq = 0 collapses to a t-invariant value: sinf(0*t - phase) ==
// sinf(-phase). With phase = -PI/2 that's sinf(PI/2) == 1, so val_from_pattern returns
// exactly `value` for every t. This is how a constant is expressed using only the two
// implemented patterns (Pattern_Constant is declared but unimplemented in val_from_pattern).
MovementInfo movement_constant(f32 value)
{
    MovementInfo info = {0};
    info.pattern = Pattern_Sinusoidal;
    info.amp = value;
    info.freq = 0.0f;
    info.phase = -PI / 2.0f;
    return info;
}

// Shared setup for a freshly-allocated (efs_PoolAlloc'd, already zeroed) entity: sets the
// fields a spawner would otherwise need to fill in itself, and marks the entity for
// per-frame parametric updates via handle_parametricMovement.
void bullet_init_common(efs_Entity* bullet, Vector2 pos, f32 radius, Color color)
{
    bullet->pos = pos;
    bullet->init_pos = pos;
    bullet->radius = radius;
    bullet->color = color;
    bullet->move_type = MovementType_Parametric;

    bullet->parametric_speed = 1.0f;
    bullet->speed_info = movement_constant(1.0f);

    bullet->targeting_type = TargetType_Direct;

    efs_EntitySetProperty(bullet, efs_prop_ParametricMovement);
}

// A set of `count` bullets that all share one local trajectory - a straight line along
// local +x at `speed` - and are fanned out into evenly spaced directions purely via
// `rotation`. Classic N-way radial burst from a single point.
void bullet_spawn_linear_spew(efs_EntityPool* pool, Vector2 origin, u32 count, f32 speed, f32 radius, Color color)
{
    for (u32 i = 0; i < count; i++)
    {
        efs_Entity* bullet = efs_PoolAlloc(pool);
        if (bullet == NULL)
        {
            break;
        }

        bullet_init_common(bullet, origin, radius, color);

        bullet->rotation = (f32)i * (2.0f * PI / (f32)count);
        bullet->move_info_x[0] = movement_linear(speed);
        bullet->move_info_y[0] = movement_linear(0.0f);
    }
}

// A set of `count` bullets spawned on a ring of `orbit_radius` around `target`. Each bullet
// traces a genuine spiral: sinusoids on both local axes, 90 degrees out of phase (amp*cos,
// amp*sin), produce circular motion around `target`, and the shared decay envelope shrinks
// that circle's radius linearly to 0 by `collapse_duration`, so every bullet spirals inward
// and lands exactly on `target`.
void bullet_spawn_inward_spiral(efs_EntityPool* pool, Vector2 target, u32 count, f32 orbit_radius, f32 orbit_freq, f32 collapse_duration, f32 radius, Color color)
{
    for (u32 i = 0; i < count; i++)
    {
        efs_Entity* bullet = efs_PoolAlloc(pool);
        if (bullet == NULL)
        {
            break;
        }

        f32 spawn_angle = (f32)i * (2.0f * PI / (f32)count);

        bullet_init_common(bullet, (Vector2){0,0}, radius, color);

        // The orbit circles around `target`, not `spawn_pos` - override the rotation
        // anchor bullet_init_common set, without touching next/prev/pos.
        bullet->init_pos = target;

        // `rotation` places each bullet's local +x (where its orbit starts, at t=0) at
        // its spawn angle around the ring.
        bullet->rotation = spawn_angle;
        bullet->move_info_x[0] = movement_sinusoidal(orbit_radius, orbit_freq, -PI / 2.0f, collapse_duration);
        bullet->move_info_y[0] = movement_sinusoidal(orbit_radius, orbit_freq, 0.0f, collapse_duration);
    }
}
