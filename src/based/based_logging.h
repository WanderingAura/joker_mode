#include <stdio.h>

typedef enum {
    bsd_LogLevel_Debug,
    bsd_LogLevel_Info,
    bsd_LogLevel_Warning,
    bsd_LogLevel_Error,
    bsd_LogLevel_Critical
} bsd_LogLevel;

#ifdef BSD_LOG_TO_FILE
  #error Not implemented yet
  #define BSD_LOG(...)
#else
  #define BSD_LOG(level, fmt, ...) bsd_log(stdout, level, __LINE__, __FILE__, fmt __VA_OPT__(,) __VA_ARGS__)
#endif

#define BSD_DBG(fmt, ...) BSD_LOG(bsd_LogLevel_Debug, fmt __VA_OPT__(,) __VA_ARGS__)
#define BSD_INF(fmt, ...) BSD_LOG(bsd_LogLevel_Info, fmt __VA_OPT__(,) __VA_ARGS__)
#define BSD_WARN(fmt, ...) BSD_LOG(bsd_LogLevel_Warning, fmt __VA_OPT__(,) __VA_ARGS__)
#define BSD_ERR(fmt, ...) BSD_LOG(bsd_LogLevel_Error, fmt __VA_OPT__(,) __VA_ARGS__)
#define BSD_CRIT(fmt, ...) BSD_LOG(bsd_LogLevel_Critical, fmt __VA_OPT__(,) __VA_ARGS__)

void bsd_log(FILE* logFile, bsd_LogLevel level, int line, const char* restrict file, const char* restrict fmt, ...);

