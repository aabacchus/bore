#define _XOPEN_SOURCE 700
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int FLAG_f;

int
rm(char *path) {
    struct stat sb;
    if (stat(path, &sb) == -1)
        return 1;

    if (unlink(path) == -1)
        return 1;

    return 0;
}

int
main(int argc, char **argv) {
    int c, ret = 0;
    FLAG_f = 0;
    while ((c = getopt(argc, argv, "f")) != -1) {
        switch (c) {
            case 'f':
                FLAG_f = 1;
                break;
            default:
                fprintf(stderr, "usage: %s [-f] file...\n", argv[0]);
                return 1;
        }
    }

    argc -= optind;
    argv += optind;

    while (argc--) {
        if (rm(*argv++) != 0) {
            if (FLAG_f)
                continue;
            fprintf(stderr, "rm: %s: %s\n", argv[-1], strerror(errno));
            ret = 1;
        }
    }
    
    return ret;
}
