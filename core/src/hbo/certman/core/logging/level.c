#include "level.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include <hbo/certman/core/ansi.h>

enum log_level log_level_parse(const char* str)
{
    if (str == NULL) {
        return LOG_LEVEL_INFO;
    }
    
    if (strncmp(LOG_LEVEL_TRACE_STR, str, sizeof(LOG_LEVEL_TRACE_STR)) == 0) {
        return LOG_LEVEL_TRACE;
    }

    if (strncmp(LOG_LEVEL_DEBUG_STR, str, sizeof(LOG_LEVEL_DEBUG_STR)) == 0) {
        return LOG_LEVEL_DEBUG;
    }

    if (strncmp(LOG_LEVEL_INFO_STR, str, sizeof(LOG_LEVEL_INFO_STR)) == 0) {
        return LOG_LEVEL_INFO;
    }

    if (strncmp(LOG_LEVEL_WARN_STR, str, sizeof(LOG_LEVEL_WARN_STR)) == 0) {
        return LOG_LEVEL_WARN;
    }

    if (strncmp(LOG_LEVEL_ERROR_STR, str, sizeof(LOG_LEVEL_ERROR_STR)) == 0) {
        return LOG_LEVEL_ERROR;
    }

    if (strncmp(LOG_LEVEL_FATAL_STR, str, sizeof(LOG_LEVEL_FATAL_STR)) == 0) {
        return LOG_LEVEL_FATAL;
    }

    fprintf(stderr, "error: logging: parsing failed: \"%s\" is not a valid log level\n", str);
    exit(1);
}

const char* log_level_to_string(enum log_level level)
{
    switch (level) {
        case LOG_LEVEL_TRACE: return LOG_LEVEL_TRACE_STR;
        case LOG_LEVEL_DEBUG: return LOG_LEVEL_DEBUG_STR;
        case LOG_LEVEL_INFO:  return LOG_LEVEL_INFO_STR;
        case LOG_LEVEL_WARN:  return LOG_LEVEL_WARN_STR;
        case LOG_LEVEL_ERROR: return LOG_LEVEL_ERROR_STR;
        case LOG_LEVEL_FATAL: return LOG_LEVEL_FATAL_STR;
        default: break;
    }
    assert(false && "unreachable: unknown log level");
}

const char* log_level_to_ansi_color(enum log_level level)
{
    switch (level) {
        case LOG_LEVEL_TRACE: return ANSI_HBLU;
        case LOG_LEVEL_DEBUG: return ANSI_HCYN;
        case LOG_LEVEL_INFO:  return ANSI_HGRN;
        case LOG_LEVEL_WARN:  return ANSI_HYEL;
        case LOG_LEVEL_ERROR: return ANSI_HRED;
        case LOG_LEVEL_FATAL: return ANSI_BHRED;
        default: break;
    }
    assert(false && "unreachable: unknown log level");
}
