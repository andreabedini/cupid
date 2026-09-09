/* --------------------------------------------------------------------------
 * Class, instance and default declarations (class.c), as the rest of the
 * static analysis sees them.  classDefn, instDefn and defaultDefn are
 * declared in connect.h.
 * ------------------------------------------------------------------------*/
#ifndef __CLASS_H__
#define __CLASS_H__

extern Void checkClassDefn(Class);  /* validate a class, first pass          */
extern Void checkClassDefn2_(List); /* second pass over a kinded group       */
extern Int visitClass(Class);       /* check the hierarchy is acyclic        */
extern Void extendFundeps(Class);   /* inherit functional dependencies       */
extern Void addMembers(Class);      /* make the member and selector names    */
extern List classBindings(String, Class, List); /* order member bindings     */
extern Void checkInstDefn(Inst);     /* validate an instance                  */
extern Void insertInst(Inst);        /* add it to its class                   */
extern Name newInstImp(Inst);        /* make the name of its dictionary       */
extern Void checkDefaultDefns(Void); /* validate the default declaration     */

#endif /* __CLASS_H__ */
