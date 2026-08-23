#pragma once

#include <raylib.h>
#include "based_basic.h"

// these buttons are intentions rather than physical keys.
// physical key presses are translated to these by the input system.
typedef enum : u32
{
    OtherButton = (1 << 0),
    DodgeButton = (1 << 1),
    ScreenTouched = (1 << 2),
    ConfirmButton = (1 << 3),
    UpButton = (1 << 4),
    DownButton = (1 << 5),
    RightButton = (1 << 6),
    LeftButton = (1 << 7),
    NextButton = (1 << 8),
    RemoveCharButton = (1 << 9),
    MenuButton = (1 << 10),
    ResetButton = (1 << 11),
} InputButtons;

#define AnyButton (UINT32_MAX)

void InitialiseInputs();
bool IsButtonsPressed(u32 buttons);

void DrawJoystick_();
#if defined(TOUCH_INPUTS_ENABLED) || defined(TOUCH_INPUTS_MOCKED)
#define DrawJoystick() DrawJoystick_()
#else
#define DrawJoystick()
#endif
void UpdateInputs();
Vector2 GetInputDir();