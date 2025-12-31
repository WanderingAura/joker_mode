#pragma once
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    bsd_LogLevel_Debug,
    bsd_LogLevel_Info,
    bsd_LogLevel_Warning,
    bsd_LogLevel_Error,
    bsd_LogLevel_Critical
} bsd_LogLevel;


static inline const char* filename(const char* path)
{
    const char* file = path;
    for (const char* p = path; *p; ++p) {
        if (*p == '/' || *p == '\\')
            file = p + 1;
    }
    return file;
}

#define _BSD_CUR_FILE_NAME filename(__FILE__)

#ifdef BSD_LOG_TO_FILE
  #error Not implemented yet
  #define BSD_LOG(...)
#else
  #define BSD_LOG(level, fmt, ...) bsd_log_stdout(level, __LINE__, _BSD_CUR_FILE_NAME, fmt __VA_OPT__(,) __VA_ARGS__)
#endif

#define BSD_DBG(fmt, ...) BSD_LOG(bsd_LogLevel_Debug, fmt __VA_OPT__(,) __VA_ARGS__)
#define BSD_INF(fmt, ...) BSD_LOG(bsd_LogLevel_Info, fmt __VA_OPT__(,) __VA_ARGS__)
#define BSD_WARN(fmt, ...) BSD_LOG(bsd_LogLevel_Warning, fmt __VA_OPT__(,) __VA_ARGS__)
#define BSD_ERR(fmt, ...) BSD_LOG(bsd_LogLevel_Error, fmt __VA_OPT__(,) __VA_ARGS__)

// only use when error warrants a crash
// NOTE: according to cppreference page on exit(), "All C streams are flushed and closed", which allows us to use buffered IO right before calling it.
#define BSD_CRIT(fmt, ...) \
    do { \
        BSD_LOG(bsd_LogLevel_Critical, fmt __VA_OPT__(,) __VA_ARGS__); \
        exit(1); \
    } while (0)

void bsd_log(FILE* logFile, bsd_LogLevel level, int line, const char* restrict file, const char* restrict fmt, ...);
void bsd_log_stdout(bsd_LogLevel level, int line, const char* restrict file, const char* restrict fmt, ...);
void bsd_SetLogLevel(bsd_LogLevel level);
