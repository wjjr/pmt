#ifndef _PMT_STRUCTS_H
#define _PMT_STRUCTS_H

#include <stdio.h>

struct file {
    FILE *fp;
    const char *name;
};

struct algorithm_context {
    struct file **files;
    const char **patterns;
    unsigned int patterns_counts;
    unsigned int files_count;
    unsigned char max_edit;
    unsigned char only_count;
};

struct algorithm {
    const char *id;
    const char *name;
    unsigned char approximate;
    unsigned char parallel;

    unsigned char (*search)(const struct algorithm_context *);
};

#endif /* _PMT_STRUCTS_H */
