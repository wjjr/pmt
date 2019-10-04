#ifndef _PMT_ERROR_H
#define _PMT_ERROR_H

#define EXIT_MATCHES 0
#define EXIT_NOMATCH 1
#define EXIT_MISTAKE 2

enum loglevel {
    SILENT,
    ERROR,
    WARN,
    INFO,
    DEBUG
};

void log_set_progname(const char *progname);

const char *log_get_progname(void);

void log_increase_level();

void log_silence();

enum loglevel log_get_loglevel();

void log_info(const char *message_format, ...);

void log_warn(const char *message_format, ...);

void log_error(const char *message_format, ...);

void log_debug(const char *message_format, ...);

void die(unsigned char status, int errnum, const char *message_format, ...);

#endif /* _PMT_ERROR_H */
