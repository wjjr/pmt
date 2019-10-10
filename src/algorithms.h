#ifndef _PMT_ALGORITHMS_H
#define _PMT_ALGORITHMS_H

#include "types.h"

const struct algorithm *get_algorithms(void);

const struct algorithm *get_algorithm(const char *algorithm_id);

const struct algorithm *choose_algorithm(const struct search_context *);

#endif /* PMT_ALGORITHMS_H */
