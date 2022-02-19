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
    FLAG_n = 1 << 1,
    FLAG_v = 1 << 2,
};

int
match(const char *string, regex_t *re) {
    int status = regexec(re, string, 0, NULL, 0);
    if (status != 0)
        return 0;
    return 1;
}

int
grep(FILE *f, regex_t *re, int flags, char *fname) {
    size_t len = LINE_MAX;
    ssize_t bytes;
    char *buf = malloc(len);
    if (buf == NULL) {
        fprintf(stderr, "grep: %s\n", strerror(errno));
        return 1;
    }

    int num_matches = 0;
    int lineno = 0;
    while ((bytes = getline(&buf, &len, f)) != -1) {
        lineno++;
        int matches = match(buf, re);
        if ((matches && ! (flags & FLAG_v)) || (! matches && (flags & FLAG_v))) {
            num_matches++;
            if (! (flags & FLAG_c)) {
                if (fname)
                    printf("%s:", fname);
                if (flags & FLAG_n)
                    printf("%d:", lineno);
                printf("%s", buf);
            }
        }
    }

    if (flags & FLAG_c) {
        if (fname)
            printf("%s:", fname);
        printf("%d\n", num_matches);
    }

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
    fprintf(stderr, "usage: grep [-E|-F] [-cinv] regexp [file...]\n");
}

int
main(int argc, char **argv) {
    int ret = 1;

    int c;
    int REGFLAGS = REG_NOSUB;
    int flags = 0;
    while ((c = getopt(argc, argv, "EFcinv")) != -1) {
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
            case 'i':
                REGFLAGS |= REG_ICASE;
                break;
            case 'n':
                flags |= FLAG_n;
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

    /* TODO: allow '--' after pattern_list
     *       make a list of regex_ts to do for the -e and -f options
     */

    char *regexp = *++argv;
    regex_t re;
    int regerr = regcomp(&re, regexp, REGFLAGS);
    if (regerr != 0) {
        char errbuf[100] = {'\0'};
        regerror(regerr, &re, errbuf, 100);
        fprintf(stderr, "grep: bad regex: %s\n", errbuf);
        regfree(&re);
        return 1;
    }

    if (argc == 1) {
        ret = grep(stdin, &re, flags, NULL);
    } else {
        FILE *f;
        while (*++argv) {
            char *fname = NULL;
            if (**argv == '-' && *(*argv + 1) == '\0') {
                f = stdin;
                fname = "stdin";
            } else {
                f = fopen(*argv, "r");
                if (f == NULL) {
                    fprintf(stderr, "grep: %s: %s\n", *argv, strerror(errno));
                    ret = 1;
                    continue;
                }
                fname = *argv;
            }

            if (argc == 2)
                fname = NULL;

            /* grep only returns 1 or 0. */
            ret *= grep(f, &re, flags, fname);

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
