/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "brute_force.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "utils/common.h"
#include "../log.h"

#define MAX_PATTERN_LENGTH (BUFFER_SIZE / 2u - 1u)

usize run_brute_force(const struct file *file, const struct pattern *pattern, const struct search_context *ctx) {
    usize i, j, buffer_read_size, total_read = 0, total_matches = 0, line_byte_offset = 0, *last_line_byte_offset = NULL;
    byte *buffer = malloc(BUFFER_SIZE);

    for (j = 0; (buffer_read_size = fread(buffer + j, 1, BUFFER_SIZE - j, file->fp)) > 0; total_read += buffer_read_size)
        for (i = 0; i < buffer_read_size; ++i, j = 0) {
            for (; j < pattern->length && (i + j) < buffer_read_size && pattern->string[j] == buffer[i + j]; ++j);

            if (j == pattern->length) {
                ++total_matches;

                if (!ctx->only_count) {
                    if (ctx->only_matching) {
                        if (ctx->print_byte_offset)
                            printf("%" PRIuSIZ ":", total_read + i + j - pattern->length);
                        printf("%.*s\n", (int) pattern->length, pattern->string);
                    } else
                        print_file_line(file, total_read + i + j, line_byte_offset, &last_line_byte_offset, ctx->print_byte_offset);
                }
            } else if ((i + j) >= buffer_read_size) {
                memcpy(&buffer[0], &buffer[i], j);
                break;
            }
        }

    log_debug(DEBUG, "%" PRIuSIZ " bytes read from '%s'", total_read, file->name);
    log_debug(DEBUG, "found %" PRIuSIZ " matches in '%s'", total_matches, file->name);

    if (ferror(file->fp))
        die(EXIT_FAILURE, EIO, "%s", file->name);

    return total_matches;
}

uint_8 brute_force_search(const struct search_context *ctx) {
    usize i, j, total_matches;

    for (total_matches = 0, i = 0; i < ctx->num_patterns; ++i) {
        if (ctx->patterns[i].length <= MAX_PATTERN_LENGTH) {
            for (j = 0; j < ctx->num_files; ++j)
                total_matches += run_brute_force(&ctx->files[j], &ctx->patterns[i], ctx);
        } else {
            log_print(WARN, "bf: ignoring pattern %" PRIuSIZ ": pattern length is greater than %" PRIu32 " bytes", i + 1, MAX_PATTERN_LENGTH);
        }
    }

    if (ctx->only_count)
        printf("%" PRIuSIZ "\n", total_matches);

    return (total_matches > 0) ? EXIT_MATCHES : EXIT_NOMATCH;
}
