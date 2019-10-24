/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _PMT_SHIFT_OR_H
#define _PMT_SHIFT_OR_H

#include "../types.h"

#define SHIFT_OR_MASK_SIZE (sizeof(usize) * CHAR_BIT)
#define SHIFT_OR_BIT_MASK(bits_to_set) ((bits_to_set) >= SHIFT_OR_MASK_SIZE ? USIZE_MAX : (USIZE_C(1) << (bits_to_set)) - 1)

void build_pattern_mask(usize *masks, const struct pattern *);

uint_8 shift_or_search(const struct search_context *);

#endif /* _PMT_SHIFT_OR_H */
