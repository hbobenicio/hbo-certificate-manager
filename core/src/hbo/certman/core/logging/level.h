#pragma once

/**
 * @brief Enumeration of all possible logging levels
 */
enum log_level {
    LOG_LEVEL_TRACE,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
};

// Macros with string representations of each log level

#define LOG_LEVEL_TRACE_STR "trace"
#define LOG_LEVEL_DEBUG_STR "debug"
#define LOG_LEVEL_INFO_STR  "info"
#define LOG_LEVEL_WARN_STR  "warn"
#define LOG_LEVEL_ERROR_STR "error"
#define LOG_LEVEL_FATAL_STR "fatal"

/**
 * @brief Parses a string into a Log Level. Panics if it fails.
 * 
 * @param str String to be parsed
 * @return enum log_level The resulting Log Level
 */
enum log_level log_level_parse(const char* str);

/**
 * @brief Convert a Log Level into a string representation
 * 
 * @param level Log Level to be converted
 * @return const char* String representation of the given Log Level
 */
const char* log_level_to_string(enum log_level level);

/**
 * @brief Gets the corresponding ansi color sequence from the Log Level
 * 
 * @param level
 * @return const char* The ansi color escape sequence relative to the given Log Level
 */
const char* log_level_to_ansi_color(enum log_level level);
