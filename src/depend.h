/* --------------------------------------------------------------------------
 * Dependency analysis (depend.c), as the rest of the static analysis sees
 * it.  depExpr is declared in connect.h because module.c uses it too.
 * ------------------------------------------------------------------------*/
#ifndef __DEPEND_H__
#define __DEPEND_H__

extern Void clearScope(Void);    /* start with no bindings in scope          */
extern Void withinScope(List);   /* enter the scope of a binding group       */
extern Void leaveScope(Void);    /* and leave it                             */
extern List topDependAnal(List); /* order a top-level binding group          */
extern Void depDefaults(Class);  /* analyse the default methods of a class   */
extern Void depInsts(Inst);      /* analyse the methods of an instance       */
extern Void depConFlds(Int, Cell, Bool); /* analyse a record construction    */
extern Cell depQVar(Int, Cell, Bool);    /* resolve a qualified variable     */
extern Void checkNameAmbigName(Int, Name, Bool); /* error if imported twice  */
extern List tcscc(List, List);    /* order tycons and classes by dependency   */
extern Syntax lookupSyntax(Text); /* fixity from the enclosing bindings      */
extern Void dependControl(Int);   /* RESET/INSTALL, called by staticAnalysis  */

#endif /* __DEPEND_H__ */
