#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

enum {
    FLAG_1 = 1 << 0,
    FLAG_A = 1 << 1,
    FLAG_F = 1 << 2,
    FLAG_a = 1 << 3,
    FLAG_c = 1 << 4,
    FLAG_i = 1 << 5,
    FLAG_l = 1 << 6,
    FLAG_m = 1 << 7,
    FLAG_p = 1 << 8,
    FLAG_u = 1 << 9,
    FLAG_q = 1 << 10,
};

struct ent {
    const char *name;
    ino_t ino;
    mode_t mode;
    nlink_t nlink;
    uid_t uid;
    gid_t gid;
    off_t size;
    dev_t rdev;
    struct timespec tim;
};

void
pretty_print_perms(mode_t mode) {
    /* entry type */
    if (S_ISDIR(mode))
        putchar('d');
    else if (S_ISBLK(mode))
        putchar('b');
    else if (S_ISCHR(mode))
        putchar('c');
    else if (S_ISLNK(mode))
        putchar('l');
    else if (S_ISFIFO(mode))
        putchar('p');
    else
        putchar('-');

    /* loop over 0400, 0200, 0100, 040, 020, etc and print the relevant char */
    char *perms = "rwx";
    for (int p = 2; p >= 0; p--) {
        for (int i = 04; i >= 1; i /= 2) {
            /* the following is a way of doing j = 010 to the power p */
            int j = 01;
            for (int k = 0; k < p; k++)
                j *= 010;

            if (mode & (i * j))
                printf("%c", perms[2 - i/2]);
            else
                putchar('-');
        }
    }
    /* alternate method flag */

    putchar(' ');
}

void
qprint(const char *name, int qflag) {
    for (const char *c = name; *c; c++) {
        if (isprint(*c) || !qflag)
            putchar(*c);
        else
            putchar('?');
    }
}

int
printname(struct ent *e, uint32_t flags) {
    if (flags & FLAG_i)
        printf("%lu ", e->ino);
    if (flags & FLAG_l) {
        /* file mode */
        pretty_print_perms(e->mode);

        /* number of links */
        printf("%4lu ", e->nlink);

        /* owner name */
        errno = 0;
        struct passwd *pwd = getpwuid(e->uid);
        if (pwd == NULL) {
            if (errno) {
                fprintf(stderr, "ls: %s: %s\n", e->name, strerror(errno));
                return 1;
            } else {
                /* couldn't find a user with that uid; print the numeric value */
                printf("%-8d ", e->uid);
            }
        } else {
            printf("%-8.8s ", pwd->pw_name);
        }

        /* group name */
        errno = 0;
        struct group *grp = getgrgid(e->gid);
        if (grp == NULL) {
            if (errno) {
                fprintf(stderr, "ls: %s: %s\n", e->name, strerror(errno));
                return 1;
            } else {
                /* couldn't find a group with that gid; print the numeric value */
                printf("%-8d ", e->gid);
            }
        } else {
            printf("%-8.8s ", grp->gr_name);
        }

        /* size (or device info for character/block special files) */
        if (S_ISBLK(e->mode) || S_ISCHR(e->mode))
            printf("%8ld ", e->rdev);
        else
            printf("%8ld ", e->size);

        /* date and time */
        struct tm *tm = localtime(&e->tim.tv_sec);
        if (tm == NULL) {
            fprintf(stderr, "ls: %s: %s\n", e->name, strerror(errno));
            return 1;
        }
        char buf[sizeof "MMM DD HH:MM"];
        strftime(buf, sizeof(buf), "%b %e %H:%M", tm);
        printf("%s ", buf);
    }

    qprint(e->name, flags & FLAG_q);

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
        putchar(s);

    if (flags & FLAG_m)
        putchar(',');
    if (flags & FLAG_1)
        putchar('\n');
    else
        putchar(' ');
    return 0;
}

int
ls(const char *path, uint32_t flags) {
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
                stt.st_nlink,
                stt.st_uid,
                stt.st_gid,
                stt.st_size,
                stt.st_rdev,
                stt.st_mtim,
            };
            if (flags & FLAG_u)
                e.tim = stt.st_atim;
            else if (flags & FLAG_c)
                e.tim = stt.st_ctim;


            printname(&e, flags);
        }
        close(dfd);
        free(dp);
    } else {
        /* file */
        struct ent e = {
            .name = path,
            .ino  = st.st_ino,
            .mode = st.st_mode,
            .nlink = st.st_nlink,
            .uid = st.st_uid,
            .gid = st.st_gid,
            .size = st.st_size,
            .rdev = st.st_rdev,
            .tim = st.st_mtim,
        };
        if (flags & FLAG_u)
            e.tim = st.st_atim;
        else if (flags & FLAG_c)
            e.tim = st.st_ctim;

        printname(&e, flags);
    }
    return 0;
}

int
main(int argc, char **argv) {
    int c, ret_val;
    uint32_t flags;
    flags = ret_val = 0;

    while ((c = getopt(argc, argv, "1AFachilmpuq")) != -1) {
        switch (c) {
            case '1':
                flags |= FLAG_1 | FLAG_q;
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
            case 'c':
                flags |= FLAG_c;
                flags &= ~FLAG_u;
                break;
            case 'h':
                printf("usage: %s [-1AFacilmpuq]\n", argv[0]);
                return 0;
            case 'i':
                flags |= FLAG_i;
                break;
            case 'l':
                flags |= FLAG_l | FLAG_1;
                flags &= ~FLAG_m;
                break;
            case 'm':
                flags |= FLAG_m;
                flags &= ~(FLAG_1 | FLAG_l);
                break;
            case 'p':
                flags |= FLAG_p;
                break;
            case 'u':
                flags |= FLAG_u;
                flags &= ~FLAG_c;
                break;
            case 'q':
                flags |= FLAG_q;
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
