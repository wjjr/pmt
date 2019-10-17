/* pmt: Copyright (c) 2019 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include "algorithms.h"
#include "log.h"

static const char optstring[] = "a:bce:hop:qv";
static const struct option longopts[] = {
        {"algorithm",     required_argument, NULL, 'a'},
        {"byte-offset",   no_argument,       NULL, 'b'},
        {"count",         no_argument,       NULL, 'c'},
        {"edit",          required_argument, NULL, 'e'},
        {"help",          no_argument,       NULL, 'h'},
        {"only-matching", no_argument,       NULL, 'o'},
        {"pattern",       required_argument, NULL, 'p'},
        {"quiet",         no_argument,       NULL, 'q'},
        {"verbose",       no_argument,       NULL, 'v'},
        {NULL,            no_argument,       NULL, '\0'}
};

static void usage(const int status, const char *const progname) {
    const struct algorithm *algorithms, *algorithm;
    int i;

    fprintf((status != 0) ? stderr : stdout, "Usage: %s [-a ALGO] [-c] [-e DIST] [-h] (PATTERN | -p PATTERN_FILE) FILE [FILE...]\n", progname);

    if (status != 0) {
        fprintf(stderr, "Try '%s --help' for more information.\n", progname);
        exit(status);
    }

    printf("Search for PATTERN in each FILE.\n");
    printf("Example: %s 'Romeo' shakespeare.txt\n\n", progname);
    printf("  -a, --algorithm=NAME  selects the algorithm used in the search\n");
    printf("  -b, --byte-offset     print byte offset of the first character of each output line\n");
    printf("  -c, --count           print only a total count of matches\n");
    printf("  -e, --edit=NUM        find patterns with a maximum edit distance of NUM operations\n");
    printf("  -h, --help            display this help text and exit\n");
    printf("  -o, --only-matching   print only the matching parts\n");
    printf("  -p, --pattern=FILE    obtain patterns from FILE, one per line\n");

    printf("\nSupported algorithms:\n");
    for (i = 0, algorithms = get_algorithms(); (algorithm = &algorithms[i])->id != NULL; ++i)
        printf("%-3s : %s%s\n", algorithm->id, algorithm->name, algorithm->approximate ? " [approximate]" : "");

    exit(status);
}

int main(int argc, char *argv[]) {
    const char *progname = "pmt";
    int opt;
    const struct algorithm *algorithm = NULL;
    bool print_byte_offset = false, only_show_count = false, only_matching = false;
    uint_8 edit_distance = 0;
    struct pattern *patterns = NULL;
    struct file *files = NULL;
    usize num_patterns = 0, num_files = 0, i;
    struct search_context *context;

    while ((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
        switch (opt) {
            case 'a':
                if ((algorithm = get_algorithm(optarg)) == NULL)
                    die(EXIT_MISTAKE, 0, "%s: unknown algorithm", optarg);
                break;
            case 'b':
                print_byte_offset = true;
                break;
            case 'c':
                only_show_count = true;
                break;
            case 'e': {
                char *endptr;
                long optval = strtol(optarg, &endptr, 10);
                edit_distance = (uint_8) optval;

                if (*endptr != '\0' || optval < 0 || optval > 255 || endptr == optarg)
                    die(EXIT_MISTAKE, 0, "%s: invalid edit distance argument", optarg);
            }
                break;
            case 'h':
                usage(EXIT_SUCCESS, progname);
                break;
            case 'o':
                only_matching = true;
                break;
            case 'p': {
                FILE *fp;
                struct stat st;
                byte *buffer;
                usize buffer_size;
                ssize len;

                if ((fp = fopen(optarg, "r+b")) == NULL || stat(optarg, &st) != 0)
                    die(EXIT_FAILURE, errno, "%s", optarg);
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
                    patterns[num_patterns - 1].length = (usize) len;
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
                usage(EXIT_MISTAKE, progname);
        }
    }

    if (optind >= argc) /* user has not specified any files */
        usage(EXIT_MISTAKE, progname);

    if (edit_distance > 0 && algorithm != NULL && !algorithm->approximate)
        die(EXIT_MISTAKE, 0, "%s: algorithm does not support approximate matching", algorithm->id);

    if (num_patterns > 1 && algorithm != NULL && !algorithm->parallel)
        log_print(WARN, "%s: algorithm does not support parallel search", algorithm->id);

    if (num_patterns == 0) {
        num_patterns = 1;
        patterns = malloc(sizeof(struct pattern));
        patterns[0].string = (byte *) argv[optind];
        patterns[0].length = strlen(argv[optind]);
        ++optind;
    }

    for (files = malloc(((usize) (argc - optind)) * sizeof(struct file)); optind < argc; ++optind) {
        FILE *fp;
        const char *filename = argv[optind];
        struct stat st;

        if ((fp = fopen(filename, "r+b")) == NULL || stat(filename, &st) != 0)
            die(EXIT_FAILURE, errno, "%s", filename);
        else if (st.st_size == 0) {
            fclose(fp);
            log_print(INFO, "%s: empty file, ignoring", filename);
        } else {
            files[num_files].fp = fp;
            files[num_files].name = filename;
            files[num_files].size = (usize) st.st_size;
            ++num_files;
        }
    }

    if (num_files == 0)
        die(EXIT_MISTAKE, 0, "no files to search");

    context = malloc(sizeof(struct search_context));
    context->files = files;
    context->num_files = num_files;
    context->patterns = patterns;
    context->num_patterns = num_patterns;
    context->edit_distance = edit_distance;
    context->only_count = only_show_count;
    context->only_matching = only_matching;
    context->print_byte_offset = print_byte_offset;

    if (algorithm == NULL)
        algorithm = choose_algorithm(context);

    log_debug(DEBUG, "algorithm=%s, only_count=%s, only_matching=%s, print_byte_offset=%s, edit_distance=%" PRIu8 ", num_patterns=%" PRIuSIZ ", num_files=%" PRIuSIZ,
              algorithm->id, only_show_count ? "true" : "false", only_matching ? "true" : "false", print_byte_offset ? "true" : "false", edit_distance, num_patterns, num_files);
    for (i = 0; i < num_patterns; ++i) log_debug(DEBUG, "pattern %" PRIuSIZ ": |%s| (length: %" PRIuSIZ " bytes)", i + 1, patterns[i].string, patterns[i].length);
    for (i = 0; i < num_files; ++i) log_debug(DEBUG, "filename %" PRIuSIZ ": |%s| (size: %" PRIuSIZ " bytes)", i + 1, files[i].name, files[i].size);

    return algorithm->search(context);
}
