#define _XOPEN_SOURCE 700
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int
rm(char *path) {
    struct stat sb;
    if (stat(path, &sb) == -1) {
        fprintf(stderr, "rm: %s: %s\n", path, strerror(errno));
        return 1;
    }
    if (unlink(path) == -1) {
        fprintf(stderr, "rm: %s: %s\n", path, strerror(errno));
        return 1;
    }
    return 0;
}

int
main(int argc, char **argv) {
    int c, ret = 0;
    while ((c = getopt(argc, argv, "")) != -1) {
        switch (c) {
            default:
                fprintf(stderr, "usage: %s file...\n", argv[0]);
                return 1;
        }
    }

    argc -= optind;
    argv += optind;

    while (argc--)
        if (rm(*argv++) != 0)
            ret = 1;
    
    return ret;
}
