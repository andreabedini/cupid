/* --------------------------------------------------------------------------
 * The part of the type checker dealing with predicates and entailment
 * (preds.c), as seen from type.c.
 * ------------------------------------------------------------------------*/

#ifndef PREDS_H
#define PREDS_H

#include "prelude.h"
#include "storage.h"

extern List preds; /* Current predicate assignment         */

extern Cell assumeEvid(Cell, int);
extern List makePredAss(List, int);
extern List copyPreds(List);
extern void qualify(List, Cell);
extern void qualifyBinding(List, Cell);
extern Cell qualifyExpr(int, List, Cell);
extern Cell scEntail(List, Cell, int, int);
extern Cell inEntail(List, Cell, int, int);
extern List scSimplify(List);
extern void elimTauts(void);
extern List elimOuterPreds(List);
extern List elimPredsUsing(List, List);
extern void reducePreds(void);
extern void normPreds(int);
extern bool resolveDefs(List, bool);

#endif /* PREDS_H */
