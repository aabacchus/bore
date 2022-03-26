#define _XOPEN_SOURCE 700
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int s_flag;
char prompt;

struct line {
    int len;
    struct line *prev, *next;
    char *s;
};

struct line *first;
int num_lines = 0;
int cur_line = 1;

void
print_byte_counts(size_t n) {
    if (!s_flag)
        printf("%zu\n", n);
}

struct line *
find_line(int num) {
    struct line *l;
    if (num < 1 || num > num_lines + 1) {
        fprintf(stderr, "bad line number\n");
        return NULL;
    }
    l = first;
    while (num--)
        l = l->next;
    return l;
}

/* make a new line before the numth line */
struct line *
insert_line_before(char *s, int len, int num) {
    struct line *old, *new;
    old = find_line(num);
    if (old == NULL)
        return NULL;

    new = malloc(sizeof *new);
    if (new == NULL) {
        fprintf(stderr, "ed: malloc: %s\n", strerror(errno));
        return NULL;
    }

    new->s = malloc(len + 1);
    if (new->s == NULL) {
        free(new);
        fprintf(stderr, "ed: malloc: %s\n", strerror(errno));
        return NULL;
    }
    memcpy(new->s, s, len);
    new->s[len] = '\0';

    new->len = len;
    new->prev = old->prev;
    new->next = old;
    new->prev->next = new;
    old->prev = new;
    num_lines++;
    cur_line = num;
    return new;
}

int
read_buf(char *path) {
    FILE *fds = fopen(path, "r");
    if (fds == NULL) {
        fprintf(stderr, "ed: fopen %s: %s\n", path, strerror(errno));
        return 1;
    }

    int c, linelen, lineno;
    char buf[LINE_MAX] = {0};
    char *bp = buf;
    linelen = 0;
    lineno = 1;
    size_t bytes = 0;
    while ((c = fgetc(fds)) != EOF) {
        linelen++;
        if (linelen > LINE_MAX) {
            fprintf(stderr, "ed: %s line %d: maximum line length exceeded\n", path, lineno);
            return 1;
        }
        *bp++ = c;
        bytes++;
        if (c == '\n') {
            insert_line_before(buf, linelen, lineno);
            lineno++;
            bp = buf;
            linelen = 0;
            continue;
        }
    }
    if (ferror(fds)) {
        fprintf(stderr, "ed: fgetc %s: %s\n", path, strerror(errno));
        return 1;
    }

    if (fclose(fds) != 0) {
        fprintf(stderr, "ed: fclose %s: %s\n", path, strerror(errno));
        return 1;
    }
    print_byte_counts(bytes);
    cur_line = 1;
    return 0;
}

int
write_buf(char *path) {
    int fd;
    ssize_t n, total = 0;

    fd = open(path, O_WRONLY);
    if (fd == -1) {
        fprintf(stderr, "ed: open %s: %s\n", path, strerror(errno));
        return 1;
    }
    struct line *l = first->next;
    for (; l != first; l = l->next) {
        n = write(fd, l->s, l->len);
        if (n == -1) {
            fprintf(stderr, "ed: write %s: %s\n", path, strerror(errno));
            return 1;
        }
        total += n;
    }
    print_byte_counts(total);
    if (close(fd) == -1) {
        fprintf(stderr, "ed: close %s: %s\n", path, strerror(errno));
        return 1;
    }
    return 0;
}


int
input(int lineno) {
    ssize_t n;
    char *tmp = NULL;
    size_t len = 0;
    while (1) {
        n = getline(&tmp, &len, stdin);
        /* getline returns bytes + 1 for \n delimiter */
        if (n == 0)
            break;
        if (n < 0) {
            fprintf(stderr, "ed: getline: %s\n", strerror(errno));
            goto input_fail;
        }
        if (tmp[0] == '.' && tmp[1] == '\n' && tmp[2] == '\0')
            break;
        if (!insert_line_before(tmp, n, lineno))
            goto input_fail;
        lineno++; /* to write following lines afterwards */
    }
    free(tmp);
    return 0;
input_fail:
    free(tmp);
    return 1;
}

int
delete_line(int lineno) {
    struct line *l = find_line(lineno);
    if (l == NULL)
        return 1;
    l->prev->next = l->next;
    l->next->prev = l->prev;
    free(l->s);
    free(l);
    num_lines--;
    return 0;
}

int
ed(char *startfile) {
    int changed = 0;
    if (startfile)
        if (read_buf(startfile) != 0)
            return 1;
    while (1) {
        if (prompt) {
            printf("%c ", prompt);
            if (fflush(stdout) == EOF) {
                fprintf(stderr, "ed: fflush: %s\n", strerror(errno));
                return 1;
            }
        }
        char *c = NULL;
        size_t c_len = 0;
        if (getline(&c, &c_len, stdin) == -1) {
            fprintf(stderr, "ed: getline: %s\n", strerror(errno));
            return 1;
        }
        char *c_initial = c;
        if (isdigit(*c)) {
            char new_c = *c - '0';
            if (0 < new_c && new_c <= num_lines)
                cur_line = new_c;
            else {
                printf("?\n");
                continue;
            }
            c++;
        }
        switch (*c) {
            case '=':
                printf("%d\n", cur_line);
                break;
            case '+':
                cur_line++;
                if (cur_line > num_lines)
                    cur_line = 1;
                break;
            case '-':
                cur_line--;
                if (cur_line <= 0)
                    cur_line = num_lines;
                break;
            case 'i':
                if (input(cur_line) != 0) {
                    printf("?\n");
                    continue;
                }
                changed = 1;
                break;
            case 'a':
                if (input(cur_line+1) != 0) {
                    printf("?\n");
                    continue;
                }
                changed = 1;
                break;
            case 'p':
                printf("%s", find_line(cur_line)->s);
                break;
            case 'd':
                if (delete_line(cur_line) != 0) {
                    printf("?\n");
                    continue;
                }
                changed = 1;
                break;
            case 'c':
                /* delete then insert */
                if (delete_line(cur_line) != 0) {
                    printf("?\n");
                    continue;
                }
                changed = 1;
                if (input(cur_line) != 0) {
                    printf("?\n");
                    continue;
                }
                break;
            case 'w':
                if (startfile) {
                    if (write_buf(startfile) != 0)
                        continue;
                    changed = 0;
                } else
                    printf("? writing to a new filename not yet implemented\n");
                break;
            case 'q':
                if (changed == 0) {
                    free(c_initial);
                    return 0;
                }
                printf("?\n");
                break;
            case 'Q':
                free(c_initial);
                return 0;
            case '\n':
                break;
            default:
                printf("?\n");
                break;
        }
        free(c_initial);
    }
    return 0;
}

void
usage_quit(char *argv0) {
    fprintf(stderr, "usage: %s [-p prompt] [-s] [file]\n", argv0);
    exit(1);
}

int
main(int argc, char **argv) {
    int c, ret;
    s_flag = 0;
    prompt = 0;
    ret = 0;

    first = malloc(sizeof(*first));
    if (first == NULL) {
        fprintf(stderr, "ed: malloc: %s\n", strerror(errno));
        return 1;
    }
    first->next = first;
    first->prev = first;
    while ((c = getopt(argc, argv, "p:s")) != EOF) {
        switch (c) {
            case 's':
                s_flag = 1;
                break;
            case 'p':
                prompt = optarg[0];
                break;
            case '?':
                usage_quit(*argv);
                break;
        }
    }
    if (argc - optind > 1)
        usage_quit(*argv);

    argv += optind;

    char *startfile = NULL;
    if (*argv)
        startfile = *argv;

    ret = ed(startfile);

    struct line *x = first->next;
    do {
        x = x->next;
        free(x->prev->s);
        free(x->prev);
    } while (x != first);

    return ret;
}
