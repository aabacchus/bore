#include <errno.h>
#include <ftw.h>
#include <stdio.h>
#include <string.h>

int
foreach(const char *name, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
    printf("%s\n", name);
    /* fnmatch
     * detect infinite loops
     */
    return 0;
}

int
main(int argc, char **argv) {
    char *argv0 = argv[0];
    if (argc < 2) {
        fprintf(stderr, "usage: %s path...\n", argv0);
        return 1;
    }
    while (*(++argv)) {
        if (nftw(*argv, foreach, 20, 0) != 0) {
            fprintf(stderr, "%s: %s: %s\n", argv0, *argv, strerror(errno));
            return 1;
        }
    }
    return 0;
}
