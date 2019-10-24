/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "shift_or.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "utils/common.h"
#include "../log.h"

void build_pattern_mask(usize *const masks, const struct pattern *const pattern) {
    usize i, all_set_mask = SHIFT_OR_BIT_MASK(pattern->length);

    for (i = 0; i < BYTE_MAX; ++i)
        masks[i] = all_set_mask; /* Set all bits of the char mask */

    for (i = 0; i < pattern->length; ++i) /* For each char of the pattern */
        masks[pattern->string[i]] &= ~(USIZE_C(1) << i); /* Unset the `i` bit of the char mask */

#ifdef __PMT_DEBUG
    {
        usize j;
        bool *c_printed = calloc(BYTE_MAX, 1);
        char *out_mask = malloc(6 + SHIFT_OR_MASK_SIZE);

        log_debug(DEBUG, "Using a %" PRIuSIZ "-bits mask for a pattern of %" PRIuSIZ " bytes", SHIFT_OR_MASK_SIZE, pattern->length);

        for (i = 0; i < pattern->length; ++i) {
            if (!c_printed[pattern->string[i]]) {
                c_printed[pattern->string[i]] = true;
                sprintf(out_mask, "%c: 0b", pattern->string[i]);

                for (j = USIZE_C(1) << (pattern->length - 1); j > 0; j >>= 1u)
                    sprintf(out_mask + strlen(out_mask), "%c", (masks[pattern->string[i]] & j) ? '1' : '0');

                log_debug(NOISY, "%s", out_mask);
            }
        }

        free(c_printed);
        free(out_mask);
    }
#endif
}

static usize run_shift_or(const usize *const masks, const struct file *const file, const struct pattern *const pattern, const struct search_context *const ctx) {
    usize i, buffer_read_size, total_read = 0, total_matches = 0, line_byte_offset = 0, *last_line_byte_offset = NULL;
    byte *const buffer = malloc(BUFFER_SIZE);
    usize s_mask = SHIFT_OR_BIT_MASK(pattern->length);
    const usize shift = pattern->length - 1u;

    for (; (buffer_read_size = fread(buffer, 1, BUFFER_SIZE, file->fp)) > 0; total_read += buffer_read_size)
        for (i = 0; i < buffer_read_size; ++i) {
            s_mask = (s_mask << 1u) | masks[buffer[i]];

            if (!((s_mask >> shift) & 1u)) {
                ++total_matches;

                if (!ctx->only_count) {
                    if (ctx->only_matching) {
                        if (ctx->print_byte_offset)
                            printf("%" PRIuSIZ ":", total_read + i + 1 - pattern->length);
                        printf("%.*s\n", (int) pattern->length, pattern->string);
                    } else
                        print_file_line(file, total_read + i, line_byte_offset, &last_line_byte_offset, ctx->print_byte_offset);
                }
            }
        }

    log_debug(DEBUG, "%" PRIuSIZ " bytes read from '%s'", total_read, file->name);
    log_debug(DEBUG, "found %" PRIuSIZ " matches in '%s'", total_matches, file->name);

    if (ferror(file->fp))
        die(EXIT_FAILURE, EIO, "%s", file->name);

    return total_matches;
}

uint_8 shift_or_search(const struct search_context *ctx) {
    usize i, j, total_matches;
    usize *masks = malloc(sizeof(usize) * BYTE_MAX);

    for (total_matches = 0, i = 0; i < ctx->num_patterns; ++i) {
        if (ctx->patterns[i].length <= SHIFT_OR_MASK_SIZE) {
            build_pattern_mask(masks, &ctx->patterns[i]);

            for (j = 0; j < ctx->num_files; ++j)
                total_matches += run_shift_or(masks, &ctx->files[j], &ctx->patterns[i], ctx);
        } else {
            log_print(WARN, "so: ignoring pattern %" PRIuSIZ ": pattern length is greater than %" PRIuSIZ " bytes", i + 1, SHIFT_OR_MASK_SIZE);
        }
    }

    if (ctx->only_count)
        printf("%" PRIuSIZ "\n", total_matches);

    return (total_matches > 0) ? EXIT_MATCHES : EXIT_NOMATCH;
}
