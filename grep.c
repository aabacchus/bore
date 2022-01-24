#define _XOPEN_SOURCE 700
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum {
    FLAG_c = 1 << 0,
    FLAG_v = 1 << 1,
};

int
match(const char *string, regex_t *re) {
    int status = regexec(re, string, 0, NULL, 0);

    if (status != 0)
        return 0;
    return 1;
}

int
grep(FILE *f, regex_t *re, int flags) {
    size_t len = LINE_MAX;
    ssize_t bytes;
    char *buf = malloc(len);
    if (buf == NULL) {
        fprintf(stderr, "grep: %s\n", strerror(errno));
        return 1;
    }

    int num_matches = 0;
    while ((bytes = getline(&buf, &len, f)) != -1) {
        int matches = match(buf, re);
        if ((matches && ! (flags & FLAG_v)) || (! matches && (flags & FLAG_v))) {
            num_matches++;
            if (! (flags & FLAG_c) && write(1, buf, bytes) == -1) {
                fprintf(stderr, "grep: %s\n", strerror(errno));
                return 1;
            }
        }
    }

    if (flags & FLAG_c)
        printf("%d\n", num_matches);

    if (ferror(f)) {
        fprintf(stderr, "grep: %s\n", strerror(errno));
        return 1;
    }
    free(buf);

    if (num_matches == 0)
        return 1;
    return 0;
}

void
print_usage(void) {
    fprintf(stderr, "usage: grep [-E|-F] [-icv] regexp [file...]\n");
}

int
main(int argc, char **argv) {
    int ret = 1;

    int c;
    int REGFLAGS = REG_NOSUB;
    int flags = 0;
    while ((c = getopt(argc, argv, "EFchiv")) != -1) {
        switch (c) {
            case 'E':
                REGFLAGS |= REG_EXTENDED;
                break;
            case 'F':
                REGFLAGS &= ~REG_EXTENDED;
                break;
            case 'c':
                flags |= FLAG_c;
                break;
            case 'h':
                print_usage();
                return 0;
            case 'i':
                REGFLAGS |= REG_ICASE;
                break;
            case 'v':
                flags |= FLAG_v;
                break;
            case '?':
                print_usage();
                return 1;
        }
    }

    argc -= optind;
    argv += optind - 1;


    if (argc == 0) {
        print_usage();
        return 1;
    }

    char *regexp = *++argv;
    regex_t re;
    regcomp(&re, regexp, REGFLAGS);

    if (argc == 1) {
        ret = grep(stdin, &re, flags);
    } else {
        FILE *f;
        while (*++argv) {
            if (**argv == '-' && *(*argv + 1) == '\0')
                f = stdin;
            else {
                f = fopen(*argv, "r");
                if (f == NULL) {
                    fprintf(stderr, "grep: %s: %s\n", *argv, strerror(errno));
                    ret = 1;
                    continue;
                }
            }

            /* grep only returns 1 or 0. */
            ret *= grep(f, &re, flags);

            if (f != stdin && fclose(f) == EOF) {
                fprintf(stderr, "grep: %s: %s\n", *argv, strerror(errno));
                ret = 1;
                continue;
            }
        }
    }

    regfree(&re);
    return ret;
}
