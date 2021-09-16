#include <stdio.h>

int
main(int argc, char **argv) {
    argv++;
    while (*argv) {
        printf("%s", *argv);
        if (*++argv)
            printf(" ");
    }
    printf("\n");
    return 0;
}
