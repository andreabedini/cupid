/* --------------------------------------------------------------------------
 * Patterns (pattern.c), as the rest of the static analysis sees them.
 * ------------------------------------------------------------------------*/
#ifndef __PATTERN_H__
#define __PATTERN_H__

extern List patVars; /* variables bound by the pattern being checked       */

extern Cell checkPat(Int, Cell); /* turn a parsed expression into a pattern */
extern Cell checkPatType(Int, String, Cell, Type); /* a pattern signature    */
extern Cell applyBtyvs(Cell); /* record the tyvars a pattern binds        */
extern Name conDefined(Int, Cell, Bool); /* the constructor a name refers to */
extern Void patternControl(Int); /* RESET/INSTALL, called by staticAnalysis  */

#endif /* __PATTERN_H__ */
