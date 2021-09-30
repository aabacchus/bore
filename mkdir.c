#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int
main(int argc, char **argv) {
    int c;
    int flag_p = 0;
    mode_t mode = 0755;
    while ((c = getopt(argc, argv, "pm:")) != -1) {
            switch (c) {
                case 'p':
                    flag_p = 1;
                    break;
                case 'm':
                    errno = 0;
                    mode = (mode_t) strtol(optarg, NULL, 8);
                    if (errno != 0) {
                        fprintf(stderr, "mkdir: %s: %s\n", optarg, strerror(errno));
                        return 1;
                    }
                    fprintf(stderr, "got mode = %d\n", mode);
                    break;
            }
    }
    argc -= optind;
    if (argc < 1) {
        fprintf(stderr, "usage: %s file...\n", *argv);
        return 1;
    }
    argv += optind - 1;

    while (*++argv) {
        if (mkdir(*argv, mode) == -1) {
            if (errno == EEXIST && flag_p)
                continue;
            fprintf(stderr, "mkdir: %s: %s\n", *argv, strerror(errno));
            return 1;
        }
    }

    return 0;
}
