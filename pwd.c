#define _XOPEN_SOURCE 700
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char **argv) {
    int c, flag_L = 1;
    long size;
    while ((c = getopt(argc, argv, "LP")) != -1) {
        switch (c) {
            case 'L':
                flag_L = 1;
                break;
            case 'P':
                flag_L = 0;
                break;
            case '?':
                fprintf(stderr, "usage: %s [-L|-P]\n", argv[0]);
                return 1;
        }
    }

    if (flag_L) {
        char *cwd = getenv("PWD");
        if (cwd == NULL)
            goto flag_P;
        else if (*cwd != '/')
            goto flag_P;
        else if (strstr(cwd, "/./") != NULL)
            goto flag_P;
        else if (strstr(cwd, "/../") != NULL)
            goto flag_P;

        /* POSIX.1-2017 does not specify what to do if $PWD is longer than PATH_MAX bytes;
         * this implementation chooses to print the pathname found from PWD in this case.
         */

        puts(cwd);
        return 0;
    }

flag_P:
    size = pathconf(".", _PC_PATH_MAX);
    if (size == -1)
        size = PATH_MAX;

    char *buf = NULL;
    for (char *tmp = NULL; tmp == NULL; size += 32) {
        char *b = realloc(buf, size);
        if (b == NULL) {
            free(buf);
            perror("pwd");
            return 1;
        }
        buf = b;

        errno = 0;
        tmp = getcwd(buf, size);
        if (tmp == NULL && errno != ERANGE) {
            perror("pwd");
            free(buf);
            return 1;
        }
    }

    puts(buf);
    free(buf);
    return 0;
}
