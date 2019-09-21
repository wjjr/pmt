
#include "kmp.h"

#include "../error.h"

unsigned char kmp_search(const struct algorithm_context *algorithm_context) {
    die(EXIT_MISTAKE, 0, "kmp: not implemented", algorithm_context->files_count);
    return 255;
}
