#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include "algorithms.h"
#include "error.h"

static const char optstring[] = "a:ce:hp:v";
static const struct option longopts[] = {
        {"algorithm", required_argument, NULL, 'a'},
        {"count",     no_argument,       NULL, 'c'},
        {"edit",      required_argument, NULL, 'e'},
        {"help",      no_argument,       NULL, 'h'},
        {"pattern",   required_argument, NULL, 'p'},
        {NULL,        no_argument,       NULL, '\0'}
};

static void usage(int status) {
    const char *progname = log_get_progname();
    const struct algorithm *algorithms, *algorithm;
    int i;

    fprintf((status != 0) ? stderr : stdout, "Usage: %s [-a ALGO] [-c] [-e DIST] [-h] (PATTERN | -p PATTERN_FILE) FILE [FILE...]\n", progname);

    if (status != 0) {
        fprintf(stderr, "Try '%s --help' for more information.\n", progname);
        exit(status);
    }

    printf("Search for PATTERN in each FILE.\n\n");
    printf("-a, --algorithm=NAME  Selects the algorithm used in the search\n");
    printf("-c, --count           Print only a total count of matches\n");
    printf("-e, --edit=NUM        Find patterns with a maximum edit distance of NUM operations\n");
    printf("-h, --help            Display this help text and exit\n");
    printf("-p, --pattern=FILE    Obtain patterns from FILE, one per line\n");

    printf("\nSupported algorithms:\n");
    for (i = 0, algorithms = get_algorithms(); (algorithm = &algorithms[i])->id != NULL; ++i)
        printf("%-3s : %s%s\n", algorithm->id, algorithm->name, algorithm->approximate ? " [approximate]" : "");

    exit(status);
}

int main(int argc, char *argv[]) {
    int opt;
    const struct algorithm *algorithm = NULL;
    unsigned char only_show_count = 0, max_edit = 0, loglevel = 0;
    const char **patterns = NULL;
    struct file **files = NULL;
    unsigned int patterns_count = 0, files_count = 0;
    struct algorithm_context *context;

    log_set_progname("pmt");

    while ((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
        switch (opt) {
            case 'a':
                if ((algorithm = get_algorithm(optarg)) == NULL)
                    die(EXIT_MISTAKE, 0, "%s: unknown algorithm", optarg);
                break;
            case 'c':
                only_show_count = 1;
                break;
            case 'e': {
                char *endptr;
                long optval = strtol(optarg, &endptr, 10);
                max_edit = optval;

                if (*endptr != '\0' || optval < 0 || optval > 255 || endptr == optarg)
                    die(EXIT_MISTAKE, 0, "%s: invalid edit distance argument", optarg);
            }
                break;
            case 'h':
                usage(EXIT_SUCCESS);
                break;
            case 'p': {
                FILE *fp;
                struct stat st;
                char *buffer;
                unsigned long size;
                long len;

                if ((fp = fopen(optarg, "r")) == NULL || stat(optarg, &st) != 0)
                    die(EXIT_MISTAKE, errno, "%s", optarg);
                else if (st.st_size == 0)
                    exit(EXIT_NOMATCH);

                for (size = 1, buffer = malloc(1); (len = getline(&buffer, &size, fp)) > 0; size = 1, buffer = malloc(1)) { /* FIXME: getline is not portable */
                    if (buffer[len - 1] == '\n')
                        buffer[--len] = '\0';
                    if (buffer[len - 1] == '\r')
                        buffer[--len] = '\0';

                    if (len < 1) {
                        free(buffer);
                        continue;
                    }

                    patterns = realloc(patterns, ++patterns_count * sizeof(void *));
                    patterns[patterns_count - 1] = buffer;
                }

                free(buffer);
                fclose(fp);
            }
                break;
            case 'v':
                ++loglevel;
                break;
            default:
                usage(EXIT_MISTAKE);
        }
    }

    if (optind >= argc)
        usage(EXIT_MISTAKE);

    if (max_edit > 0 && algorithm != NULL && !algorithm->approximate)
        die(EXIT_MISTAKE, 0, "%s: this algorithm does not support approximate matching", algorithm->id);

    if (patterns_count == 0) {
        patterns_count = 1;
        patterns = malloc(sizeof(void *));
        patterns[0] = argv[optind++];
    }

    for (files = malloc((argc - optind) * sizeof(void *)); optind < argc; ++optind) {
        FILE *fp;
        const char *filename = argv[optind];
        struct stat st;

        if ((fp = fopen(filename, "r+")) == NULL || stat(filename, &st) != 0)
            die(EXIT_MISTAKE, errno, "%s", filename);
        else if (st.st_size == 0) {
            fclose(fp);
            log_info(loglevel, "%s: empty file, ignoring", filename);
        } else {
            files[files_count] = malloc(sizeof(struct file));
            files[files_count]->fp = fp;
            files[files_count]->name = filename;
            ++files_count;
        }
    }

    if (files_count == 0)
        die(EXIT_MISTAKE, 0, "no files to search");

    context = malloc(sizeof(struct algorithm_context));
    context->files = files;
    context->files_count = files_count;
    context->patterns = patterns;
    context->patterns_counts = patterns_count;
    context->max_edit = max_edit;
    context->only_count = only_show_count;

    if (algorithm == NULL)
        algorithm = choose_algorithm(context);

    if (loglevel >= DEBUG) {
        unsigned int i;

        log_debug(loglevel, "algorithm=%s, only_count=%s, max_edit=%d, patterns_count=%d, files_count=%d", algorithm->id, only_show_count ? "true" : "false", max_edit, patterns_count, files_count);

        for (i = 0; i < patterns_count; ++i)
            log_debug(loglevel, "pattern %d: |%s|", i + 1, patterns[i]);

        for (i = 0; i < files_count; ++i)
            log_debug(loglevel, "file %d: %s", i + 1, files[i]->name);
    }

    return algorithm->search(context);
}
