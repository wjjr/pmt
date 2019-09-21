#ifndef _PMT_ERROR_H
#define _PMT_ERROR_H

#define EXIT_MATCHES 0
#define EXIT_NOMATCH 1
#define EXIT_MISTAKE 2

enum loglevel {
    SILENT,
    INFO,
    WARN,
    ERROR,
    DEBUG
};

void log_set_progname(const char *progname);

const char *log_get_progname(void);

void log_info(unsigned char loglevel, const char *message_format, ...);

void log_warn(unsigned char loglevel, const char *message_format, ...);

void log_error(unsigned char loglevel, const char *message_format, ...);

void log_debug(unsigned char loglevel, const char *message_format, ...);

void die(unsigned char status, int errnum, const char *message_format, ...);

#endif /* _PMT_ERROR_H */
