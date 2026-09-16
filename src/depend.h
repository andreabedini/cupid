/* --------------------------------------------------------------------------
 * Dependency analysis (depend.c), as the rest of the static analysis sees
 * it.  depExpr is declared in connect.h because module.c uses it too.
 * ------------------------------------------------------------------------*/
#ifndef __DEPEND_H__
#define __DEPEND_H__

#include "prelude.h"
#include "storage.h"

extern void clearScope(void);    /* start with no bindings in scope          */
extern void withinScope(List);   /* enter the scope of a binding group       */
extern void leaveScope(void);    /* and leave it                             */
extern List topDependAnal(List); /* order a top-level binding group          */
extern void depDefaults(Class);  /* analyse the default methods of a class   */
extern void depInsts(Inst);      /* analyse the methods of an instance       */
extern void depConFlds(int, Cell, bool); /* analyse a record construction    */
extern Cell depQVar(int, Cell, bool);    /* resolve a qualified variable     */
extern void checkNameAmbigName(int, Name, bool); /* error if imported twice  */
extern List tcscc(List, List);    /* order tycons and classes by dependency   */
extern Syntax lookupSyntax(Text); /* fixity from the enclosing bindings      */
extern void resetDepend(void);    /* clear the scope stacks and dep. lists   */
extern void installDepend(void);  /* register roots and reset; INSTALL only  */
extern void dependControl(int);   /* RESET/INSTALL, called by staticAnalysis  */

#endif /* __DEPEND_H__ */
