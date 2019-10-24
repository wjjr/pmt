/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "aho_corasick.h"

#include <stdlib.h>
#include <errno.h>
#include "utils/common.h"
#include "utils/queue.h"
#include "../log.h"

struct state {
    struct state **transitions;
    struct state *failure;
    const struct pattern **patterns;
    usize num_patterns;
};

static __inline struct state *state_new(void) {
    struct state *s = malloc(sizeof(struct state));

    s->transitions = calloc(ALPH_SIZE, sizeof(void *));
    s->failure = NULL;
    s->patterns = NULL;
    s->num_patterns = 0;

    return s;
}

static const struct state *build_sm(const struct pattern *const patterns, const usize num_patterns) {
    struct state *start, *s, *t, *u;
    struct queue *q = NULL;
    usize i, j;
    byte c;

    /* Goto { */
    start = s = state_new();

    for (i = 0; i < num_patterns; ++i, s = start) {
        for (j = 0; j < patterns[i].length; ++j) {
            c = patterns[i].string[j];
            s = (s->transitions[c] == NULL) ? s->transitions[c] = state_new() : s->transitions[c];
        }

        s->patterns = realloc(s->patterns, ++s->num_patterns * sizeof(void *));
        s->patterns[s->num_patterns - 1] = &patterns[i];
    }

    for (i = 0; i < ALPH_SIZE; ++i)
        if (start->transitions[i] == NULL)
            start->transitions[i] = start;
    /* } */

    /* Failure { */
    for (q = queue_push(q, start); (s = queue_pop(q)) != NULL;)
        for (i = 0; i < ALPH_SIZE; ++i)
            if ((t = s->transitions[i]) != NULL && t != start) {
                if (s == start) {
                    t->failure = start;
                } else {
                    for (u = s->failure; u->transitions[i] == NULL; u = u->failure);
                    t->failure = u->transitions[i];

                    for (j = 0; j < t->failure->num_patterns; ++j) {
                        t->patterns = realloc(t->patterns, ++t->num_patterns * sizeof(void *));
                        t->patterns[t->num_patterns - 1] = t->failure->patterns[j];
                    }
                }

                q = queue_push(q, t);
            }

    free(q);
    /* } */

    /* TODO: construct deterministic finite automaton */

    return start;
}

static usize run_aho_corasick(const struct state *const sm, const struct file *const file, const struct search_context *const ctx) {
    usize i, j, buffer_read_size, total_read = 0, total_matches = 0, max_pattern_length;
    byte *buffer = malloc(BUFFER_SIZE);
    struct line last_line = {-1, -1};
    const struct state *s, *t, *u = NULL;
    const struct pattern *pattern = NULL;
    usize pattern_offset = 0;
    bool has_failed;

    for (s = sm; (buffer_read_size = fread(buffer, 1, BUFFER_SIZE, file->fp)) > 0; total_read += buffer_read_size)
        for (i = 0; i < buffer_read_size; ++i) {
            for (t = s, has_failed = 0; (s = t->transitions[buffer[i]]) == NULL; t = t->failure, has_failed = 1);

            if (u && has_failed) {
                total_matches += u->num_patterns;

                if (!ctx->only_count && ctx->only_matching) {
                    max_pattern_length = 0, pattern = NULL;

                    for (j = 0; j < u->num_patterns; ++j)
                        if (u->patterns[j]->length >= max_pattern_length)
                            pattern = u->patterns[j];

                    if (pattern) {
                        if (ctx->print_byte_offset)
                            printf("%" PRIuSIZ ":", pattern_offset - pattern->length);
                        printf("%.*s\n", (int) pattern->length, pattern->string);
                    }
                }

                u = NULL;
            }

            if (s->num_patterns > 0) {
                u = s;
                pattern_offset = total_read + i + 1;

                if (!ctx->only_count && !ctx->only_matching)
                    print_file_line(file, total_read + i, buffer, BUFFER_SIZE, buffer_read_size, i, ctx->print_byte_offset, &last_line);
            }
        }

    log_debug(DEBUG, "%" PRIuSIZ " bytes read from '%s'", total_read, file->name);
    log_debug(DEBUG, "found %" PRIuSIZ " matches in '%s'", total_matches, file->name);

    if (ferror(file->fp))
        die(EXIT_FAILURE, EIO, "%s", file->name);

    return total_matches;
}

uint_8 aho_corasick_search(const struct search_context *const ctx) {
    usize i, total_matches;
    const struct state *sm;

    sm = build_sm(ctx->patterns, ctx->num_patterns);

    for (total_matches = 0, i = 0; i < ctx->num_files; ++i)
        total_matches += run_aho_corasick(sm, &ctx->files[i], ctx);

    if (ctx->only_count)
        printf("%" PRIuSIZ "\n", total_matches);

    return (total_matches > 0) ? EXIT_MATCHES : EXIT_NOMATCH;
}
