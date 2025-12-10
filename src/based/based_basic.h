#pragma once
#include <stdint.h>

/* ==== BASIC TYPES ==== */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef s8 b8;
typedef s16 b16;
typedef s32 b32;
typedef s64 b64;

/* ==== BASIC MACROS ==== */
#define ArrayCount(arr) (sizeof(arr)/(sizeof(*(arr))))
#define STRINGIFY(s) #s

// returns the most significant byte of an integral value
#define GetMSB(x) ((x) >> (sizeof(x) * 8 - 8))

// the least significant 3 bytes of an integral value
#define GetLS3B(x) ((x) & 0xffffff)