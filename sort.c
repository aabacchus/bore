#define _XOPEN_SOURCE 700
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum FLAGS {
    FLAG_r = 1 << 0,
};

int flags;

int
alphacompare(const void *s1, const void *s2) {
    int ret = strcmp(*(const char **) s1, *(const char **) s2);
    if (flags & FLAG_r)
        ret *= -1;
    return ret;
}

int
sort(FILE *f) {
    char **lines = malloc(sizeof(char *));
    if (lines == NULL) {
        fprintf(stderr, "sort: %s\n", strerror(errno));
        return 1;
    }
    size_t lsize = sizeof(char *);
    size_t lused = 0;
    int i = 0;
    char *buf = malloc(LINE_MAX);
    if (buf == NULL) {
        fprintf(stderr, "sort: %s\n", strerror(errno));
        free(lines);
        return 1;
    }
    ssize_t bufused = 0;
    size_t bufsize = LINE_MAX;
    while ((bufused = getline(&buf, &bufsize, f)) != -1) {
        if (bufused + lused > lsize) {
            char **tmp = realloc(lines, bufused + lused);
            if (tmp == NULL) {
                fprintf(stderr, "sort: %s\n", strerror(errno));
                return 1;
            }
            lused += bufused;
            lines = tmp;
        }
        lines[i] = strndup(buf, bufused);
        //strncpy(lines[i], buf, bufused);
        lines[i][bufused] = '\0';
        i++;
        bufused = 0;
    }
    if (ferror(f)) {
        fprintf(stderr, "sort: %s\n", strerror(errno));
        free(lines);
        free(buf);
        return 1;
    }

    qsort(lines, i, sizeof lines[0], alphacompare);

    for (int j = 0; j < i; j++) {
        printf("%s", lines[j]);
    }

    free(lines);
    free(buf);
    return 0;
}

int
main(int argc, char **argv) {
    int c;
    int ret = 0;
    flags = 0;
    while ((c = getopt(argc, argv, "r")) != -1) {
        switch (c) {
            case 'r':
                flags |= FLAG_r;
                break;
        }
    }
    argc -= optind;
    argv += optind - 1;

    if (argc == 0)
        return sort(stdin);

    while (*++argv) {
        FILE *f;
        if (**argv == '-' && *(*argv + 1) == '\0')
            f = stdin;
        else {
            f = fopen(*argv, "r");
            if (f == NULL) {
                fprintf(stderr, "sort: %s: %s\n", *argv, strerror(errno));
                ret = 1;
                continue;
            }
        }
        if (sort(f) != 0)
            ret = 1;

        if (f != stdin && fclose(f) == EOF) {
            fprintf(stderr, "sort: %s: %s\n", *argv, strerror(errno));
            ret = 1;
            continue;
        }
    }
    return ret;
}

