#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

enum {
    FLAG_1 = 1 << 0,
    FLAG_A = 1 << 1,
    FLAG_F = 1 << 2,
    FLAG_a = 1 << 3,
    FLAG_i = 1 << 4,
    FLAG_l = 1 << 5,
    FLAG_p = 1 << 6,
};

struct ent {
    const char *name;
    ino_t ino;
    mode_t mode;
};

int
printname(struct ent *e, int flags) {
    if (flags & FLAG_i)
        printf("%lu ", e->ino);

    printf("%s", e->name);

    char s = 0;
    if (flags & FLAG_p)
        if (S_ISDIR(e->mode))
            s = '/';
    if (flags & FLAG_F) {
        if (S_ISSOCK(e->mode))
            s = '=';
        else if (S_ISDIR(e->mode))
            s = '/';
        else if (S_ISFIFO(e->mode))
            s = '|';
        else if (S_ISLNK(e->mode))
            s = '@';
        else if (e->mode & S_IXUSR || e->mode & S_IXGRP || e->mode & S_IXOTH)
            s = '*';
    }
    if (s != 0)
        putc(s, stdout);

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
        int dfd = open(path, O_RDONLY);
        n = scandir(path, &dp, 0, alphasort);
        if (n == -1) {
            fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
            return 1;
        }
        for (i = 0; i < n; i++) {
            if (~flags & FLAG_a && ~flags & FLAG_A && dp[i]->d_name[0] == '.')
                /* if no -A or -a, skip \.* */
                continue;
            if (flags & FLAG_A && dp[i]->d_name[0] == '.'
                    && (dp[i]->d_name[1] == '\0'
                        || (dp[i]->d_name[1] == '.' && dp[i]->d_name[2] == '\0')))
                /* if -A, skip '.' or '..' */
                continue;

            struct stat stt;
            if (fstatat(dfd, dp[i]->d_name, &stt, AT_SYMLINK_NOFOLLOW) == -1) {
                fprintf(stderr, "ls: %s: %s\n", dp[i]->d_name, strerror(errno));
                return 1;
            }
            struct ent e = {
                dp[i]->d_name,
                stt.st_ino,
                stt.st_mode,
            };

            printname(&e, flags);
        }
        close(dfd);
        free(dp);
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

    while ((c = getopt(argc, argv, "1AFahilp")) != -1) {
        switch (c) {
            case '1':
                flags |= FLAG_1;
                break;
            case 'A':
                flags |= FLAG_A;
                break;
            case 'F':
                flags |= FLAG_F;
                break;
            case 'a':
                flags |= FLAG_a;
                break;
            case 'h':
                printf("usage: %s [-1AFailp]\n", argv[0]);
                return 0;
            case 'i':
                flags |= FLAG_i;
                break;
            case 'l':
                flags |= FLAG_l | FLAG_1;
                break;
            case 'p':
                flags |= FLAG_p;
                break;
        }
    }
    argv += optind - 1;

    if(!isatty(1))
        flags |= FLAG_1;

    if (argc == optind) {
        if (ls(".", flags) != 0)
            ret_val = 1;
    }
    else while (*++argv)
        if (ls(*argv, flags) != 0)
            ret_val = 1;

    if (!(flags & FLAG_1))
        puts(""); /* final newline */

    return ret_val;
}
