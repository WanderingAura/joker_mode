#include <stdarg.h>
#include <raylib.h>
#include <assert.h>

#include "based_basic.h"
#include "based_logging.h"

#define LOG_LINE_LEN_MAX 4096
static bsd_LogLevel currentLogLevel = bsd_LogLevel_Info;

void bsd_LoggingInit()
{
    char* dbgLevel = getenv("SOC_DBG_LEVEL");
    if (dbgLevel != NULL)
    {
        switch (dbgLevel[0])
        {
            case 'd':
            {
                currentLogLevel = bsd_LogLevel_Debug;
            }
            break;
            case 'i':
            {
                currentLogLevel = bsd_LogLevel_Info;
            }
            break;
            case 'w':
            {
                currentLogLevel = bsd_LogLevel_Warning;
            }
            break;
            case 'e':
            {
                currentLogLevel = bsd_LogLevel_Error;
            }
            break;
            case 'c':
            {
                currentLogLevel = bsd_LogLevel_Critical;

            }
            break;

            default:
            {
                fprintf(stderr, "unrecognised debug level: %s\n", dbgLevel);
            }
            break;
        }
    }
}

void bsd_SetLogLevel(bsd_LogLevel level)
{
    currentLogLevel = level;
}

// NOTE: unused for now, this is for logging into files
void bsd_log(FILE* logFile, bsd_LogLevel level, int line, const char* restrict file, const char* restrict fmt, ...)
{
    if (level < currentLogLevel)
    {
        return;
    }

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

    if (n >= curLineMax)
    {
        BSD_WARN("the following log message has been truncated because it's above %d characters", LOG_LINE_LEN_MAX);
    }

    n = fprintf(logFile, "%s\n", logBuf);
#ifdef BSD_FLUSH_ON_LOG
    fflush(logFile);
#endif
    assert(n > 0);
}

void bsd_log_stdout(bsd_LogLevel level, int line, const char* restrict file, const char* restrict fmt, ...)
{
    if (level < currentLogLevel)
    {
        return;
    }

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
    assert(n < LOG_LINE_LEN_MAX);

    char* curPos = logBuf + n;
    int curLineMax = LOG_LINE_LEN_MAX - n - 1;

    va_list args;

    va_start(args, fmt);

    n = vsnprintf(curPos, curLineMax, fmt, args);
    assert(n < curLineMax);

    va_end(args);

    n = fprintf(stdout, "%s\n", logBuf);
#ifdef BSD_FLUSH_ON_LOG
    fflush(logFile);
#endif
    assert(n > 0);
}