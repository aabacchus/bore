#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
cat(FILE *fp, const char *fname) {
    int c = -1;

    while ((c = fgetc(fp)) != EOF) {
        putchar(c);
    }

    /* Check whether the EOF came from an end-of-file or an error. */
    if ((ferror(fp))) {
        fprintf(stderr, "cat: fgetc %s: %s\n", fname, strerror(errno));
        return 1;
    }

    return 0;
}

int
main(int argc, char **argv) {
    if (argc > 1 && **(argv + 1) == '-' && *(*(argv + 1) + 1) == 'u') {
        /* ignore -u */
        argv++;
        argc--;
    }

    if (argc == 1) {
        return cat(stdin, "stdin");
    }

    int err = 0;
    const char *fname = NULL;
    FILE *fp = NULL;

    while (*++argv) {
        if (**argv == '-' && *(*argv + 1) == '\0') {
            fp = stdin;
            fname = "stdin";
        } else {
            fp = fopen(*argv, "r");
            if (!fp) {
                fprintf(stderr, "cat: fopen %s: %s\n", *argv, strerror(errno));
                err = 1;
                continue;
            }
            fname = *argv;
        }

        if ((cat(fp, fname)) == 1) {
            err = 1;
        }

        if (fp != stdin && (fclose(fp)) == EOF) {
            fprintf(stderr, "cat: fclose %s: %s\n", fname, strerror(errno));
            err = 1;
        }
    }

    return err;
}
