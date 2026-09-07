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

#define Hilite()         doNothing()
#define Lolite()         doNothing()
#define errorStream	 stdout

#define ERRMSG(l)	 Hilite(); errHead(l); FPrintf(errorStream,
#define EEND       	 ); Lolite(); errFail()
#define EEND_NORET       ); Lolite()
#define ETHEN		 );
#define ERRTEXT		 Hilite(); FPrintf(errorStream,
#define ERREXPR(e)	 Hilite(); printExp(errorStream,e); Lolite()
#define ERRTYPE(e)	 Hilite(); printType(errorStream,e); Lolite()
#define ERRCONTEXT(qs)   Hilite(); printContext(errorStream,qs); Lolite()
#define ERRPRED(pi)      Hilite(); printPred(errorStream,pi); Lolite()
#define ERRKIND(k)	 Hilite(); printKind(errorStream,k); Lolite()
#define ERRKINDS(ks)	 Hilite(); printKinds(errorStream,ks); Lolite()
#define ERRFD(fd)	 Hilite(); printFD(errorStream,fd); Lolite()

extern Void errHead      (Int);		   /* in main.c		   */
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
