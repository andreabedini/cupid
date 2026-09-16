/*
 * The Cuddles evaluator / command interpreter + support functions.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 */
#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

#include "prelude.h"
#include "storage.h"

extern void startEvaluator(void);
extern void stopEvaluator(void);
extern void evaluator(Module);
extern int runProgram(Module);

/* Call every subsystem's hook for the phase named, in the fixed order
 * evaluator.c's subsystems table lists them. */
extern void installSubsystems(void); /* once, before the Prelude is read */
extern void resetSubsystems(void);   /* before each script load/evaluation */
extern void markSubsystems(void);    /* garbage collection: mark structures */
extern void exitSubsystems(void);    /* before exit()                      */
extern void breakSubsystems(void);   /* after an interrupt                 */

extern void loadPrelude(void);

#endif /* __EVALUATOR_H__ */
