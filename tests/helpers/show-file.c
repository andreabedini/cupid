/* show-file FILE: copy FILE to standard output, as cat does.  The tests
 * use it as a preprocessor that changes nothing; see tests/run-tests. */
#include <stdio.h>

int main(int argc, char **argv) {
    FILE *f;
    int c;
    if (argc != 2 || (f = fopen(argv[1], "r")) == NULL) {
        perror(argc == 2 ? argv[1] : "usage: show-file FILE");
        return 1;
    }
    while ((c = getc(f)) != EOF)
        putchar(c);
    fclose(f);
    return 0;
}
