#pragma once

#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "level.h"

struct log_record {
    const char* tag;
    enum log_level level;
    const char* msg;
    const char* caused_by;
};

void log_log(struct log_record msg, ...);

#define log_trace(MESSAGE) log_tracef((MESSAGE), NULL)
#define log_tracef(FORMAT, ...) \
    log_log((struct log_record) { \
        .tag = LOG_TAG, \
        .level = LOG_LEVEL_TRACE, \
        .msg = FORMAT, \
        .caused_by = NULL, \
    }, __VA_ARGS__)

#define log_debug(MESSAGE) log_debugf((MESSAGE), NULL)
#define log_debugf(FORMAT, ...) \
    log_log((struct log_record) { \
        .tag = LOG_TAG, \
        .level = LOG_LEVEL_DEBUG, \
        .msg = FORMAT, \
        .caused_by = NULL, \
    }, __VA_ARGS__)

#define log_info(MESSAGE) log_infof((MESSAGE), NULL)
#define log_infof(FORMAT, ...) \
    log_log((struct log_record) { \
        .tag = LOG_TAG, \
        .level = LOG_LEVEL_INFO, \
        .msg = FORMAT, \
        .caused_by = NULL, \
    }, __VA_ARGS__)

#define log_warn(MESSAGE) log_warnf((MESSAGE), NULL)
#define log_warnf(FORMAT, ...) \
    log_log((struct log_record) { \
        .tag = LOG_TAG, \
        .level = LOG_LEVEL_WARN, \
        .msg = FORMAT, \
        .caused_by = NULL, \
    }, __VA_ARGS__)

#define log_error(MESSAGE) log_errorf((MESSAGE), NULL)
#define log_errorf(FORMAT, ...) \
    log_log((struct log_record) { \
        .tag = LOG_TAG, \
        .level = LOG_LEVEL_ERROR, \
        .msg = (FORMAT), \
        .caused_by = NULL, \
    }, __VA_ARGS__)

#define log_fatal(MESSAGE) log_fatalf((MESSAGE), NULL)
#define log_fatalf(FORMAT, ...) \
    log_log((struct log_record) { \
        .tag = LOG_TAG, \
        .level = LOG_LEVEL_FATAL, \
        .msg = (FORMAT), \
        .caused_by = NULL, \
    }, __VA_ARGS__)
