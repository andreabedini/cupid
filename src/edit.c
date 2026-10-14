#include "prelude.h"
#include "errors.h"
#include "opts.h"
#include "machdep.h"

/* Expand one word of the editor template, from *hep up to the next space,
 * into a malloc'd string, and leave *hep on that space (or the NUL).
 *
 * The template is split into words before this runs, so a name that
 * contains spaces or shell punctuation lands in one argv element and needs
 * no quoting: `%s' is the whole word it expands to, whatever is in it.
 * The word is measured on the first pass and written on the second.
 */
static char *expandWord(char **hep, char *nm, char *fullNm, char *lineStr,
                        bool *expandedName) {
    char *out = NULL;
    char *o = NULL;
    size_t need = 0;
    int pass;

    for (pass = 0; pass < 2; pass++) {
        char *he = *hep;
        o = out;
        need = 0;
        while (*he && !isspace((unsigned char)*he)) {
            if (*he != '%') {
                if (pass)
                    *o++ = *he;
                else
                    need++;
                he++;
                continue;
            }
            he++;
            if (*he == 's' || *he == 'f') {
                char *src = (*he == 's') ? nm : fullNm;
                if (pass) {
                    strcpy(o, src);
                    o += strlen(src);
                } else
                    need += strlen(src);
                *expandedName = true;
                he++;
            } else if (*he == 'd') {
                if (pass) {
                    strcpy(o, lineStr);
                    o += strlen(lineStr);
                } else
                    need += strlen(lineStr);
                he++;
            } else if (*he == '%') { /* %% is one % */
                if (pass)
                    *o++ = '%';
                else
                    need++;
                he++;
            } else if (*he == '\0') {
                /* trailing '%' with nothing after it: keep it literally, and
                 * don't step past the terminating NUL. */
                if (pass)
                    *o++ = '%';
                else
                    need++;
            } else {
                if (pass) {
                    *o++ = '%';
                    *o++ = *he;
                } else
                    need += 2;
                he++;
            }
        }
        if (pass) {
            *o = '\0';
            *hep = he;
        } else if ((out = (char *)malloc(need + 1)) == NULL) {
            errHead(0);
            errPrintf("Unable to start editor");
            errFail();
        }
    }
    return out;
}

/* Start editor on file name at given line. Both name and line or just line may
 * be zero */
bool startEdit(int line, char *nm) {
    Argv cmd;
    char *fullNm;
    char *he;
    bool expandedName = false;
    bool syncEdit = true;
    char lineStr[2 + 3 * sizeof(int)];

    /* First off, check whether we have actually got a plausible editor
     * available. On a Mac, files have creator information, telling which
     * program to launch to, so an editor named to the empty string ""
     * is often desirable.
     */
    if (!cupidEdit || *cupidEdit == '\0') {
        errHead(0);
        errPrintf("Cupid is not configured to use an editor");
        errFail();
    }

    /* More sanity checks */
    if (nm == NULL) {
        return false;
    }

    fullNm = RealPath(nm);
    snprintf(lineStr, sizeof lineStr, "%d", (int)(line ? line : 1));

    he = cupidEdit;

    /* If the editor command is prefixed with '&', the editor is
     * started up asynchronously (the default is for Cupid to block
     * and wait for the editor to exit).
     */
    while (*he == '&') {
        syncEdit = false;
        he++;
    }

    /* The editor is run directly, so the words of the template are the
     * argv: they are separated on whitespace and on nothing else. */
    argvInit(&cmd);
    while (*he) {
        char *word;
        while (*he && isspace((unsigned char)*he))
            he++;
        if (!*he)
            break;
        word = expandWord(&he, nm, fullNm, lineStr, &expandedName);
        argvWord(&cmd, word);
        free(word);
    }

    if (cmd.used == 0) { /* a template of nothing but spaces */
        argvFree(&cmd);
        errHead(0);
        errPrintf("Cupid is not configured to use an editor");
        errFail();
    }

    if (!expandedName) { /* If file name was not included, add it. */
        argvWord(&cmd, nm);
    }

    if (argvRun(&cmd, syncEdit)) {
        Printf("Warning: Editor terminated abnormally\n");
        argvFree(&cmd);
        return false;
    }
    argvFree(&cmd);
    return syncEdit;
}
