
#include "log.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

static enum log_level log_level = WARN;

static __inline__ char error_prefix(enum log_level log_lvl) {
    switch (log_lvl) {
        case FATAL:
            return 'F';
        case SILENT:
            return 'S';
        case ERROR:
            return 'E';
        case WARN:
            return 'W';
        case INFO:
            return 'I';
        case DEBUG:
            return 'D';
        default:
            return '*';
    }
}

static void error_log(enum log_level log_lvl, unsigned char status, int errnum, const char *message_format, va_list args) {
    fprintf(stderr, "%c: ", error_prefix(log_lvl));

    vfprintf(stderr, message_format, args);

    if (errnum) {
        const char *error_string = strerror(errnum);
        fprintf(stderr, ": %s", error_string ? error_string : "Unknown system error");
    }

    putc('\n', stderr);

    if (status)
        exit(status);
}

void log_increase_level(void) {
    if (log_level != SILENT)
        ++log_level;
}

void log_silence(void) {
    log_level = SILENT;
}

enum log_level log_get_level(void) {
    return log_level;
}

void log_print(enum log_level log_lvl, const char *message_format, ...) {
    if (log_lvl <= log_level) {
        va_list args;

        va_start(args, message_format);
        error_log(log_lvl, 0, 0, message_format, args);
        va_end(args);
    }
}

void die(unsigned char status, int errnum, const char *message_format, ...) {
    va_list args;

    va_start(args, message_format);
    error_log(FATAL, status, errnum, message_format, args);
    va_end(args);
}
