#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
head(FILE *f, int n) {
    int i;
    size_t len = LINE_MAX;
    char *buf = malloc(len);
    if (buf == NULL) {
        fprintf(stderr, "head: %s\n", strerror(errno));
        return 1;
    }
    for (i = 0; i < n; i++) {
        if (getline(&buf, &len, f) == -1) {
            if (ferror(f)) {
                fprintf(stderr, "head: %s\n", strerror(errno));
                return 1;
            }
            if (feof(f))
                break;
        }
        printf(buf);
    }

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
