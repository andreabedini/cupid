/* --------------------------------------------------------------------------
 * Error handling support functions
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: errors.h,v $
 * $Revision: 1.13 $
 * $Date: 2005/09/01 14:57:50 $
 * ------------------------------------------------------------------------*/
#ifndef __ERRORS_H__
#define __ERRORS_H__

extern Void internal	    (String) CUDDLES_noreturn;
extern Void fatal	    (String) CUDDLES_noreturn;
extern Void stopAnyPrinting (Void);

/*---------------------------------------------------------------------------
 * Compiler output
 * Tweaking this lets us redirect prompts, error messages, etc - but has no
 * effect on output of Haskell programs (which should use hPutStr and friends).
 *-------------------------------------------------------------------------*/

extern Void   cuddlesPrintf            (const char *, ...);
extern Void   cuddlesPutchar           (int);
extern Void   cuddlesFlushStdout       (Void);
extern Void   cuddlesEnableOutput      (Bool);
extern String cuddlesClearOutputBuffer (Void);

extern Void   cuddlesFFlush    	    (FILE*);
extern Void   cuddlesFPrintf   	    (FILE*, const char*, ...);
extern Void   cuddlesPutc      	    (int, FILE*);

#define Printf         	     cuddlesPrintf
#define Putchar        	     cuddlesPutchar
#define FlushStdout    	     cuddlesFlushStdout
#define EnableOutput   	     cuddlesEnableOutput
#define ClearOutputBuffer    cuddlesClearOutputBuffer

#define FFlush               cuddlesFFlush
#define FPrintf              cuddlesFPrintf
#define Putc                 cuddlesPutc


/*-------------------------------------------------------------------------*/

#define errorStream	 stdout		   /* where error messages go   */

/* Reporting an error: errHead(line) prints the header, errPrintf and the
 * err* functions print the parts of the message, and errFail ends it and
 * returns to the command loop.  (Before this the same sequence was written
 * with the ERRMSG/ETHEN/EEND macros, whose parentheses did not balance
 * within a line.) */
extern Void errHead      (Int);		   /* ERROR "file":line -  */
extern Void errPrintf    (const char *,...) CUDDLES_printf(1,2);
extern Void errExpr      (Cell);
extern Void errType      (Type);
extern Void errContext   (List);
extern Void errPred      (Cell);
extern Void errKind      (Kind);
extern Void errKinds     (Kinds);
extern Void errFD        (Cell);
extern Void errFail	 (Void) CUDDLES_noreturn;
extern Void errAbort	 (Void);

extern sigProto(breakHandler);
extern volatile sig_atomic_t broken;	/* set by the interrupt handler;   */
					/* see the note in prelude.h	   */
CUDDLES_inline Void allowBreak(Void)	/* act on an interrupt, if one came*/
{
    if (broken) {
	broken = FALSE;
	sigRaise(breakHandler);
    }
}

extern Bool breakOn      (Bool);		   /* in machdep.c	   */

#include <setjmp.h>
extern jmp_buf catch_error;          /* jump buffer for error trapping  */

/*-------------------------------------------------------------------------*/

#endif /* __ERRORS_H__ */
