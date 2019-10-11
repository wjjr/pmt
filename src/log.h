#ifndef _PMT_LOG_H
#define _PMT_LOG_H

#undef EXIT_FAILURE
#define EXIT_MATCHES 0
#define EXIT_NOMATCH 1
#define EXIT_MISTAKE 2
#define EXIT_FAILURE 3

enum log_level {
    FATAL,
    SILENT,
    ERROR,
    WARN,
    INFO,
    DEBUG
};

void log_increase_level(void);

void log_silence(void);

void log_print(enum log_level, const char *message_format, ...);

void die(unsigned char status, int err_num, const char *message_format, ...);

#ifndef __PMT_DEBUG
static __inline__ void log_debug(const char *message_format __attribute__ ((__unused__)), ...) {}
#else
void log_debug(const char *message_format, ...);
#endif

#endif /*_PMT_LOG_H*/
