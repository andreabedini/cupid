/* --------------------------------------------------------------------------
 * The part of the type checker dealing with predicates and entailment
 * (preds.c), as seen from type.c.
 * ------------------------------------------------------------------------*/

#ifndef PREDS_H
#define PREDS_H

extern List preds;			/* Current predicate assignment	   */

extern Cell  assumeEvid      (Cell,Int);
extern List  makePredAss     (List,Int);
extern List  copyPreds       (List);
extern Void  qualify         (List,Cell);
extern Void  qualifyBinding  (List,Cell);
extern Cell  qualifyExpr     (Int,List,Cell);
extern Cell  scEntail        (List,Cell,Int,Int);
extern Cell  inEntail        (List,Cell,Int,Int);
extern List  scSimplify      (List);
extern Void  elimTauts       (Void);
extern List  elimOuterPreds  (List);
extern List  elimPredsUsing  (List,List);
extern Void  reducePreds     (Void);
extern Void  normPreds       (Int);
extern Bool  resolveDefs     (List,Bool);

#endif /* PREDS_H */
