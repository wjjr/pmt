/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#ifndef _PMT_C90_TYPES_H
#define _PMT_C90_TYPES_H

#define int_least8_t signed char
#define int_least16_t signed short
#define int_least32_t signed int
#define int_least64_t signed long
#define uint_least8_t unsigned char
#define uint_least16_t unsigned short
#define uint_least32_t unsigned int
#define uint_least64_t unsigned long

#define PRIu8 "u"
#define PRIdPTR "ld"
#define PRIuPTR "lu"

#define __attribute__(_)

struct stat
{
    unsigned long st_size;
};

static __inline int stat (const char *__restrict __file, struct stat *__restrict __buf) {
    __buf->st_size = 0xFFFFu;
    return 0;
}

#endif /*_PMT_C90_TYPES_H */
