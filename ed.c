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

void
print_byte_counts(size_t n) {
    if (!s_flag)
        printf("%zu\n", n);
}

int
read_buf(char *path) {
    int fd;
    ssize_t n = 0;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "ed: %s: %s\n", path, strerror(errno));
        return -1;
    }
    do {
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
    print_byte_counts(buf_used);
    return 0;
}

int
write_buf(char *path) {
    int fd;
    ssize_t n;

    fd = open(path, O_WRONLY);
    if (fd == -1) {
        fprintf(stderr, "ed: %s: %s\n", path, strerror(errno));
        return -1;
    }
    n = write(fd, buf_start, buf_used);
    if (n == -1) {
        fprintf(stderr, "ed: %s: %s\n", path, strerror(errno));
        return -1;
    }
    printf("%ld\n", n);
    close(fd);
    return 0;
}


/* set buf appropriately so that input() reads into the right bit. */
void
input(void) {
    ssize_t n;
    char *tmp = malloc(BUFSIZ);
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
        memcpy(buf, tmp, n);
        buf += n;
        buf_used += n;
    }
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
        int c = fgetc(stdin);
        switch (c) {
            case 'i':
                buf = buf_start;
                input();
                break;
            case 'a':
                buf += buf_used;
                input();
                break;
            case 'p':
                printf("%s", buf_start);
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
    buf = buf_start;
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
    return 0;
}
