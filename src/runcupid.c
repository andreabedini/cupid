/* --------------------------------------------------------------------------
 * Run a Haskell program, and, built as fficupid, generate the C stubs for
 * a module's foreign imports.
 *
 * This is the interpreter without the prompt: it starts the same evaluator
 * cupid starts, loads the Prelude and the module it is given, and runs
 * main.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "evaluator.h"
#include "machdep.h"
#include "opts.h"
#include "script.h"

/* The name in the error messages is fixed: fficupid is this program built
 * with FFI_COMPILER, and its errors have always come out as runcupid'.  */
static const char *progName = "runcupid";

/* The interpreter's own output is collected here rather than printed, so
 * that an error can be shown once, under a heading, on the way out; a run
 * that succeeds shows none of it.  The buffer grows as needed, so a long
 * message arrives whole. */
static struct {
    char *text;
    size_t len, cap;
} captured;

static void captureWrite(void *ctx, const char *text, size_t len) {
    (void)ctx;
    if (captured.len + len + 1 > captured.cap) {
        size_t cap = captured.cap ? captured.cap : 1024;
        char *grown;
        while (captured.len + len + 1 > cap)
            cap *= 2;
        if (!(grown = realloc(captured.text, cap)))
            return; /* keep what fits; the error heading still shows */
        captured.text = grown;
        captured.cap = cap;
    }
    memcpy(captured.text + captured.len, text, len);
    captured.len += len;
    captured.text[captured.len] = '\0';
}

static const OutputSink captureSink = {captureWrite, NULL, NULL};

static void reportError(void) {
    normalTerminal();
    fprintf(stderr, "%s: Error occurred\n", progName);
    fprintf(stderr, "%s\n", captured.len > 0 ? captured.text : "");
    fprintf(stderr, "\n");
    fflush(stderr);
}

static void usageError(const char *message, char *arg) {
    fprintf(stderr, "%s: %s%s\n", progName, message, arg ? arg : "");
    fflush(stderr);
    exit(1);
}

int main(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    int exitCode = 0;
    int i;

    CStackBase = &argc; /* Save stack base for use in gc   */

    if (!initSystem())
        usageError("failed to initialize, exiting", 0);
    installSignalHandlers();

#if defined(FFI_COMPILER)
    generateFFI = true;
#endif

    if (setjmp(catch_error)) { /* every interpreter error lands here */
        reportError();
        exit(1);
    }

    startEvaluator();
    readOptionSettings();

    /* The options come before the module name; everything after it belongs
     * to the program, or, for fficupid, to the C compiler.            */
    for (i = 1; i < argc && argv[i] && (argv[i][0] == '+' || argv[i][0] == '-');
         ++i)
        if (!processOption(argv[i]))
            usageError("unrecognised option ", argv[i]);
    argv += i;
    argc -= i;

    if (argc < 1)
        usageError("missing file argument", 0);

    setOutputSink(&captureSink);
    if (!loadPrelude()) {
        reportError();
        exit(1);
    }

#if defined(FFI_COMPILER)
    for (i = 1; i < argc; ++i) /* the C compiler's flags */
        ffiSetFlags(argv[i]);
#endif

    addScriptName(argv[0], true);
    readScripts(0);
    resetSubsystems();

#if !defined(FFI_COMPILER)
    {
        Module main = findModule(findText("Main"));
        if (isNull(main))
            usageError("can't find \"Main\" module", 0);
        setCupidArgs(argc, argv); /* what getArgs returns */
        exitCode = runProgram(main);
    }
#endif

    clearStack();
    stopAnyPrinting();
    exitSubsystems();
    stopEvaluator();

    exit(exitCode);
    return 0; /*NOTUSED*/
}
