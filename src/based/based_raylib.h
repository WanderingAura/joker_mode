#pragma once
#include <raylib.h>
#include "based_basic.h"

#define bsd_BRIGHT_RED (Color){0xFF,0x7A, 0x61, 0xFF}

// default settings
#define PERIODIC_FADE_FREQ 0.8f
#define PERIODIC_FADE_ALPHA_LOW 0.2f
#define PERIODIC_FADE_ALPHA_HIGH 1.0f

typedef struct {
    f32 freq;
    f32 low;
    f32 high;
} PeriodicFadeParams;

#define PeriodicFade(c, ...) PeriodicFade_(c, (PeriodicFadeParams){.freq = PERIODIC_FADE_FREQ, .low = PERIODIC_FADE_ALPHA_LOW, .high = PERIODIC_FADE_ALPHA_HIGH, __VA_ARGS__})

Color PeriodicFade_(Color c, PeriodicFadeParams params);
void DrawHCentreText(const char* text, f32 y, f32 fontSize, f32 lowX, f32 highX, Color color);
void DrawHCentreScreenText(const char* text, f32 y, f32 fontSize, Color color);
f32 CalculateCentredPosition(f32 low, f32 high, f32 size);