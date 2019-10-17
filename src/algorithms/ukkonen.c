/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#include "ukkonen.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "utils/common.h"
#include "utils/queue.h"
#include "../log.h"

#define HASH_MASK 0x1FFFu
#define HASH_SIZE (HASH_MASK + 1)

/*// state { //*/
struct state {
    usize index;
    struct state **transitions;
    const uint_8 *column;
    bool is_final;
};

static struct state *state_new(const uint_8 *const column, const bool is_final) {
    static usize index = 0;
    struct state *s = malloc(sizeof(struct state));

    s->index = index++;
    s->transitions = calloc(ALPH_SIZE, sizeof(void *));
    s->column = column;
    s->is_final = is_final;

    return s;
}
/*// } state //*/

/*// hash { //*/
/* TODO: store states with a tree */
static __inline usize hash_column(const uint_8 *column, const usize column_size) {
    usize i, h;

    for (i = 0, h = 0; i < column_size; ++i)
        h = ((h << 2u) + h) + column[i];

    return h & HASH_MASK;
}

static __inline void *hash_new(void) { return calloc(HASH_SIZE, sizeof(void *)); }

static __inline struct state *hash_get(struct state ***states, const uint_8 *const column, const usize column_size) {
    usize i, h = hash_column(column, column_size);

    if (states[h])
        for (i = 0; states[h][i]; ++i)
            if (memcmp(states[h][i]->column, column, column_size) == 0)
                return states[h][i];

    return NULL;
}

static __inline void hash_add(struct state ***states, const uint_8 *const column, const usize column_size, struct state *const s) {
    usize i = 0, h = hash_column(column, column_size);

    if (states[h]) {
        for (i = 0; states[h][i]; ++i);
        states[h] = realloc(states[h], (i + 2) * sizeof(void *));
    } else
        states[h] = malloc(2 * sizeof(void *));

    states[h][i] = s;
    states[h][i + 1] = NULL;
}
/*// } hash //*/

static __inline uint_8 *next_column(const struct pattern *const pattern, const uint_8 *const prev_column, const uint_8 max_edit, const byte c) {
    usize i;
    uint_8 *column = malloc(pattern->length + 1), x, y, z;

    for (column[0] = 0, i = 1; i <= pattern->length; ++i) {
        x = (uint_8) (prev_column[i] + 1u), y = (uint_8) (column[i - 1] + 1u), z = (uint_8) (prev_column[i - 1] + (pattern->string[i - 1] == c ? 0u : 1u));
        column[i] = (uint_8) MIN(x, MIN(y, MIN(z, max_edit + 1)));
    }

    return column;
}

static const struct state *build_dfa(const struct pattern *const pattern, const uint_8 max_edit) {
    struct state *start, *s, *t;
    struct queue *q = NULL;
    void *h = hash_new();
    usize i, j = 0, k, column_size = pattern->length + 1;
    uint_8 *column;

    for (column = malloc(column_size), i = 0; i < column_size; column[i] = (uint_8) MIN(i, max_edit + 1u), ++i);
    start = state_new(column, pattern->length <= max_edit);
    hash_add(h, column, column_size, start);

    for (q = queue_push(q, start); (s = queue_pop(q)) != NULL;) {
        for (i = 0; i < ALPH_SIZE; ++i) {
            column = next_column(pattern, s->column, max_edit, (byte) i);

            if (!(t = hash_get(h, column, column_size))) {
                t = state_new(column, column[pattern->length] <= max_edit);
                hash_add(h, column, column_size, t);
                queue_push(q, t);
            }

            s->transitions[i] = t;

            log_debug(NOISY, "%c%.2" PRIuSIZ " -> 0x%.2x (%c) -> %c%.2" PRIuSIZ, s->is_final ? 'F' : 'S', s->index, (byte) i, (byte) ((i > 31 && i < 127) ? i : 0x1Au), t->is_final ? 'F' : 'S', t->index);
        }
    }

    for (i = 0, j = 0, k = state_new(NULL, false)->index; i < HASH_SIZE; ++i)
        if (((void **) h)[i]) ++j;
    log_debug(DEBUG, "%" PRIuSIZ " states stored in %" PRIuSIZ "/%u hash table entries (%.2f%% collision)", k, j, HASH_SIZE, (((double) (k - j) / (double) k)) * 100);

    return start;
}

static usize run_ukkonen(const struct state *const dfa, const struct file *const file, const struct search_context *const ctx) {
    usize i, buffer_read_size, total_read = 0, total_matches = 0, line_byte_offset = 0, *last_line_byte_offset = NULL;
    byte *buffer = malloc(BUFFER_SIZE);
    const struct state *s;

    for (s = dfa; (buffer_read_size = fread(buffer, 1, BUFFER_SIZE, file->fp)) > 0; total_read += buffer_read_size)
        for (i = 0; i < buffer_read_size; ++i) {
            if ((s = s->transitions[buffer[i]])->is_final) {
                ++total_matches;

                if (!ctx->only_count)
                    print_file_line(file, total_read + i, line_byte_offset, &last_line_byte_offset, ctx->print_byte_offset);
            }

            if (buffer[i] == LF)
                line_byte_offset = total_read + i;
        }

    log_debug(DEBUG, "%" PRIuSIZ " bytes read from '%s'", total_read, file->name);
    log_debug(DEBUG, "found %" PRIuSIZ " matches in '%s'", total_matches, file->name);

    if (ferror(file->fp))
        die(EXIT_FAILURE, EIO, "%s", file->name);

    return total_matches;
}

uint_8 ukkonen_search(const struct search_context *ctx) {
    usize i, j, total_matches;
    const struct state *dfa;

    if (ctx->only_matching)
        die(EXIT_MISTAKE, NOERR, "uk: the option --only-matching is not supported with this algorithm");

    for (total_matches = 0, i = 0; i < ctx->num_patterns; ++i) {
        if (ctx->edit_distance >= ctx->patterns[i].length) {
            log_print(WARN, "uk: ignoring pattern %" PRIuSIZ ": edit distance is greater or equal to the pattern length", i + 1);
            continue;
        }

        dfa = build_dfa(&ctx->patterns[i], ctx->edit_distance);

        for (j = 0; j < ctx->num_files; ++j)
            total_matches += run_ukkonen(dfa, &ctx->files[j], ctx);
    }

    if (ctx->only_count)
        printf("%" PRIuSIZ "\n", total_matches);

    return (total_matches > 0) ? EXIT_MATCHES : EXIT_NOMATCH;
}
