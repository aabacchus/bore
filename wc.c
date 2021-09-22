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

/* function to handle which counts to print, given the flags.
 * give the fname as NULL in order not to print a name.
 */
void
print_count(struct count *ct, char *fname, int flags) {
    if (flags == (FLAG_c | FLAG_l | FLAG_w)) {
        /* default */
        printf("%8ld%8ld%8ld", ct->l, ct->w, ct->c);
    } else {
        char *format_str = " %8d";
        char *fstr = "%d";
        if (flags & FLAG_l) {
            printf(fstr, ct->l);
            fstr = format_str;
        }
        if (flags & FLAG_w) {
            printf(fstr, ct->w);
            fstr = format_str;
        }
        if (flags & FLAG_c) {
            printf(fstr, ct->c);
            fstr = format_str;
        }
    }

    if (fname)
        printf(" %s", fname);
    printf("\n");
}


int
wc(struct count *ct, FILE *f) {
    int c, in_word = 0;
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n')
            ++ct->l;
        if (isspace(c))
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
    flags = 0;
    ret_val = 0;
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
            default:
                flags = FLAG_c | FLAG_l | FLAG_w;
                break;
        }
    }
    /* if no options given */
    if (optind == 1)
        flags = FLAG_c | FLAG_l | FLAG_w;

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

        if (fileno(f) != 0 && fclose(f) == EOF) {
            fprintf(stderr, "wc: %s: %s\n", *argv, strerror(errno));
            ret_val = 1;
        }

        print_count(&ct, *argv, flags);
        global.l += ct.l;
        global.w += ct.w;
        global.c += ct.c;
    }
    if (optind + 1 == argc)
        return ret_val;

    /* print globals */
    if (optind == argc)
        print_count(&global, NULL, flags);
    else
        print_count(&global, "total", flags);
    return ret_val;
}
