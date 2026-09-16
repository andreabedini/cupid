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

/* Handle fatal error */
void fatal(char *msg) {
    FlushStdout();
    Printf("\nFATAL ERROR: %s\n", msg);
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
 * We can redirect compiler output (prompts, error messages, etc) by
 * tweaking these functions.
 * ------------------------------------------------------------------------*/

static bool disableOutput = false; /* redirect output to buffer?      */

#include <stdarg.h>

/* ----------------------------------------------------------------------- */

#define BufferSize 10000 /* size of redirected output buffer  */

typedef struct _CuddlesStream {
    char buffer[BufferSize]; /* buffer for redirected output      */
    int next;                /* next space in buffer              */
} CuddlesStream;

static void vBufferedPrintf(CuddlesStream *, const char *, va_list);
static void bufferedPutchar(CuddlesStream *, int);
static char *bufferClear(CuddlesStream *stream);

static void vBufferedPrintf(CuddlesStream *stream, const char *fmt,
                            va_list ap) {
    int spaceLeft = BufferSize - stream->next;
    char *p = &stream->buffer[stream->next];
    int charsAdded = vsnprintf(p, spaceLeft, fmt, ap);
    if (0 <= charsAdded && charsAdded < spaceLeft)
        stream->next += charsAdded;
    else
        stream->next = 0;
}

static void bufferedPutchar(CuddlesStream *stream, int c) {
    if (BufferSize - stream->next >= 2) {
        stream->buffer[stream->next++] = (char)c;
        stream->buffer[stream->next] = '\0';
    }
}

static char *bufferClear(CuddlesStream *stream) {
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

void cuddlesEnableOutput(bool f) {
    disableOutput = !f;
}

char *cuddlesClearOutputBuffer(void) {
    return bufferClear(&outputStream);
}

void cuddlesPrintf(const char *fmt, ...) {
    va_list ap;        /* pointer into argument list           */
    va_start(ap, fmt); /* make ap point to first arg after fmt */
    if (!disableOutput) {
        vprintf(fmt, ap);
    } else {
        vBufferedPrintf(&outputStream, fmt, ap);
    }
    va_end(ap); /* clean up                             */
}

void cuddlesPutchar(int c) {
    if (!disableOutput) {
        putchar(c);
    } else {
        bufferedPutchar(&outputStream, c);
    }
}

void cuddlesFlushStdout(void) {
    if (!disableOutput) {
        fflush(stdout);
    }
}

void cuddlesFFlush(FILE *fp) {
    if (!disableOutput) {
        fflush(fp);
    }
}

static void cuddlesVFPrintf(FILE *fp, const char *fmt, va_list ap) {
    if (!disableOutput) {
        vfprintf(fp, fmt, ap);
    } else {
        vBufferedPrintf(&outputStream, fmt, ap);
    }
}

void cuddlesFPrintf(FILE *fp, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    cuddlesVFPrintf(fp, fmt, ap);
    va_end(ap);
}

void cuddlesPutc(int c, FILE *fp) {
    if (!disableOutput) {
        fputc(c, fp);
    } else {
        bufferedPutchar(&outputStream, c);
    }
}
