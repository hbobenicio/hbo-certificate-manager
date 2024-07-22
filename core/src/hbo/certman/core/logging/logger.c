#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>

#include "level.h"
#include <emulator/config.h>
#include <emulator/ansi.h>

#ifndef LOG_RECORD_MSG_MAX_SIZE
#define LOG_RECORD_MSG_MAX_SIZE 1024
#endif

/**
 * @brief The main logging function
 * 
 * @param record 
 * @param ... 
 */
void log_log(struct log_record record, ...)
{
    if (record.level < config()->log_level) {
        return;
    }

    int rc, offset = 0;
    char buf[LOG_RECORD_MSG_MAX_SIZE] = {0};

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    // TIMESTAMP
    // TODO improve timestamping by printing milisseconds probably with clock_gettime()
    assert(offset < LOG_RECORD_MSG_MAX_SIZE);
    rc = snprintf(
        buf + offset,
        LOG_RECORD_MSG_MAX_SIZE - offset,
        "[%04d-%02d-%02d %02d:%02d:%02d] ",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec
    );
    assert(rc >= 0);
    assert(rc < LOG_RECORD_MSG_MAX_SIZE - offset);
    offset += rc;

    // Log Level and Log Tag
    assert(offset < LOG_RECORD_MSG_MAX_SIZE);
    rc = snprintf(
        buf + offset,
        LOG_RECORD_MSG_MAX_SIZE - offset,
        "%s%s" ANSI_HWHT ": " ANSI_HORA "%s" ANSI_HWHT ": ",
        log_level_to_ansi_color(record.level),
        log_level_to_string(record.level),
        record.tag
    );
    assert(rc >= 0);
    assert(rc < LOG_RECORD_MSG_MAX_SIZE - offset);
    offset += rc;

    // The Log Message itself
    assert(offset < LOG_RECORD_MSG_MAX_SIZE);
    va_list args;
    va_start(args, record);
    rc = vsnprintf(buf + offset, LOG_RECORD_MSG_MAX_SIZE - offset, record.msg, args);
    va_end(args);
    assert(rc >= 0);
    assert(rc < LOG_RECORD_MSG_MAX_SIZE - offset);
    offset += rc;

    // caused by suffix message, if any
    if (record.caused_by) {
        assert(offset < LOG_RECORD_MSG_MAX_SIZE);
        rc = snprintf(buf + offset, LOG_RECORD_MSG_MAX_SIZE - offset, ". caused by: %s\n", record.caused_by);
        assert(rc >= 0);
        assert(rc < LOG_RECORD_MSG_MAX_SIZE - offset);
        offset += rc;
    } else {
        assert(offset < LOG_RECORD_MSG_MAX_SIZE);
        rc = snprintf(buf + offset, LOG_RECORD_MSG_MAX_SIZE - offset, "\n");
        assert(rc >= 0);
        assert(rc < LOG_RECORD_MSG_MAX_SIZE - offset);
        offset += rc;
    }

    // Trailing ansi reset graphics escape sequence
    assert(offset < LOG_RECORD_MSG_MAX_SIZE);
    rc = snprintf(
        buf + offset,
        LOG_RECORD_MSG_MAX_SIZE - offset,
        ANSI_RESET
    );
    assert(rc >= 0);
    assert(rc < LOG_RECORD_MSG_MAX_SIZE - offset);
    offset += rc;

    // Finally dispatch the write call with the formatted message
    fputs(buf, stderr);

    if (record.level == LOG_LEVEL_FATAL) {
        exit(1);
    }
}
