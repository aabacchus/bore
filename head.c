#define _XOPEN_SOURCE 700
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
head(FILE *f, int n) {
    int i;
    ssize_t bytes;
    char *buf = NULL;
    size_t len = 0;
    for (i = 0; i < n; i++) {
        bytes = getline(&buf, &len, f);
        if (bytes == -1) {
            if (ferror(f)) {
                fprintf(stderr, "head: %s\n", strerror(errno));
                return 1;
            }
            if (feof(f))
                break;
        }
        if (write(1, buf, bytes) == -1) {
            fprintf(stderr, "head: %s\n", strerror(errno));
            return 1;
        }
    }
    free(buf);

    return 0;
}

int
main(int argc, char **argv) {
    int n = 10;
    int ret = 0;

    int c;
    while ((c = getopt(argc, argv, ":n:")) != -1) {
        switch (c) {
            case 'n':
                n = atoi(optarg);
                if (n <= 0) {
                    fprintf(stderr, "head: invalid number '%s'\n", optarg);
                    return 1;
                }
                break;
            case ':':
            case '?':
                fprintf(stderr, "usage: %s [-n num] [file...]\n", *argv);
                return 1;
        }
    }

    argc -= optind;
    argv += optind - 1;


    if (argc == 0)
        return head(stdin, n);

    FILE *f;
    while (*++argv) {
        if (**argv == '-' && *(*argv + 1) == '\0')
            f = stdin;
        else {
            f = fopen(*argv, "r");
            if (f == NULL) {
                fprintf(stderr, "head: %s: %s\n", *argv, strerror(errno));
                ret = 1;
                continue;
            }
        }
        if (head(f, n) != 0)
            ret = 1;

        if (f != stdin && fclose(f) == EOF) {
            fprintf(stderr, "head: %s: %s\n", *argv, strerror(errno));
            ret = 1;
            continue;
        }
    }
    return ret;
}
