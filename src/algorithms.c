
#include "algorithms.h"

#include <string.h>
#include "algorithms/aho_corasick.h"
#include "algorithms/kmp.h"
#include "algorithms/boyer_moore.h"
#include "algorithms/sellers.h"
#include "algorithms/ukkonen.h"
#include "algorithms/shiftor.h"
#include "algorithms/wu_manber.h"

static const struct algorithm algorithms[] = {
        {"ac",  "Aho-Corasick (1975)",                 0, 1, &aho_corasick_search},
        {"kmp", "Knuth-Morris-Pratt (1977)",           0, 0, &kmp_search},
        {"bm",  "Boyer-Moore (1977)",                  0, 0, &boyer_moore_search},
        {"se",  "Sellers (1980)",                      1, 0, &sellers_search},
        {"uk",  "Ukkonen (1985)",                      1, 0, &ukkonen_search},
        {"so",  "Shift-Or (Baeza-Yates–Gonnet, 1992)", 0, 0, &shiftor_search},
        {"wm",  "Wu-Manber (1992)",                    1, 0, &wu_manber_search},
        {NULL,  0,                                     0, 0, NULL}
};

const struct algorithm *get_algorithms(void) {
    return algorithms;
}

const struct algorithm *get_algorithm(const char *algorithm_id) {
    int i = 0;
    const struct algorithm *algorithm;

    for (i = 0; (algorithm = &algorithms[i])->id != NULL; ++i)
        if (strcmp(algorithm_id, algorithm->id) == 0)
            return algorithm;

    return NULL;
}

const struct algorithm *choose_algorithm(struct algorithm_context *algorithm_context) {
    return get_algorithm(algorithm_context->max_edit > 0 ? "wm" : (algorithm_context->patterns_counts > 1 ? "ac" : "bm"));
}
