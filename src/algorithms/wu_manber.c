/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "wu_manber.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "shift_or.h"
#include "utils/common.h"
#include "../log.h"

static usize run_wu_manber(const usize *const masks, const struct file *const file, const struct pattern *const pattern, const struct search_context *const ctx) {
    usize i, j, buffer_read_size, total_read = 0, total_matches = 0, line_byte_offset = 0, *last_line_byte_offset = NULL;
    byte *const buffer = malloc(BUFFER_SIZE);
    usize *s_mask = malloc((ctx->edit_distance + 1u) * sizeof(usize)), *s_mask_prev = malloc((ctx->edit_distance + 1u) * sizeof(usize)), *s_mask_swp;
    const usize shift = pattern->length - 1u, all_set_mask = SHIFT_OR_BIT_MASK(pattern->length);


    for (s_mask_prev[0] = all_set_mask, i = 1; i < ctx->edit_distance; ++i)
        s_mask_prev[i] = s_mask_prev[0] << i;

    for (; (buffer_read_size = fread(buffer, 1, BUFFER_SIZE, file->fp)) > 0; total_read += buffer_read_size)
        for (i = 0; i < buffer_read_size; ++i) {
            s_mask[0] = (s_mask_prev[0] << 1u & all_set_mask) | masks[buffer[i]];

            for (j = 1; j <= ctx->edit_distance; ++j)
                s_mask[j] = ((s_mask_prev[j] << 1u & all_set_mask) | masks[buffer[i]]) & (s_mask_prev[j - 1] << 1u & all_set_mask) & (s_mask[j - 1] << 1u & all_set_mask) & s_mask_prev[j - 1];

            if (!((s_mask[ctx->edit_distance] >> shift) & 1u)) {
                ++total_matches;

                if (!ctx->only_count)
                    print_file_line(file, total_read + i, line_byte_offset, &last_line_byte_offset, ctx->print_byte_offset);
            }

            s_mask_swp = s_mask_prev;
            s_mask_prev = s_mask;
            s_mask = s_mask_swp;
        }

    log_debug(DEBUG, "%" PRIuSIZ " bytes read from '%s'", total_read, file->name);
    log_debug(DEBUG, "found %" PRIuSIZ " matches in '%s'", total_matches, file->name);

    if (ferror(file->fp))
        die(EXIT_FAILURE, EIO, "%s", file->name);

    return total_matches;
}

uint_8 wu_manber_search(const struct search_context *ctx) {
    usize i, j, total_matches;
    usize *masks = malloc(sizeof(usize) * BYTE_MAX);

    if (ctx->only_matching)
        die(EXIT_MISTAKE, NOERR, "wm: the option --only-matching is not supported with this algorithm");

    for (total_matches = 0, i = 0; i < ctx->num_patterns; ++i) {
        if (ctx->patterns[i].length <= SHIFT_OR_MASK_SIZE && ctx->edit_distance < ctx->patterns[i].length) {
            build_pattern_mask(masks, &ctx->patterns[i]);

            for (j = 0; j < ctx->num_files; ++j)
                total_matches += run_wu_manber(masks, &ctx->files[j], &ctx->patterns[i], ctx);
        } else if (ctx->patterns[i].length > SHIFT_OR_MASK_SIZE) {
            log_print(WARN, "wm: ignoring pattern %" PRIuSIZ ": pattern length is greater than %" PRIuSIZ " bytes", i + 1, SHIFT_OR_MASK_SIZE);
        } else if (ctx->edit_distance >= ctx->patterns[i].length) {
            log_print(WARN, "wm: ignoring pattern %" PRIuSIZ ": edit distance is greater or equal to the pattern length", i + 1);
        }
    }

    if (ctx->only_count)
        printf("%" PRIuSIZ "\n", total_matches);

    return (total_matches > 0) ? EXIT_MATCHES : EXIT_NOMATCH;
}
