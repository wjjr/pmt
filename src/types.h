#ifndef _PMT_TYPES_H
#define _PMT_TYPES_H

#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>
#include <limits.h>

#define PRIdSIZ PRIdPTR
#define PRIuSIZ PRIuPTR
#define USIZE_MAX SIZE_MAX
#define BYTE_MAX UCHAR_MAX

#if USIZE_MAX > UINT32_MAX
#define USIZE_C(c) UINT64_C(c)
#else
#define USIZE_C(c) UINT32_C(c)
#endif

#define false 0
#define true 1

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
typedef uint_8 byte;
typedef uint_8 bool;

struct file {
    FILE *fp;
    const char *name;
    usize size;
};

struct pattern {
    const byte *string;
    usize length;
};

struct search_context {
    const struct file *files;
    const struct pattern *patterns;
    usize num_files;
    usize num_patterns;
    uint_8 edit_distance;
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
