#include "bullet.h"
#include "based_logging.h"

inline f32 calc_sin_pattern(f32 t, f32 amp, f32 freq, f32 phase)
{
    f32 result = amp * sinf(freq * t - phase);
    return result;
}

inline f32 calc_linear_pattern(f32 t, f32 speed)
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

inline f32 val_from_pattern(const MovementInfo* info, f32 t)
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
            result += calc_sin_pattern(t, info->amp, info->freq, info->phase);
        } break;
        default:
        {
            BSD_ERR("Unhandled pattern %d", info->pattern);
        }
    }
    return result;
}

Vector2 calc_pos_from_patterns(MovementInfo* infos_x, MovementInfo* infos_y, Vector2 init_pos, f32 t)
{
    Vector2 current_pos = init_pos;

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

    return current_pos;
}

Vector2 entity_pos(Entity* entity)
{
    switch (entity->type)
    {
        case EntityType_Bullet:
        {
            return entity->bullet.pos;
        } break;
        default:
        {
            BSD_CRIT("Unknown entity");
            break;
        }
    }
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

void bullet_update(Bullet* bullet)
{
    f32 t = GetFrameTime();
    bullet->time_since_spawn += t * bullet->parametric_speed;
    if (bullet->move_type == MovementType_Parametric)
    {
        bullet->parametric_speed = val_from_pattern(&bullet->speed_info, bullet->time_since_spawn);

        bullet->pos = calc_pos_from_patterns(bullet->move_info_x, bullet->move_info_y, bullet->init_pos, bullet->time_since_spawn);
    }
    else if (bullet->move_type == MovementType_Velocity)
    {
        Vector2 target_pos = entity_pos(bullet->target);
        bullet->vel = vel_from_target(bullet->targeting_type, target_pos, bullet->pos, bullet->parametric_speed, bullet->vel);
        bullet->pos = Vector2Add(bullet->pos, Vector2Scale(bullet->vel, t));
    }
    else
    {
        BSD_CRIT("Unknown bullet movement type");
    }
}

void bullet_draw(Bullet* bullet)
{
    DrawCircle(bullet->x, bullet->y, bullet->radius, bullet->color);
}