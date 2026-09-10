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
#include "char.h"
#include "evaluator.h" /* everybody() proto only */
#include <setjmp.h>

static Void local cuddlesVFPrintf(FILE *, const char *, va_list);

jmp_buf catch_error; /* jump buffer for error trapping  */

/* --------------------------------------------------------------------------
 * Error handling:
 * ------------------------------------------------------------------------*/

Void stopAnyPrinting() {  /* terminate printing of expression,*/
    if (printing) {       /* after successful termination or  */
        printing = FALSE; /* runtime error (e.g. interrupt)   */
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

Void errHead(Int l) /* print start of error message     */
{
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

Void errFail() {             /* terminate error message and     */
    Putc('\n', errorStream); /* produce exception to return to  */
    FFlush(errorStream);     /* main command loop               */
    longjmp(catch_error, 1);
}

Void errPrintf(const char *fmt, ...) /* part of an error message           */
{
    va_list ap;
    va_start(ap, fmt);
    cuddlesVFPrintf(errorStream, fmt, ap);
    va_end(ap);
}

Void errExpr(Cell e) {
    printExp(errorStream, e);
}
Void errType(Type t) {
    printType(errorStream, t);
}
Void errContext(List qs) {
    printContext(errorStream, qs);
}
Void errPred(Cell pi) {
    printPred(errorStream, pi);
}
Void errKind(Kind k) {
    printKind(errorStream, k);
}
Void errKinds(Kinds ks) {
    printKinds(errorStream, ks);
}
Void errFD(Cell fd) {
    printFD(errorStream, fd);
}

Void errAbort() {      /* altern. form of error handling  */
    failed();          /* used when suitable error message*/
    stopAnyPrinting(); /* has already been printed        */
    errFail();
}

Void internal(String msg) /* handle internal error           */
{
    failed();
    stopAnyPrinting();
    Printf("INTERNAL ERROR: %s\n", msg);

    Printf("This is a bug in Cuddles; please report it.\n");

    FlushStdout();
    longjmp(catch_error, 1);
}

Void fatal(String msg) /* handle fatal error              */
{
    FlushStdout();
    Printf("\nFATAL ERROR: %s\n", msg);
    everybody(EXIT);
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
sigHandler(breakHandler) { /* respond to break interrupt      */
    Printf("{Interrupted!}\n");
    breakOn(TRUE); /* reinstall break trapping (in case breakOn(FALSE) ran */
                   /* while this break was pending)                        */
    everybody(BREAK);
    failed();
    stopAnyPrinting();
    FlushStdout();
    clearerr(stdin);
    longjmp(catch_error, 1);
    sigResume; /*NOTREACHED*/
}

/* --------------------------------------------------------------------------
 * Compiler output
 * We can redirect compiler output (prompts, error messages, etc) by
 * tweaking these functions.
 * ------------------------------------------------------------------------*/

static Bool disableOutput = FALSE; /* redirect output to buffer?      */

#include <stdarg.h>

/* ----------------------------------------------------------------------- */

#define BufferSize 10000 /* size of redirected output buffer  */

typedef struct _CuddlesStream {
    char buffer[BufferSize]; /* buffer for redirected output      */
    Int next;                /* next space in buffer              */
} CuddlesStream;

static Void local vBufferedPrintf(CuddlesStream *, const char *, va_list);
static Void local bufferedPutchar(CuddlesStream *, int);
static String local bufferClear(CuddlesStream *stream);

static Void local vBufferedPrintf(CuddlesStream *stream, const char *fmt,
                                  va_list ap) {
    Int spaceLeft = BufferSize - stream->next;
    char *p = &stream->buffer[stream->next];
    Int charsAdded = vsnprintf(p, spaceLeft, fmt, ap);
    if (0 <= charsAdded && charsAdded < spaceLeft)
        stream->next += charsAdded;
    else
        stream->next = 0;
}

static Void local bufferedPutchar(CuddlesStream *stream, int c) {
    if (BufferSize - stream->next >= 2) {
        stream->buffer[stream->next++] = (char)c;
        stream->buffer[stream->next] = '\0';
    }
}

static String local bufferClear(CuddlesStream *stream) {
    if (stream->next == 0) {
        return "";
    } else {
        stream->next = 0;
        return stream->buffer;
    }
}

/* ----------------------------------------------------------------------- */

static CuddlesStream outputStream;
/* ADR note:
 * We rely on standard C semantics to initialise outputStream.next to 0.
 */

Void cuddlesEnableOutput(Bool f) {
    disableOutput = !f;
}

String cuddlesClearOutputBuffer() {
    return bufferClear(&outputStream);
}

Void cuddlesPrintf(const char *fmt, ...) {
    va_list ap;        /* pointer into argument list           */
    va_start(ap, fmt); /* make ap point to first arg after fmt */
    if (!disableOutput) {
        vprintf(fmt, ap);
    } else {
        vBufferedPrintf(&outputStream, fmt, ap);
    }
    va_end(ap); /* clean up                             */
}

Void cuddlesPutchar(int c) {
    if (!disableOutput) {
        putchar(c);
    } else {
        bufferedPutchar(&outputStream, c);
    }
}

Void cuddlesFlushStdout() {
    if (!disableOutput) {
        fflush(stdout);
    }
}

Void cuddlesFFlush(FILE *fp) {
    if (!disableOutput) {
        fflush(fp);
    }
}

static Void local cuddlesVFPrintf(FILE *fp, const char *fmt, va_list ap) {
    if (!disableOutput) {
        vfprintf(fp, fmt, ap);
    } else {
        vBufferedPrintf(&outputStream, fmt, ap);
    }
}

Void cuddlesFPrintf(FILE *fp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    cuddlesVFPrintf(fp, fmt, ap);
    va_end(ap);
}

Void cuddlesPutc(int c, FILE *fp) {
    if (!disableOutput) {
        fputc(c, fp);
    } else {
        bufferedPutchar(&outputStream, c);
    }
}
