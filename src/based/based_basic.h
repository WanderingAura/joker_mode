#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "based_logging.h"

#if defined(_MSC_VER)
    #define DISABLE_UNUSED_WARNING \
        __pragma(warning(push)) \
        __pragma(warning(disable: 4101))
        __pragma(warning(disable: 4505))
    #define ENABLE_UNUSED_WARNING \
        __pragma(warning(pop))
#elif defined(__GNUC__) || defined(__clang__)
    #define DISABLE_UNUSED_WARNING \
        _Pragma("GCC diagnostic push") \
        _Pragma("GCC diagnostic ignored \"-Wunused-variable\"")
        _Pragma("GCC diagnostic ignored \"-Wunused-function\"")
    #define ENABLE_UNUSED_WARNING \
        _Pragma("GCC diagnostic pop")
#else
    #define DISABLE_UNUSED_WARNING
    #define ENABLE_UNUSED_WARNING
#endif

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
#define STRINGIFY(s) #s

#if defined(__GNUC__)
# define MAYBE_UNUSED __attribute__((unused))
#else
# define MAYBE_UNUSED
#endif

#ifndef NDEBUG
#define DBG_ASSERT_MSG(cond, msg, ...) \
    do { \
        if (!(cond)) \
        { \
            BSD_CRIT("Assertion" STRINGIFY(cond) " failed" msg __VA_OPT__(,) __VA_ARGS__); \
        } \
    } while (0)
#else
#define DBG_ASSERT_MSG(cond, msg, ...)
#endif

bool bsd_IsDigit(char c);
