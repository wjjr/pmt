#ifndef _PMT_TYPES_H
#define _PMT_TYPES_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

typedef int_least8_t int_8;
typedef int_least16_t int_16;
typedef int_least32_t int_32;
typedef int_least64_t int_64;
typedef uint_least8_t uint_8;
typedef uint_least16_t uint_16;
typedef uint_least32_t uint_32;
typedef uint_least64_t uint_64;
typedef size_t usize;
typedef ptrdiff_t ssize;
typedef unsigned char byte;
typedef unsigned char bool;

struct file {
    FILE *fp;
    const char *name;
    uint_64 size;
};

struct pattern {
    const byte *string;
    uint_64 length;
};

struct search_context {
    const struct file *files;
    const struct pattern *patterns;
    uint_64 num_files;
    uint_64 num_patterns;
    uint_8 max_edit;
    bool only_count;
    bool only_matching;
    bool print_byte_offset;
};

struct algorithm {
    const char *const id;
    const char *const name;
    const bool approximate;
    const bool parallel;

    uint_8 (*const search)(const struct search_context *);
};

#endif /*_PMT_TYPES_H */
