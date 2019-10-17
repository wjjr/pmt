/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _PMT_COMMON_H
#define _PMT_COMMON_H

#include "../../log.h"

#define BUFFER_SIZE 262144 /* 256KiB */
#define ALPH_SIZE (BYTE_MAX + 1)
#define LF '\n'

#undef MAX
#define MAX(x, y) (((x)>(y))?(x):(y))
#undef MIN
#define MIN(x, y) (((x)<(y))?(x):(y))

static __inline __attribute__((unused)) void print_file_line(const struct file *const file, const usize offset, const usize line_byte_offset, usize **const last_line_byte_offset, const bool print_byte_offset) {
    if (*last_line_byte_offset == NULL) {
        *last_line_byte_offset = malloc(sizeof(usize));
        **last_line_byte_offset = 0;
    }

    /* FIXME: implement print line */
    log_print(WARN, "print_file_line('%s', %" PRIuSIZ ", %" PRIuSIZ ", %" PRIuSIZ ", %" PRIu8 "): not implemented", file->name, offset, line_byte_offset, **last_line_byte_offset, print_byte_offset);
}

#endif /*_PMT_COMMON_H*/
