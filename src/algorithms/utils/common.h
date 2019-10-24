/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _PMT_COMMON_H
#define _PMT_COMMON_H

#include "../../types.h"

#define BUFFER_SIZE 262144 /* 256KiB */
#define ALPH_SIZE (BYTE_MAX + 1)
#define LF '\n'

#undef MAX
#define MAX(x, y) (((x)>(y))?(x):(y))
#undef MIN
#define MIN(x, y) (((x)<(y))?(x):(y))

struct line {
    ssize beg;
    ssize end;
};

void print_file_line(const struct file *, usize file_offset, const byte *buffer, usize buffer_capacity, usize buffer_size, usize buffer_index, bool print_byte_offset, struct line *last_line);

#endif /*_PMT_COMMON_H*/
