#define _XOPEN_SOURCE 700
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 4096

int
tee(int *fds, int len) {
    unsigned char buf[BUF_SIZE];
    ssize_t n;
    while ((n = read(0, buf, BUF_SIZE)) > 0) {
        int i = 0;
        for (; i < len; i++) {
            if (write(fds[i], buf, n) == -1) {
                fprintf(stderr, "tee: write: %s\n", strerror(errno));
                return 1;
            }
        }
        if (write(1, buf, n) == -1) {
            fprintf(stderr, "tee: write: %s\n", strerror(errno));
            return 1;
        }
    }
    if (n == -1) {
        fprintf(stderr, "tee: read: %s\n", strerror(errno));
        return 1;
    }
    return 0;
}

int
main(int argc, char **argv) {
    int err = 0;
    int flags = O_WRONLY | O_CREAT;

    int c;
    while ((c = getopt(argc, argv, "ai")) != EOF) {
        switch (c) {
            case 'a':
                flags |= O_APPEND;
                break;
            case 'i':
                signal(SIGINT, SIG_IGN);
                break;
        }
    }

    argv += optind - 1;
    int fds[argc - optind];
    int i = 0;

    while (*++argv) {
        fds[i] = open(*argv, flags, 0644);
        if (fds[i++] == -1) {
            fprintf(stderr, "tee: open %s: %s\n", *argv, strerror(errno));
            err = 1;
            continue;
        }
    }
    if (tee(fds, i) != 0) {
        err = 1;
    }
    while (--i > 0) {
        if (close(fds[i]) == -1) {
            fprintf(stderr, "tee: close: %s\n", strerror(errno));
            err = 1;
        }
    }
    return err;
}
