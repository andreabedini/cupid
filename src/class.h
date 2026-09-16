/* --------------------------------------------------------------------------
 * Class, instance and default declarations (class.c), as the rest of the
 * static analysis sees them.  classDefn, instDefn and defaultDefn are
 * declared in connect.h.
 * ------------------------------------------------------------------------*/
#ifndef __CLASS_H__
#define __CLASS_H__

#include "prelude.h"
#include "storage.h"

extern void checkClassDefn(Class);  /* validate a class, first pass          */
extern void checkClassDefn2_(List); /* second pass over a kinded group       */
extern int visitClass(Class);       /* check the hierarchy is acyclic        */
extern void extendFundeps(Class);   /* inherit functional dependencies       */
extern void addMembers(Class);      /* make the member and selector names    */
extern List classBindings(char *, Class, List); /* order member bindings     */
extern void checkInstDefn(Inst);     /* validate an instance                  */
extern void insertInst(Inst);        /* add it to its class                   */
extern Name newInstImp(Inst);        /* make the name of its dictionary       */
extern void checkDefaultDefns(void); /* validate the default declaration     */

#endif /* __CLASS_H__ */
