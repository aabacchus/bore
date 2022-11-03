#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char **argv) {
    char *base, *suffix;

    // TODO: handle `basename -- string`
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: %s string [suffix]\n", argv[0]);
        return 1;
    }

    base = basename(argv[1]);
    if (argc == 3) {
        suffix = strstr(base, argv[2]);
        if (suffix != NULL)
            *suffix = '\0';
    }

    printf("%s\n", base);
    return 0;
}
