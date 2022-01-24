#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int
main(void) {
    char *tty = ttyname(0);
    if (tty == NULL) {
        if (errno != ENOTTY) {
            perror("tty");
            return 2;
        }
        printf("not a tty\n");
        return 1;
    }
    printf("%s\n", tty);
    return 0;
}
