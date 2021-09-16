#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 4096

int
cat(int fd, char *fname) {
    unsigned char buf[BUF_SIZE];
    ssize_t n;
    while ((n = read(fd, buf, BUF_SIZE)) > 0) {
        if (n == -1){
            fprintf(stderr, "cat: read %s: %s\n", fname, strerror(errno));
            return 1;
        }
        if (write(1, buf, n) == -1) {
            fprintf(stderr, "cat: write: %s\n", strerror(errno));
            return 1;
        }
    }
    return 0;
}

int
main(int argc, char **argv) {
    if (argc > 1 && **(argv+1) == '-' && *(*(argv+1)+1) == 'u') {
        /* ignore -u */
        argv++;
        argc--;
    }
    if (argc == 1)
        return cat(0, "stdin");

    int fd;
    int err = 0;
    char *fname;

    while (*++argv) {
        if (**argv == '-' && *(*argv + 1) == '\0') {
            fd = 0;
            fname = "stdin";
        } else {
            fd = open(*argv, O_RDONLY);
            if (fd == -1) {
                fprintf(stderr, "cat: open %s: %s\n", *argv, strerror(errno));
                err = 1;
                continue;
            }
            fname = *argv;
        }
        if (cat(fd, fname) != 0) {
            err = 1;
            continue;
        }
        if (fd != 0 && close(fd) == -1){
            fprintf(stderr, "cat: close %s: %s\n", fname, strerror(errno));
            err = 1;
        }
    }
    return err;
}
