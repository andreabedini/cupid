#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "opts.h"
#include "strutil.h"
#include "machdep.h"

/* Expand one word of the editor template, from *hep up to the next space,
 * into a malloc'd string, and leave *hep on that space (or the NUL).
 *
 * The template is split into words before this runs, so a name that
 * contains spaces or shell punctuation lands in one argv element and needs
 * no quoting: `%s' is the whole word it expands to, whatever is in it.
 * The word is measured on the first pass and written on the second.
 */
static String local expandWord(String *hep, String nm, String fullNm,
                               String lineStr, Bool *expandedName) {
    String out = NULL;
    String o = NULL;
    size_t need = 0;
    Int pass;

    for (pass = 0; pass < 2; pass++) {
        String he = *hep;
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
                String src = (*he == 's') ? nm : fullNm;
                if (pass) {
                    strcpy(o, src);
                    o += strlen(src);
                } else
                    need += strlen(src);
                *expandedName = TRUE;
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
        } else if ((out = (String)malloc(need + 1)) == NULL) {
            errHead(0);
            errPrintf("Unable to start editor");
            errFail();
        }
    }
    return out;
}

Bool startEdit(Int line, String nm) /* Start editor on file name at    */
                                    /* given line.  Both name and line */
{                                   /* or just line may be zero        */
    Argv cmd;
    String fullNm;
    String he;
    Bool expandedName = FALSE;
    Bool syncEdit = TRUE;
    char lineStr[2 + 3 * sizeof(Int)];

    /* First off, check whether we have actually got a plausible editor
     * available. On a Mac, files have creator information, telling which
     * program to launch to, so an editor named to the empty string ""
     * is often desirable.
     */
    if (!cuddlesEdit || *cuddlesEdit == '\0') {
        errHead(0);
        errPrintf("Cuddles is not configured to use an editor");
        errFail();
    }

    /* More sanity checks */
    if (nm == NULL) {
        return FALSE;
    }

    fullNm = RealPath(nm);
    sprintf(lineStr, "%d", (int)(line ? line : 1));

    he = cuddlesEdit;

    /* If the editor command is prefixed with '&', the editor is
     * started up asynchronously (the default is for Cuddles to block
     * and wait for the editor to exit).
     */
    while (*he == '&') {
        syncEdit = FALSE;
        he++;
    }

    /* The editor is run directly, so the words of the template are the
     * argv: they are separated on whitespace and on nothing else. */
    argvInit(&cmd);
    while (*he) {
        String word;
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
        errPrintf("Cuddles is not configured to use an editor");
        errFail();
    }

    if (!expandedName) { /* If file name was not included, add it. */
        argvWord(&cmd, nm);
    }

    if (argvRun(&cmd, syncEdit)) {
        Printf("Warning: Editor terminated abnormally\n");
        argvFree(&cmd);
        return FALSE;
    }
    argvFree(&cmd);
    return syncEdit;
}
