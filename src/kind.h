/* --------------------------------------------------------------------------
 * Kind inference (kind.c), as the rest of the static analysis sees it.
 * ------------------------------------------------------------------------*/
#ifndef __KIND_H__
#define __KIND_H__

#include "prelude.h"
#include "storage.h"

extern List unkindTypes; /* types still waiting for a kind annotation   */

extern void kindTCGroup(List);             /* kind a mutually recursive group */
extern void kindType(int, char *, Type);   /* check a type is well-kinded     */
extern void kindPred(int, int, int, Cell); /* likewise for a predicate      */
extern void fixKinds(void); /* write the inferred kinds into unkindTypes  */
extern void kindInst(Inst, int); /* check the predicates of an instance     */
extern void stripBinderKinds(int, Text, Cell); /* (a :: k) binders -> a   */
extern void noUnclaimedKinds(void); /* every kind signature met its decl    */
extern void resetKind(void);   /* clear unkindTypes                          */
extern void installKind(void); /* register roots and reset; INSTALL only    */

#endif /* __KIND_H__ */
