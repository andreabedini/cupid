/* --------------------------------------------------------------------------
 * Error handling support functions
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/
#ifndef __ERRORS_H__
#define __ERRORS_H__

#include "prelude.h"
#include "storage.h"

extern void internal(char *) CUPID_noreturn;
extern void fatal(char *) CUPID_noreturn;
extern void stopAnyPrinting(void);

/*---------------------------------------------------------------------------
 * Compiler output
 * The interpreter's prompts, messages and errors can be sent to a sink
 * instead of the streams they name.  This has no effect on the output of
 * Haskell programs (which should use hPutStr and friends).
 *-------------------------------------------------------------------------*/

typedef struct OutputSink {
    void (*write)(void *ctx, const char *text, size_t len);
    void (*flush)(void *ctx); /* NULL if there is nothing to flush */
    void *ctx;
} OutputSink;

extern void setOutputSink(const OutputSink *); /* NULL: the streams again */

extern void cupidPrintf(const char *, ...);
extern void cupidPutchar(int);
extern void cupidFlushStdout(void);

extern void cupidFFlush(FILE *);
extern void cupidFPrintf(FILE *, const char *, ...);
extern void cupidPutc(int, FILE *);

#define Printf cupidPrintf
#define Putchar cupidPutchar
#define FlushStdout cupidFlushStdout

#define FFlush cupidFFlush
#define FPrintf cupidFPrintf
#define Putc cupidPutc

/*-------------------------------------------------------------------------*/

#define errorStream stdout /* where error messages go   */

/* Reporting an error: errHead(line) prints the header, errPrintf and the
 * err* functions print the parts of the message, and errFail ends it and
 * returns to the command loop.  (Before this the same sequence was written
 * with the ERRMSG/ETHEN/EEND macros, whose parentheses did not balance
 * within a line.) */
extern void errHead(int); /* ERROR "file":line -  */
extern void errPrintf(const char *, ...) CUPID_printf(1, 2);
extern void errExpr(Cell);
extern void errType(Type);
extern void errContext(List);
extern void errPred(Cell);
extern void errKind(Kind);
extern void errKinds(Kinds);
extern void errFD(Cell);
extern void errFail(void) CUPID_noreturn;
extern void errAbort(void);

extern sigProto(breakHandler);
extern volatile sig_atomic_t broken; /* set by the interrupt handler;   */
                                     /* see the note in prelude.h          */
/* Act on an interrupt, if one came */
CUPID_inline void allowBreak(void) {
    if (broken) {
        broken = false;
        sigRaise(breakHandler);
    }
}

extern bool breakOn(bool); /* in machdep.c         */

#include <setjmp.h>
extern jmp_buf catch_error; /* jump buffer for error trapping  */

/*-------------------------------------------------------------------------*/

#endif /* __ERRORS_H__ */
