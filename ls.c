#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

enum {
    FLAG_1 = 1 << 0
};

int
ls(const char *path, int flags) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        return 1;
    }
    if (S_ISDIR(st.st_mode)) {
        /* directory */
        DIR *dir = opendir(path);
        if (dir == NULL) {
            fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
            return 1;
        }

        struct dirent *dp;
        while ((dp = readdir(dir)) != NULL) {
            printf("%s", dp->d_name);

            if (flags & FLAG_1)
                putc('\n', stdout);
            else
                putc(' ', stdout);
        }
    } else {
        /* file */
        printf("%s", path);
        if (flags & FLAG_1)
            putc('\n', stdout);
        else
            putc(' ', stdout);
    }
    return 0;
}

int
main(int argc, char **argv) {
    int c, flags, ret_val;
    flags = ret_val = 0;

    while ((c = getopt(argc, argv, "1l")) != -1) {
        switch (c) {
            case '1':
            case 'l':
                flags |= FLAG_1;
                break;
        }
    }
    argv += optind - 1;

    if(!isatty(1))
        flags |= FLAG_1;

    if (argc == optind)
        ret_val = ls(".", flags);
    else while (*++argv)
        ret_val = ls(*argv, flags);

    if (!flags & FLAG_1)
        puts(""); /* final newline */

    return ret_val;
}
