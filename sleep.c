#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void
sig_handler(int sig) {
    (void)sig;
    exit(0);
}

int
main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s time\n", *argv);
        return 1;
    }

    signal(SIGALRM, sig_handler);

    errno = 0;
    unsigned long t = strtoul(argv[1], NULL, 10);
    if (errno || (signed long)t < 0)
        return 1;
    while (t > 0)
        t = sleep(t);
    return 0;
}
