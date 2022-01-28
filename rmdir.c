#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv) {
    if (argc == 1) {
        fprintf(stderr, "usage: %s dir...\n", *argv);
        return 1;
    }

    while (*++argv != NULL) {
        if (rmdir(*argv) != 0) {
            fprintf(stderr, "rmdir: %s: %s\n", *argv, strerror(errno));
            return 1;
        }
    }
    return 0;
}
