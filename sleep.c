#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s time\n", *argv);
        return 1;
    }

    errno = 0;
    unsigned long t = strtoul(argv[1], NULL, 10);
    if (errno || (signed long)t < 0)
        return 1;
    while (t > 0)
        t = sleep(t);
    return 0;
}
