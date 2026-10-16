/* subst FROM TO FILE: copy FILE to standard output with every FROM
 * replaced by TO, as sed -e s/FROM/TO/g does for plain text.  The tests
 * use it as a preprocessor; see tests/run-tests. */
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    char line[4096];
    FILE *f;
    if (argc != 4 || argv[1][0] == '\0' || (f = fopen(argv[3], "r")) == NULL) {
        perror(argc == 4 ? argv[3] : "usage: subst FROM TO FILE");
        return 1;
    }
    size_t n = strlen(argv[1]);
    while (fgets(line, sizeof line, f) != NULL) {
        char *s = line, *hit;
        while ((hit = strstr(s, argv[1])) != NULL) {
            fwrite(s, 1, (size_t)(hit - s), stdout);
            fputs(argv[2], stdout);
            s = hit + n;
        }
        fputs(s, stdout);
    }
    fclose(f);
    return 0;
}
