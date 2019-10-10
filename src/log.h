#ifndef _PMT_LOG_H
#define _PMT_LOG_H

#define EXIT_MATCHES 0
#define EXIT_NOMATCH 1
#define EXIT_MISTAKE 2

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

enum log_level log_get_level(void);

void log_print(enum log_level, const char *message_format, ...);

void log_debug(const char *message_format, ...);

void die(unsigned char status, int err_num, const char *message_format, ...);

#endif /*_PMT_LOG_H*/
