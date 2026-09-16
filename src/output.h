/* --------------------------------------------------------------------------
 * Unparse expressions and types - for use in error messages, type checker
 * and for debugging.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * ------------------------------------------------------------------------*/
#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "prelude.h"
#include "storage.h"

extern void printExp(FILE *, Cell);
extern void printType(FILE *, Cell);
extern void printContext(FILE *, List);
extern void printPred(FILE *, Cell);
extern void printKind(FILE *, Kind);
extern void printKinds(FILE *, Kinds);
extern void printFD(FILE *, Pair);

#endif /* __OUTPUT_H__ */
