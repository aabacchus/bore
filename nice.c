#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv) {
    int n = 10;
    if (argc > 3 && strcmp("-n", argv[1]) == 0) {
        n = atoi(argv[2]);
        argv += 2;
        argc -= 2;
    }

    if (argc == 1) {
        fprintf(stderr, "usage: nice [-n increment] utility [args]\n");
        return 1;
    }

    errno = 0;
    if (nice(n) == -1 && errno) {
        perror("nice");
    }

    errno = 0;
    execvp(argv[1], argv + 1);

    if (errno) {
        perror(argv[1]);
        return 127;
    }
    return 126;
}
