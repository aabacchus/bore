#define _XOPEN_SOURCE 700
#include <stdio.h>

int
main(int argc, char **argv) {
    (void)argc;
    argv++;
    while (*argv) {
        printf("%s", *argv);
        if (*++argv)
            printf(" ");
    }
    printf("\n");
    return 0;
}
