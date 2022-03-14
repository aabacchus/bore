#define _XOPEN_SOURCE 700
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
    FLAG_S = 1 << 3,
    FLAG_a = 1 << 4,
    FLAG_c = 1 << 5,
    FLAG_f = 1 << 6,
    FLAG_g = 1 << 7,
    FLAG_i = 1 << 8,
    FLAG_l = 1 << 9,
    FLAG_m = 1 << 10,
    FLAG_n = 1 << 11,
    FLAG_o = 1 << 12,
    FLAG_p = 1 << 13,
    FLAG_q = 1 << 14,
    FLAG_r = 1 << 15,
    FLAG_t = 1 << 16,
    FLAG_u = 1 << 17,
};

uint32_t flags;

struct ent {
    char *name;
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

    /* loop over 0400, 0200, 040, 020, etc and print the relevant char */
    char *perms = "rw";
    for (int p = 2; p >= 0; p--) {
        /* the following is a way of doing j = 010 to the power p */
        int j = 01;
        for (int k = 0; k < p; k++)
            j *= 010;
        for (int i = 04; i >= 1; i /= 2) {
            if (i > 1) {
                if (mode & (i * j))
                    printf("%c", perms[2 - i/2]);
                else
                    putchar('-');
            } else {
                /* i == 1 */

                if ((mode & S_ISUID && p == 2)
                        || (mode & S_ISGID && p == 1))
                    if (~mode & j)
                        putchar('S');
                    else
                        putchar('s');
                else if (p == 0 && S_ISDIR(mode) && mode & S_ISVTX)
                    if (~mode & j)
                        putchar('T');
                    else
                        putchar('t');
                else if (mode & j)
                    putchar('x');
                else
                    putchar('-');
            }
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
printname(struct ent *e) {
    if (flags & FLAG_i)
        printf("%lu ", e->ino);
    if (flags & FLAG_l) {
        /* file mode */
        pretty_print_perms(e->mode);

        /* number of links */
        printf("%4lu ", e->nlink);

        /* owner name */
        if (~flags & FLAG_g) {
            errno = 0;
            struct passwd *pwd = getpwuid(e->uid);
            if (pwd == NULL || flags & FLAG_n) {
                if (errno) {
                    fprintf(stderr, "ls: %s: %s\n", e->name, strerror(errno));
                    return 1;
                } else {
                    /* couldn't find a user with that uid; print the numeric value */
                    printf("%-8u ", e->uid);
                }
            } else {
                printf("%-8.8s ", pwd->pw_name);
            }
        }

        /* group name */
        if (~flags & FLAG_o) {
            errno = 0;
            struct group *grp = getgrgid(e->gid);
            if (grp == NULL || flags & FLAG_n) {
                if (errno) {
                    fprintf(stderr, "ls: %s: %s\n", e->name, strerror(errno));
                    return 1;
                } else {
                    /* couldn't find a group with that gid; print the numeric value */
                    printf("%-8u ", e->gid);
                }
            } else {
                printf("%-8.8s ", grp->gr_name);
            }
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
sort(const void *s1, const void *s2) {
    int ret = 0;
    const struct ent *e1 = (struct ent *)s1;
    const struct ent *e2 = (struct ent *)s2;

    if (flags & FLAG_S)
        ret = e2->size - e1->size;
    if (flags & FLAG_t)
        ret = e2->tim.tv_sec - e1->tim.tv_sec;
    if (ret == 0)
        ret = strcmp(e1->name, e2->name);
    if (flags & FLAG_r)
        ret *= -1;
    return ret;
}

/* entries is an array of struct ents, each of which has a name on the heap.
 * free_all frees those names, then frees the array.
 */
void
free_all(struct ent *entries, size_t num_entries) {
    for (size_t i = 0; i < num_entries; i++)
        free(entries[i].name);
    free(entries);
}

int
add_entries(const char *path, struct ent **entr, size_t *nentr) {
    struct stat st;
    if (lstat(path, &st) == -1) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        return 1;
    }

    size_t num_entries = *nentr;
    struct ent *entries = *entr;

    DIR *dirp = NULL;
    struct dirent *dp = NULL;

    if (S_ISDIR(st.st_mode)) {
        /* directory */
        dirp = opendir(path);
        if (dirp == NULL) {
            fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
            return 1;
        }
        int dfd = dirfd(dirp);
        if (dfd == -1) {
            fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
            goto closedir_and_die;
        }

        do {
            errno = 0;
            dp = readdir(dirp);
            if (dp == NULL) {
                if (errno == 0) {
                    /* end of directory */
                    closedir(dirp);
                    goto finished_scan;
                }
                fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
                free_all(entries, num_entries);
                goto closedir_and_die;
            }

            if (~flags & FLAG_a && ~flags & FLAG_A && dp->d_name[0] == '.')
                /* if no -A or -a, skip \.* */
                continue;
            if (flags & FLAG_A && dp->d_name[0] == '.'
                    && (dp->d_name[1] == '\0'
                        || (dp->d_name[1] == '.' && dp->d_name[2] == '\0')))
                /* if -A, skip '.' or '..' */
                continue;

            struct stat stt;
            if (fstatat(dfd, dp->d_name, &stt, AT_SYMLINK_NOFOLLOW) == -1) {
                fprintf(stderr, "ls: %s: %s\n", dp->d_name, strerror(errno));
                free_all(entries, num_entries);
                goto closedir_and_die;
            }

            /* add entry to entries */
            num_entries++;
            (*nentr)++;
            struct ent *tmp = realloc(entries, num_entries * sizeof(struct ent));
            if (tmp == NULL) {
                fprintf(stderr, "ls: realloc: %s\n", strerror(errno));
                free_all(entries, num_entries);
                goto closedir_and_die;
            }
            entries = tmp;
            *entr = entries;
            entries[num_entries - 1].name  = strdup(dp->d_name);
            entries[num_entries - 1].ino   = stt.st_ino;
            entries[num_entries - 1].mode  = stt.st_mode;
            entries[num_entries - 1].nlink = stt.st_nlink;
            entries[num_entries - 1].uid   = stt.st_uid;
            entries[num_entries - 1].gid   = stt.st_gid;
            entries[num_entries - 1].size  = stt.st_size;
            entries[num_entries - 1].rdev  = stt.st_rdev;
            entries[num_entries - 1].tim   = stt.st_mtim;
            if (flags & FLAG_u)
                entries[num_entries - 1].tim = stt.st_atim;
            else if (flags & FLAG_c)
                entries[num_entries - 1].tim = stt.st_ctim;

        } while (dp != NULL);

        if (closedir(dirp) == -1) {
            fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
            free_all(entries, num_entries);
            return 1;
        }
        //free(dp);
    } else {
        /* file */
        num_entries++;
        (*nentr)++;
        struct ent *tmp = realloc(entries, num_entries * sizeof(struct ent));
        if (tmp == NULL) {
            fprintf(stderr, "ls: realloc: %s\n", strerror(errno));
            free_all(entries, num_entries);
            return 1;
        }
        entries = tmp;
        *entr = entries;
        entries[num_entries - 1].name  = strdup(path);
        entries[num_entries - 1].ino   = st.st_ino;
        entries[num_entries - 1].mode  = st.st_mode;
        entries[num_entries - 1].nlink = st.st_nlink;
        entries[num_entries - 1].uid   = st.st_uid;
        entries[num_entries - 1].gid   = st.st_gid;
        entries[num_entries - 1].size  = st.st_size;
        entries[num_entries - 1].rdev  = st.st_rdev;
        entries[num_entries - 1].tim   = st.st_mtim;
        if (flags & FLAG_u)
            entries[num_entries - 1].tim = st.st_atim;
        else if (flags & FLAG_c)
            entries[num_entries - 1].tim = st.st_ctim;
    }

finished_scan:
    return 0;

closedir_and_die:
    if (closedir(dirp) == -1) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        free_all(entries, num_entries);
        return 1;
    }
    //free(dp);

    return 1;
}

int
ls(char **paths, int num) {
    size_t num_entries = 0;
    struct ent *entries = NULL;

    /* This puts all the struct ents into one array, which is then sorted.
     * Solves problem of sorting individual files given as arguments (ls -tr foo.bar foo.baz)
     * but we should not mix these files with files from other directories (ls foo.bar dir/ )
     */
    if (num == 0) {
        if (add_entries(".", &entries, &num_entries) != 0)
            return 1;
    } else {
        for (int i = 0; i < num; i++) {
            if (add_entries(paths[i], &entries, &num_entries) != 0)
                return 1;
        }
    }

    if (~flags & FLAG_f)
        /* is this wrong if num > 1, because argument order dictates order in entries?
         * but POSIX does say that -f is only about directory entries.
         * Best to write args separately, and then directories according to -f etc with a dir: prefix.
         * Like:
         * $ ls -1f foo.txt bar.txt dir/
         * .
         * foo.txt
         * ..
         * bar.txt
         *
         * dir:
         * a.txt
         * .
         * ..
         * b.txt
         *
        */
        qsort(entries, num_entries, sizeof(struct ent), sort);
    for (size_t i = 0; i < num_entries; i++) {
        printname(&entries[i]);
    }

    free_all(entries, num_entries);
    return 0;
}

int
main(int argc, char **argv) {
    int c, ret_val;
    flags = ret_val = 0;

    while ((c = getopt(argc, argv, "1AFSacfgilmnopqrtu")) != -1) {
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
            case 'S':
                flags |= FLAG_S;
                flags &= ~FLAG_t;
                break;
            case 'a':
                flags |= FLAG_a;
                break;
            case 'c':
                flags |= FLAG_c;
                flags &= ~FLAG_u;
                break;
            case 'f':
                flags |= FLAG_f;
                break;
            case 'g':
                flags |= FLAG_g | FLAG_l | FLAG_1;
                flags &= ~FLAG_m;
                break;
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
            case 'n':
                flags |= FLAG_n | FLAG_l | FLAG_1;
                flags &= ~FLAG_m;
                break;
            case 'o':
                flags |= FLAG_o | FLAG_l | FLAG_1;
                flags &= ~FLAG_m;
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
            case 'r':
                flags |= FLAG_r;
                break;
            case 't':
                flags |= FLAG_t;
                flags &= ~FLAG_S;
                break;
            case '?':
                fprintf(stderr, "usage: %s [-1AFSacfgilmnopqrtu]\n", argv[0]);
                return 1;
        }
    }
    argv += optind;
    argc -= optind;

    if(!isatty(1))
        flags |= FLAG_1;

    if (ls(argv, argc) != 0)
        ret_val = 1;

    if (!(flags & FLAG_1))
        puts(""); /* final newline */

    return ret_val;
}
