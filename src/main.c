#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include "algorithms.h"
#include "error.h"

static const char optstring[] = "a:ce:hp:qv";
static const struct option longopts[] = {
        {"algorithm", required_argument, NULL, 'a'},
        {"count",     no_argument,       NULL, 'c'},
        {"edit",      required_argument, NULL, 'e'},
        {"help",      no_argument,       NULL, 'h'},
        {"pattern",   required_argument, NULL, 'p'},
        {"quiet",     no_argument,       NULL, 'q'},
        {"verbose",   no_argument,       NULL, 'v'},
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
    uint_8 only_show_count = 0, max_edit = 0;
    struct pattern *patterns = NULL;
    struct file *files = NULL;
    uint_64 num_patterns = 0, num_files = 0;
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
                max_edit = (uint_8) optval;

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
                byte *buffer;
                uint_64 buffer_size;
                int_64 len;

                if ((fp = fopen(optarg, "r")) == NULL || stat(optarg, &st) != 0)
                    die(EXIT_MISTAKE, errno, "%s", optarg);
                else if (st.st_size == 0)
                    exit(EXIT_NOMATCH);

                for (buffer_size = 1, buffer = malloc(1); (len = getline((char **) &buffer, &buffer_size, fp)) > 0; buffer_size = 1, buffer = malloc(1)) { /* FIXME: getline is not portable */
                    if (buffer[len - 1] == '\n')
                        buffer[--len] = '\0';
                    if (buffer[len - 1] == '\r')
                        buffer[--len] = '\0';

                    if (len < 1) {
                        free(buffer);
                        continue;
                    }

                    patterns = realloc(patterns, ++num_patterns * sizeof(struct pattern));
                    patterns[num_patterns - 1].string = buffer;
                    patterns[num_patterns - 1].length = (uint_64) len;
                }

                free(buffer);
                fclose(fp);
            }
                break;
            case 'q':
                log_silence();
                break;
            case 'v':
                log_increase_level();
                break;
            default:
                usage(EXIT_MISTAKE);
        }
    }

    if (optind >= argc)
        usage(EXIT_MISTAKE);

    if (max_edit > 0 && algorithm != NULL && !algorithm->approximate)
        die(EXIT_MISTAKE, 0, "%s: this algorithm does not support approximate matching", algorithm->id);

    if (num_patterns > 1 && algorithm != NULL && !algorithm->parallel)
        log_print(WARN, "%s: algorithm does not support parallel search", algorithm->id);

    if (num_patterns == 0) {
        num_patterns = 1;
        patterns = malloc(sizeof(struct pattern));
        patterns[0].string = (byte *) argv[optind];
        patterns[0].length = strlen(argv[optind]);
        ++optind;
    }

    for (files = malloc(((uint_64) (argc - optind)) * sizeof(struct file)); optind < argc; ++optind) {
        FILE *fp;
        const char *filename = argv[optind];
        struct stat st;

        if ((fp = fopen(filename, "r+")) == NULL || stat(filename, &st) != 0)
            die(EXIT_MISTAKE, errno, "%s", filename);
        else if (st.st_size == 0) {
            fclose(fp);
            log_print(INFO, "%s: empty file, ignoring", filename);
        } else {
            files[num_files].fp = fp;
            files[num_files].name = filename;
            files[num_files].size = (uint_64) st.st_size;
            ++num_files;
        }
    }

    if (num_files == 0)
        die(EXIT_MISTAKE, 0, "no files to search");

    context = malloc(sizeof(struct algorithm_context));
    context->files = files;
    context->num_files = num_files;
    context->patterns = patterns;
    context->num_patterns = num_patterns;
    context->max_edit = max_edit;
    context->only_count = only_show_count;

    if (algorithm == NULL)
        algorithm = choose_algorithm(context);

    if (log_get_loglevel() >= DEBUG) {
        uint_64 i;

        log_print(DEBUG, "algorithm=%s, only_count=%s, max_edit=%d, num_patterns=%d, num_files=%d", algorithm->id, only_show_count ? "true" : "false", max_edit, num_patterns, num_files);

        for (i = 0; i < num_patterns; ++i)
            log_print(DEBUG, "pattern %d: |%s|", i + 1, patterns[i]);

        for (i = 0; i < num_files; ++i)
            log_print(DEBUG, "file %d: |%s|", i + 1, files[i].name);
    }

    return algorithm->search(context);
}
