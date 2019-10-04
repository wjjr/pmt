
#include "error.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

static const char *error_log_progname;
static enum loglevel loglevel = WARN;

static __inline__ char error_prefix(enum loglevel loglvl) {
    switch (loglvl) {
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

static void error_log(enum loglevel loglvl, unsigned char status, int errnum, const char *message_format, va_list args) {
    fprintf(stderr, "%c: %s: ", error_prefix(loglvl), error_log_progname);

    vfprintf(stderr, message_format, args);

    if (errnum) {
        const char *error_string = strerror(errnum);
        fprintf(stderr, ": %s", error_string ? error_string : "Unknown system error");
    }

    putc('\n', stderr);

    if (status)
        exit(status);
}

void log_set_progname(const char *progname) {
    error_log_progname = progname;
}

const char *log_get_progname(void) {
    return error_log_progname;
}

void log_increase_level() {
    if (loglevel != SILENT)
        ++loglevel;
}

void log_silence() {
    loglevel = SILENT;
}

enum loglevel log_get_loglevel() {
    return loglevel;
}

void log_print(enum loglevel loglvl, const char *message_format, ...) {
    if (loglvl <= loglevel) {
        va_list args;

        va_start(args, message_format);
        error_log(loglvl, 0, 0, message_format, args);
        va_end(args);
    }
}

void die(unsigned char status, int errnum, const char *message_format, ...) {
    va_list args;

    va_start(args, message_format);
    error_log(FATAL, status, errnum, message_format, args);
    va_end(args);
}
