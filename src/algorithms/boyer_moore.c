/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "boyer_moore.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "utils/common.h"
#include "../log.h"

#define MIN_PATTERN_LENGTH 2u

static usize delta_0[ALPH_SIZE];
static usize delta_1[ALPH_SIZE];
static usize *delta_2;

static void build_delta(const struct pattern *pattern) {
    usize i;

    for (i = 0; i < ALPH_SIZE; ++i)
        delta_0[i] = delta_1[i] = pattern->length;

    for (i = 0; i < pattern->length; ++i)
        delta_0[pattern->string[i]] = delta_1[pattern->string[i]] = pattern->length - i - 1;

    delta_0[pattern->string[pattern->length - 1]] = MAX(BUFFER_SIZE, pattern->length) << 1u;

    if (delta_2)
        free(delta_2);

    if ((delta_2 = calloc(pattern->length, sizeof(usize)))) {
        /* FIXME: Build delta_2 table */
    } else {
        log_print(WARN, "bm: %s: continuing without delta_2 table", strerror(ENOMEM));
    }
}

static usize run_boyer_moore(const struct file *const file, const struct pattern *const pattern, const struct search_context *const ctx) {
    usize i, j, k, buffer_read_size, total_read = 0, total_matches = 0, buffer_size = (MAX(BUFFER_SIZE, pattern->length) + 4095) & -USIZE_C(4096);
    byte *buffer_p = malloc(buffer_size * 2), *buffer, *back_buffer, *swp_buffer;
    struct line last_line = {-1, -1};
    const usize large = MAX(BUFFER_SIZE, pattern->length) << 1u;
    bool found;

    if (!buffer_p) {
        if (delta_2) {
            log_print(WARN, "bm: %s: freeing delta_2 table and trying again", strerror(ENOMEM));
            free(delta_2);
            buffer_p = malloc(buffer_size * 2);
        }

        if (!buffer_p)
            die(EXIT_FAILURE, ENOMEM, "bm");
    }

    for (buffer = buffer_p, back_buffer = &buffer_p[buffer_size], i = 0; (buffer_read_size = fread(buffer, 1, buffer_size, file->fp)) > 0;) {
        while (i < buffer_read_size) {
            for (j = pattern->length - 1, found = false; i < buffer_read_size; i += delta_0[buffer[i]]);

            if (i >= large) {
                for (--j, k = (i -= large - 1) - 2; buffer[k] == pattern->string[j] && k > 0 && j > 0; --k, --j);

                if (k == 0 && j != 0 && total_read != 0) {
                    for (--j, k = 1; back_buffer[BUFFER_SIZE - k] == pattern->string[j] && j > 0; --j, ++k);

                    if (j == 0 && back_buffer[BUFFER_SIZE - k] == pattern->string[j])
                        found = true;
                } else if (j == 0 && buffer[k] == pattern->string[j])
                    found = true;

                if (found) {
                    ++total_matches;

                    if (!ctx->only_count) {
                        if (ctx->only_matching) {
                            if (ctx->print_byte_offset)
                                printf("%" PRIuSIZ ":", total_read + i - pattern->length);
                            printf("%.*s\n", (int) pattern->length, pattern->string);
                        } else {
                            print_file_line(file, total_read + i, buffer, BUFFER_SIZE, buffer_read_size, i, ctx->print_byte_offset, &last_line);
                        }
                    }
                }
            }

            if (i < buffer_read_size)
                i += MAX(delta_1[buffer[i]], delta_2 ? delta_2[j] : 0);
        }

        total_read += buffer_read_size;
        i -= buffer_read_size;

        swp_buffer = back_buffer;
        back_buffer = buffer;
        buffer = swp_buffer;
    }

    log_debug(DEBUG, "%" PRIuSIZ " bytes read from '%s'", total_read, file->name);
    log_debug(DEBUG, "found %" PRIuSIZ " matches in '%s'", total_matches, file->name);

    if (ferror(file->fp))
        die(EXIT_FAILURE, EIO, "%s", file->name);
    else if (fseek(file->fp, 0L, SEEK_SET))
        die(EXIT_FAILURE, errno, "%s", file->name);

    free(buffer_p);

    return total_matches;
}

uint_8 boyer_moore_search(const struct search_context *const ctx) {
    usize i, j, total_matches;

    for (total_matches = 0, i = 0; i < ctx->num_patterns; ++i) {
        if (ctx->patterns[i].length < MIN_PATTERN_LENGTH) {
            log_print(WARN, "bm: ignoring pattern %" PRIuSIZ ": pattern length is less than %" PRIu32 " bytes", i + 1, MIN_PATTERN_LENGTH);
            continue;
        }

        build_delta(&ctx->patterns[i]);

        for (j = 0; j < ctx->num_files; ++j)
            total_matches += run_boyer_moore(&ctx->files[j], &ctx->patterns[i], ctx);
    }

    if (ctx->only_count)
        printf("%" PRIuSIZ "\n", total_matches);

    return (total_matches > 0) ? EXIT_MATCHES : EXIT_NOMATCH;
}
