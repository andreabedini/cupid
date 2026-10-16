/* fake-editor ARG...: print EDITOR and the arguments, separated by spaces.
 * tests/edit-template sets it as the editor; see tests/run-tests. */
#include <stdio.h>

int main(int argc, char **argv) {
    fputs("EDITOR", stdout);
    for (int i = 1; i < argc; i++)
        printf(" %s", argv[i]);
    putchar('\n');
    return 0;
}
