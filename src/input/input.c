#include "input.h"
#include "based_basic.h"
#include "based_core.h"
#include "raymath.h"
#include <raylib.h>

#define JOYSTICK_FUZZY_FACTOR 1.5f

typedef struct
{
    Vector2 center;
    float radius;
    s32 touch_id;
} VJoystick;

typedef struct
{
    Vector2 input_dir;
    u32 buttons_pressed;
    VJoystick joystick;
} InputState;

static Vector2 InputDirFromMockJoystick_(VJoystick* joystick);
static Vector2 InputDirFromJoystick_(VJoystick* joystick);

#if defined(TOUCH_INPUTS_ENABLED) || defined(TOUCH_INPUTS_MOCKED)
#define InputDirFromJoystick(joystick) InputDirFromJoystick_(joystick)
#else
#define InputDirFromJoystick(joystick) (input_state.input_dir)
#endif

typedef int InputMap[512];

static InputState input_state;
static InputMap input_map;

void InitialiseInputs()
{
    // TODO: the input key map should be empty on android, otherwise we'll get problems from the player
    // pressing the volume buttons for example.

    // NOTE: we skip the first index because that's the 'no key pressed' case
    for (u32 i = 1; i < ArrayCount(input_map); i++)
    {
        input_map[i] = OtherButton;
    }

    input_map[KEY_LEFT_SHIFT] = DodgeButton;
    input_map[KEY_RIGHT_SHIFT] = DodgeButton;
    input_map[KEY_SPACE] = NextButton;
    input_map[KEY_ENTER] = ConfirmButton;
    input_map[KEY_BACKSPACE] = RemoveCharButton;
    input_map[KEY_W] = UpButton;
    input_map[KEY_A] = LeftButton;
    input_map[KEY_S] = DownButton;
    input_map[KEY_D] = RightButton;
    input_map[KEY_LEFT] = LeftButton;
    input_map[KEY_RIGHT] = RightButton;
    input_map[KEY_UP] = UpButton;
    input_map[KEY_DOWN] = DownButton;
    input_map[KEY_R] = ResetButton;
    input_map[KEY_ESCAPE] = MenuButton;

    // initialise the joystick.

    VJoystick* joystick = &input_state.joystick;
    const f32 distance_away_from_edge = 125.0f;
    joystick->center = (Vector2){distance_away_from_edge, GetScreenHeight() - distance_away_from_edge};
    joystick->radius = 75.0f;
}

bool IsButtonsPressed(u32 buttons)
{
    return (input_state.buttons_pressed & buttons) != 0;
}

static inline bool IsButtonsPressedInternal(u32 buttons, u32 pressed_mask)
{
    return (pressed_mask & buttons) != 0;
}

void DrawJoystick_()
{
    VJoystick* joystick = &input_state.joystick;
    DrawCircleV(joystick->center, joystick->radius, BEIGE);
    if (input_state.input_dir.x != 0.0f || input_state.input_dir.y != 0.0f)
    {
        Vector2 i_unit_vector = {1.0f, 0.0f};
        f32 move_angle = Vector2Angle(i_unit_vector, input_state.input_dir) * RAD2DEG;
        const f32 sector_width_degrees = 25.0f;
        const int segment_number = 10;
        DrawCircleSector(joystick->center, joystick->radius,
            move_angle - sector_width_degrees / 2.0f, move_angle + sector_width_degrees / 2.0f,
            segment_number, BLUE);
    }
}

void UpdateInputs()
{
    // the reason for this variable is for if we want to check the previous state
    // for more complex input handling
    u32 new_buttons_pressed = 0;
    for (u32 key = 32; key < 512; key++)
    {
        if (IsKeyDown(key))
        {
            new_buttons_pressed |= input_map[key];
        }
    }

    if (GetTouchPointCount() > 0)
    {
        new_buttons_pressed |= ScreenTouched;
        new_buttons_pressed |= NextButton;
    }

    // calculate the direction from key input
    input_state.input_dir.y = (float)(IsButtonsPressedInternal(DownButton, new_buttons_pressed)
        - IsButtonsPressedInternal(UpButton, new_buttons_pressed));
    
    input_state.input_dir.x = (float)(IsButtonsPressedInternal(RightButton, new_buttons_pressed)
        - IsButtonsPressedInternal(LeftButton, new_buttons_pressed));
    
    input_state.buttons_pressed = new_buttons_pressed;
    Vector2 joystick_dir = InputDirFromJoystick(&input_state.joystick);
    if (Vector2LengthSqr(joystick_dir) > 0.0f)
    {
        input_state.input_dir = joystick_dir;
    }
}

int GetTouchPointCountMocked()
{
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        return 1;
    }
    return 0;
}

int GetTouchPointIdMocked(int i)
{
    return 0;
}

Vector2 GetTouchPointPositionMocked(int i)
{
    Assert(IsMouseButtonDown(MOUSE_BUTTON_LEFT));

    return GetMousePosition();
}

#ifdef TOUCH_INPUTS_MOCKED
#define GetVTouchPointId(i) GetTouchPointIdMocked(i)
#define GetVTouchPointCount() GetTouchPointCountMocked()
#define GetVTouchPosition(i) GetTouchPointPositionMocked(i)
#else
#define GetVTouchPointId(i) GetTouchPointId(i)
#define GetVTouchPointCount() GetTouchPointCount()
#define GetVTouchPosition(i) GetTouchPosition(i)
#endif

static Vector2 InputDirFromJoystick_(VJoystick* joystick)
{
    // if the joystick is currently touched, we need to check if that id
    // still exists, if not, then reset it
    bool joystick_released = true;
    for (u32 i = 0; i < GetVTouchPointCount(); i++)
    {
        if (joystick->touch_id >= 0)
        {
            // if we're pressing the joystick check if it's been released
            int touchpoint_id = GetVTouchPointId(i);
            if (touchpoint_id == joystick->touch_id)
            {
                joystick_released = false;
            }
        }
        else
        {
            // check whether touch is inside joystick
            Vector2 touchpoint_pos = GetVTouchPosition(i);
            if (CheckCollisionPointCircle(touchpoint_pos, joystick->center, joystick->radius*JOYSTICK_FUZZY_FACTOR))
            {
                joystick_released = false;
                joystick->touch_id = GetVTouchPointId(i);
            }
        }

    }

    Vector2 result = {0};
    if (joystick_released)
    {
        joystick->touch_id = -1;
    }
    else
    {
        // joystick is being held, calculate input dir
        Vector2 joystick_touch_pos = GetVTouchPosition(joystick->touch_id);
        Vector2 touch_dir = Vector2Subtract(joystick_touch_pos, joystick->center);

        const f32 deadzone_radius = 20.0f;
        f32 touch_magnitude = Vector2Length(touch_dir);
        if (touch_magnitude >= deadzone_radius)
        {
            // normalise
            result = Vector2Scale(touch_dir, 1.0f / touch_magnitude);
        }

    }
    return result;
}

Vector2 GetInputDir()
{
    return input_state.input_dir;
}