#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

enum {
    FLAG_1 = 1 << 0
};

struct ent {
    const char *name;
    mode_t mode;
};

int
printname(struct ent *e, int flags) {
    printf("%s", e->name);

    if (flags & FLAG_1)
        putc('\n', stdout);
    else
        putc(' ', stdout);
    return 0;
}

int
ls(const char *path, int flags) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        return 1;
    }
    if (S_ISDIR(st.st_mode)) {
        /* directory */
        int n, i;
        struct dirent **dp;
        n = scandir(path, &dp, 0, alphasort);
        if (n == -1) {
            fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
            return 1;
        }
        for (i = 0; i < n; i++) {
            struct stat stt;
            if (lstat(dp[i]->d_name, &stt) == -1) {
                fprintf(stderr, "ls: %s: %s\n", dp[i]->d_name, strerror(errno));
                return 1;
            }
            struct ent e = {
                dp[i]->d_name,
                stt.st_mode,
            };

            printname(&e, flags);
        }
    } else {
        /* file */
        struct ent e = { .name = path, .mode = st.st_mode };
        printname(&e, flags);
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
