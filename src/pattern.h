/* --------------------------------------------------------------------------
 * Patterns (pattern.c), as the rest of the static analysis sees them.
 * ------------------------------------------------------------------------*/
#ifndef __PATTERN_H__
#define __PATTERN_H__

#include "prelude.h"
#include "storage.h"

extern List patVars; /* variables bound by the pattern being checked       */

extern Cell checkPat(int, Cell); /* turn a parsed expression into a pattern */
extern Cell checkPatType(int, char *, Cell, Type); /* a pattern signature    */
extern Cell applyBtyvs(Cell); /* record the tyvars a pattern binds        */
extern Name conDefined(int, Cell, bool); /* the constructor a name refers to */
extern void patternControl(int); /* RESET/INSTALL, called by staticAnalysis  */

#endif /* __PATTERN_H__ */
