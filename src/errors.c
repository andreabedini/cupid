/* --------------------------------------------------------------------------
 * Error handling support functions
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * ------------------------------------------------------------------------*/
#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "output.h"
#include "opts.h"
#include "goal.h"
#include "evaluator.h" /* installSubsystems() etc. proto only */
#include <setjmp.h>

static void cuddlesVFPrintf(FILE *, const char *, va_list);

jmp_buf catch_error; /* jump buffer for error trapping  */

/* --------------------------------------------------------------------------
 * Error handling:
 * ------------------------------------------------------------------------*/

/* Terminate printing of expression, after successful termination or runtime
 * error (e.g. interrupt) */
void stopAnyPrinting(void) {
    if (printing) {
        printing = false;
        Putchar('\n');
        if (showStats) {
#define plural(v) v, (v == 1 ? "" : "s")
            Printf("(%lu reduction%s, ", plural(numReductions));
            Printf("%lu cell%s", plural(numCells));
            if (numGcs > 0)
                Printf(", %u garbage collection%s", plural(numGcs));
            Printf(")\n");
#undef plural
        }
        FlushStdout();
        garbageCollect();
    }
}

/* Print start of error message */
void errHead(int l) {
    failed(); /* failed to reach target ...       */
    stopAnyPrinting();
    FPrintf(errorStream, "ERROR");

    /*
     * Encapsulating the filename portion inside of d-quotes makes it
     * a tad easier for an Emacs-mode to decipher the location of the error.
     * -- sof 9/01.
     */
    if (scriptFile) {
        setLastEdit(scriptFile, l);
        FPrintf(errorStream, " \"%s\"", scriptFile);
        if (l)
            FPrintf(errorStream, ":%d", l);
        scriptFile = 0;
    }
    FPrintf(errorStream, " - ");
    FFlush(errorStream);
}

/* Terminate error message and produce exception to return to main command loop */
void errFail(void) {
    Putc('\n', errorStream);
    FFlush(errorStream);
    longjmp(catch_error, 1);
}

/* Part of an error message */
void errPrintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    cuddlesVFPrintf(errorStream, fmt, ap);
    va_end(ap);
}

void errExpr(Cell e) {
    printExp(errorStream, e);
}
void errType(Type t) {
    printType(errorStream, t);
}
void errContext(List qs) {
    printContext(errorStream, qs);
}
void errPred(Cell pi) {
    printPred(errorStream, pi);
}
void errKind(Kind k) {
    printKind(errorStream, k);
}
void errKinds(Kinds ks) {
    printKinds(errorStream, ks);
}
void errFD(Cell fd) {
    printFD(errorStream, fd);
}

/* Altern. form of error handling used when suitable error message has already
 * been printed */
void errAbort(void) {
    failed();
    stopAnyPrinting();
    errFail();
}

/* Handle internal error */
void internal(char *msg) {
    failed();
    stopAnyPrinting();
    Printf("INTERNAL ERROR: %s\n", msg);

    Printf("This is a bug in Cuddles; please report it.\n");

    FlushStdout();
    longjmp(catch_error, 1);
}

/* Handle fatal error.  The message goes straight to stderr, not through
 * Printf: a program that has set an output sink is collecting the
 * interpreter's output to show later, and there is no later. */
void fatal(char *msg) {
    FlushStdout();
    fflush(stdout); /* whatever was printed before comes first */
    fprintf(stderr, "\nFATAL ERROR: %s\n", msg);
    fflush(stderr);
    exitSubsystems();
    exit(1);
}

/* --------------------------------------------------------------------------
 * Break interrupt handler:
 * ------------------------------------------------------------------------*/
/* breakHandler is never installed as the OS-level signal handler: it is
 * raised as an ordinary function call from allowBreak(), once the "broken"
 * flag set by the real (async-signal-safe) handler has been noticed at a
 * safe point.  So the Printf/longjmp/etc below run in normal control flow,
 * not signal context, on every platform.
 */
/* Respond to break interrupt */
sigHandler(breakHandler) {
    Printf("{Interrupted!}\n");
    breakOn(true); /* reinstall break trapping (in case breakOn(false) ran */
                   /* while this break was pending)                        */
    breakSubsystems();
    failed();
    stopAnyPrinting();
    FlushStdout();
    clearerr(stdin);
    longjmp(catch_error, 1);
    sigResume; /*NOTREACHED*/
}

/* --------------------------------------------------------------------------
 * Compiler output
 *
 * Prompts, messages and errors go through these functions.  With no sink
 * set they write to the stream they are given; with a sink set, all of
 * them write to the sink instead, whatever stream they name.  This has no
 * effect on the output of Haskell programs, which uses the IO primitives.
 * ------------------------------------------------------------------------*/

#include <stdarg.h>

static const OutputSink *outputSink = NULL; /* NULL: write to the stream */

/* Send the interpreter's output to sink, or back to the streams if sink
 * is NULL.  The sink is used, not copied, so it must outlive its use. */
void setOutputSink(const OutputSink *sink) {
    outputSink = sink;
}

/* Format into a buffer and hand the text to the sink.  Most messages fit
 * the local buffer; a longer one is formatted again into one of its exact
 * size, so nothing is cut short. */
static void sinkVPrintf(const char *fmt, va_list ap) {
    char local[256];
    va_list again;
    int n;

    va_copy(again, ap);
    n = vsnprintf(local, sizeof local, fmt, ap);
    if (n >= 0 && (size_t)n < sizeof local)
        outputSink->write(outputSink->ctx, local, (size_t)n);
    else if (n >= 0) {
        char *whole = malloc((size_t)n + 1);
        if (whole) {
            vsnprintf(whole, (size_t)n + 1, fmt, again);
            outputSink->write(outputSink->ctx, whole, (size_t)n);
            free(whole);
        }
    }
    va_end(again);
}

static void sinkPutc(int c) {
    char ch = (char)c;
    outputSink->write(outputSink->ctx, &ch, 1);
}

static void cuddlesVFPrintf(FILE *fp, const char *fmt, va_list ap) {
    if (outputSink)
        sinkVPrintf(fmt, ap);
    else
        vfprintf(fp, fmt, ap);
}

void cuddlesPrintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    cuddlesVFPrintf(stdout, fmt, ap);
    va_end(ap);
}

void cuddlesPutchar(int c) {
    cuddlesPutc(c, stdout);
}

void cuddlesFlushStdout(void) {
    cuddlesFFlush(stdout);
}

void cuddlesFFlush(FILE *fp) {
    if (!outputSink)
        fflush(fp);
    else if (outputSink->flush)
        outputSink->flush(outputSink->ctx);
}

void cuddlesFPrintf(FILE *fp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    cuddlesVFPrintf(fp, fmt, ap);
    va_end(ap);
}

void cuddlesPutc(int c, FILE *fp) {
    if (outputSink)
        sinkPutc(c);
    else
        fputc(c, fp);
}
