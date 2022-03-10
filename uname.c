#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>

int
main(int argc, char **argv) {
    int c;
    int OPT_m, OPT_n, OPT_r, OPT_s, OPT_v;
    OPT_m = OPT_n = OPT_r = OPT_s = OPT_v = 0;

    if (argc == 1)
        OPT_s = 1;

    while ((c = getopt(argc, argv, ":amnrsv")) != -1) {
        switch (c) {
            case 'a':
                OPT_m = OPT_n = OPT_r = OPT_s = OPT_v = 1;
                break;
            case 'm':
                OPT_m = 1;
                break;
            case 'n':
                OPT_n = 1;
                break;
            case 'r':
                OPT_r = 1;
                break;
            case 's':
                OPT_s = 1;
                break;
            case 'v':
                OPT_v = 1;
                break;
            case '?':
                fprintf(stderr, "usage: %s [-amnrsv]\n", *argv);
                return 1;
        }
    }

    struct utsname ut;
    if (uname(&ut) == -1) {
        perror("uname");
        return 1;
    }

    /* TODO: don't print unnecessary trailing spaces */
    if (OPT_s) printf("%s ", ut.sysname);
    if (OPT_n) printf("%s ", ut.nodename);
    if (OPT_r) printf("%s ", ut.release);
    if (OPT_v) printf("%s ", ut.version);
    if (OPT_m) printf("%s ", ut.machine);
    printf("\n");
    
    return 0;
}
