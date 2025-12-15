#include <stdarg.h>
#include <raylib.h>
#include <assert.h>

#include "based_basic.h"
#include "based_logging.h"

#define LOG_LINE_LEN_MAX 4096

void bsd_log(FILE* logFile, bsd_LogLevel level, int line, const char* restrict file, const char* restrict fmt, ...)
{
    static char* dbgLevels[] =
    {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERR",
        "CRIT"
    };

    char logBuf[LOG_LINE_LEN_MAX];

    f64 timeSinceInit = GetTime();

    int n = snprintf(logBuf, LOG_LINE_LEN_MAX - 1, "[%12.6lf] [%s] [%s:%d] ",
            timeSinceInit, dbgLevels[level], file, line);
    assert(n > 25);

    char* curPos = logBuf + n;
    int curLineMax = LOG_LINE_LEN_MAX - n - 1;

    va_list args;

    va_start(args, fmt);

    n = vsnprintf(curPos, curLineMax, fmt, args);
    assert(n > 0);

    va_end(args);

    n = fprintf(logFile, "%s\n", logBuf);
    assert(n > 0);
}