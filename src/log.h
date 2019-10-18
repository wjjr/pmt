/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _PMT_LOG_H
#define _PMT_LOG_H

#undef EXIT_FAILURE
#define EXIT_MATCHES 0
#define EXIT_NOMATCH 1
#define EXIT_MISTAKE 2
#define EXIT_FAILURE 3
#define NOERR 0

#ifndef __MINGW32__
#ifdef WIN32
#define __attribute__(_)
#endif
#define _PRINTF_FORMAT(f, f_params) __attribute__((format(printf, f, f_params)))
#else
#define _PRINTF_FORMAT(f, f_params) __attribute__((format(gnu_printf, f, f_params)))
#endif

enum log_level {
    FATAL,
    SILENT,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    NOISY
};

void log_increase_level(void);

void log_silence(void);

void _PRINTF_FORMAT(2, 3) log_print(enum log_level, const char *message_format, ...);

void _PRINTF_FORMAT(3, 4) __attribute__((noreturn)) die(unsigned char status, int err_num, const char *message_format, ...);

#ifndef __PMT_DEBUG
static __inline void _PRINTF_FORMAT(2, 3) log_debug(enum log_level log_level __attribute__((unused)), const char *message_format __attribute__((unused)), ...) {}
#else
void _PRINTF_FORMAT(2, 3) log_debug(enum log_level, const char *message_format, ...);
#endif

#endif /*_PMT_LOG_H*/
