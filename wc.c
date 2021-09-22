#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

enum {
    FLAG_c = 1 << 0,
    FLAG_l = 1 << 1,
    FLAG_w = 1 << 2,
};

struct count {
    long w, c, l;
};

int wc(struct count *ct, FILE *f) {
    int c, in_word = 0;
    while ((c = fgetc(f)) != EOF) {
		if (c == '\n')
            ++ct->l;
		if (c == ' ' || c == '\t' || c == '\n')
            in_word=0;
		else if (in_word == 0) {
			in_word = 1;
			++ct->w;
		}
		++ct->c;
    }
    if (ferror(f)) {
        fprintf(stderr, "wc: %s\n", strerror(errno));
        return 1;
    }
    return 0;
}

int
main(int argc, char **argv) {
    int c, flags, ret_val;
    flags = ret_val = 0;
    struct count global = { 0 };

    while ((c = getopt(argc, argv, "clw")) != -1) {
        switch (c) {
            case 'c':
                flags |= FLAG_c;
                break;
            case 'l':
                flags |= FLAG_l;
                break;
            case 'w':
                flags |= FLAG_w;
                break;
            break;
        }
    }
    argv += optind - 1;

    if (optind == argc) {
        if (wc(&global, stdin) != 0)
            ret_val = 1;
    } else while (*++argv) {
        struct count ct = { 0 };
        FILE *f;
        if (**argv == '-')
            f = stdin;
        else {
            f = fopen(*argv, "r");
            if (f == NULL) {
                fprintf(stderr, "wc: %s: %s\n", *argv, strerror(errno));
                ret_val = 1;
                continue;
            }
        }
        if (wc(&ct, f) != 0) {
            ret_val = 1;
            continue;
        }
        printf("%8ld%8ld%8ld %s\n", ct.l, ct.w, ct.c, *argv);
        global.l += ct.l;
        global.w += ct.w;
        global.c += ct.c;
    }
    if (optind + 1 == argc)
        return ret_val;

    /* print globals */
    printf("%8ld%8ld%8ld", global.l, global.w, global.c);
    if (optind == argc)
        printf(" total\n");
    else
        printf("\n");
    return ret_val;
}
