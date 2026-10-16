/* say ARG...: print the arguments, separated by spaces, as echo does.
 * The tests use it as an editor, which shows the command line that Cupid
 * built; see tests/run-tests. */
#include <stdio.h>

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++)
        printf(i > 1 ? " %s" : "%s", argv[i]);
    putchar('\n');
    return 0;
}
