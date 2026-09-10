/* --------------------------------------------------------------------------
 * Kind inference (kind.c), as the rest of the static analysis sees it.
 * ------------------------------------------------------------------------*/
#ifndef __KIND_H__
#define __KIND_H__

#include "prelude.h"
#include "storage.h"

extern List unkindTypes; /* types still waiting for a kind annotation   */

extern Void kindTCGroup(List);             /* kind a mutually recursive group */
extern Void kindType(Int, String, Type);   /* check a type is well-kinded     */
extern Void kindPred(Int, Int, Int, Cell); /* likewise for a predicate      */
extern Void fixKinds(Void); /* write the inferred kinds into unkindTypes  */
extern Void kindInst(Inst, Int); /* check the predicates of an instance     */
extern Void kindControl(Int); /* RESET/INSTALL, called by staticAnalysis    */

#endif /* __KIND_H__ */
