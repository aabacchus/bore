#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int s_flag;
char prompt;

/* buf is a pointer to the bit we're working on at the moment, 
 * while buf_start stores the beginning of our data.
 */
char *buf, *buf_start;
int buf_size = BUFSIZ;
int buf_used = 0;

struct line {
    char s[1];
    int len;
    struct line *prev, *next;
};

struct line *first;
int num_lines = 0;
int cur_line = 0;

void
print_byte_counts(size_t n) {
    if (!s_flag)
        printf("%zu\n", n);
}

struct line *
find_line(int num) {
    struct line *l;
    if (num < 1 || num > num_lines + 1)
        return NULL;
    l = first;
    while (num--)
        l = l->next;
    return l;
}

/* make a new line before the numth line */
struct line *
insert_line_before(char *s, int len, int num) {
    struct line *old, *new;
    new = malloc(sizeof(struct line) + len);
    if (new == NULL) {
        fprintf(stderr, "ed: malloc: %s\n", strerror(errno));
        return NULL;
    }
    old = find_line(num);

    memcpy(new->s, s, len);
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
    int fd, bytes = 0, lineno = 0;
    ssize_t n = 0;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "ed: %s: %s\n", path, strerror(errno));
        return -1;
    }
    do {
        char *eol = memchr(buf, '\n', buf_used);
        if (eol) {
            lineno++;
            int offset = eol - buf + 1;
            insert_line_before(buf, offset, lineno);
            cur_line = 1;
            buf += offset;
            buf_used -= offset;
            bytes += offset;
            continue;
        }
        if (buf_used >= buf_size) {
            int size_new = buf_size * 3 / 2;
            char *b_new = realloc(buf_start, size_new);
            buf_start = b_new;
            buf = buf_start + buf_used;
            buf_size = size_new;
        }
        n = read(fd, buf, buf_size - buf_used);
        buf_used += n;
    } while (n > 0);
    if (n == -1) {
        fprintf(stderr, "ed: %s: %s\n", path, strerror(errno));
        return -1;
    }
    close(fd);
    print_byte_counts(bytes);
    return 0;
}

int
write_buf(char *path) {
    int fd;
    ssize_t n, total = 0;

    fd = open(path, O_WRONLY);
    if (fd == -1) {
        fprintf(stderr, "ed: %s: %s\n", path, strerror(errno));
        return -1;
    }
    struct line *l = first->next;
    for (; l != first; l = l->next) {
        n = write(fd, l->s, l->len);
        if (n == -1) {
            fprintf(stderr, "ed: %s: %s\n", path, strerror(errno));
            return -1;
        }
        total += n;
    }
    printf("%ld\n", total);
    close(fd);
    return 0;
}


void
input(int lineno) {
    ssize_t n;
    char *tmp = malloc(BUFSIZ);
    if (tmp == NULL) {
        fprintf(stderr, "ed: malloc: %s\n", strerror(errno));
        return;
    }
    size_t len = BUFSIZ;
    while (1) {
        n = getline(&tmp, &len, stdin);
        /* getline returns bytes + 1 for \n delimiter */
        if (n == 0)
            return;
        if (n < 0) {
            fprintf(stderr, "ed: %s\n", strerror(errno));
            return;
        }
        if (tmp[0] == '.' && tmp[1] == '\n' && tmp[2] == '\0')
            return;
        fprintf(stderr, "got %ld bytes: '%s'\n", n, tmp);
        insert_line_before(tmp, n, lineno);
    }
    free(tmp);
}

int
ed(char *startfile) {
    if (startfile) 
        if (read_buf(startfile) != 0)
            return 1;
    while (1) {

        if (prompt)
            printf("%c ", prompt);
        fflush(stdout);
        char *c = malloc(BUFSIZ);
        if (c == NULL) {
            fprintf(stderr, "ed: malloc: %s\n", strerror(errno));
            return 1;
        }
        size_t c_len = BUFSIZ;
        getline(&c, &c_len, stdin);
        if (isdigit(*c)) {
            char new_c = *c - '0';
            if (new_c <= num_lines)
                cur_line = new_c;
            else {
                printf("?\n");
                continue;
            }
//            c++;
        }
        else switch (*c) {
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
                input(cur_line);
                break;
            case 'a':
                input(cur_line+1);
                break;
            case 'p':
                printf("%s", find_line(cur_line)->s);
                break;
            case 'w':
                if (startfile)
                    write_buf(startfile);
                else
                    printf("? writing to a new filename not yet implemented\n");
                break;
            case 'q':
                return 0;
            case '\n':
                break;
            default:
                printf("?\n");
                break;
        }
        free(c);
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
    s_flag = 0;
    prompt = 0;
    int c;

    buf_start = malloc(buf_size);
    if (buf_start == NULL) {
        fprintf(stderr, "ed: malloc: %s\n", strerror(errno));
        return 1;
    }
    buf = buf_start;
    first = malloc(sizeof(first));
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

    ed(startfile);
    free(buf_start);
    struct line *x = first;
    while (x) {
        x = x->next;
        free(x);
    }
    return 0;
}
