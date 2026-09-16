/* --------------------------------------------------------------------------
 * Type expressions (typeexp.c), as the rest of the static analysis sees
 * them.  zonkTyvarsIn, oclose, isAmbiguous, calcFunDepsPreds and ambigError
 * are declared in connect.h because type.c uses them too.
 * ------------------------------------------------------------------------*/
#ifndef __TYPEEXP_H__
#define __TYPEEXP_H__

#include "prelude.h"
#include "storage.h"

extern List tcDeps; /* tycons and classes the declaration being checked uses */

extern List typeVarsIn(Cell, List, List, List); /* tyvars of an expression  */
extern Type checkSigType(int, char *, Cell, Type); /* check a signature     */
extern void checkOptQuantVars(int, List, List); /* forall vars are all used */
extern Type depTopType(int, List, Type);  /* resolve a type, rank-2 allowed */
extern Type depCompType(int, List, Type); /* resolve a constructor argument */
extern Type depTypeExp(int, List, Type);  /* resolve a monotype             */
extern Type depTypeVar(int, List, Text);  /* resolve one type variable      */
extern Cell depPredExp(int, List, Cell);  /* resolve a predicate            */
extern List checkQuantVars(int, List, List, Cell); /* check a forall        */
extern List offsetTyvarsIn(Type, List); /* offsets of the tyvars in a type  */
extern List otvars(Cell, List);         /* likewise for a predicate         */
extern List odiff(List, List);          /* difference of offset lists       */
extern bool osubset(List, List);        /* subset test on offset lists      */
extern List calcFunDeps(List);          /* fundeps of a context             */
extern void resetTypeexp(void);   /* clear tcDeps                          */
extern void installTypeexp(void); /* register roots and reset; INSTALL only*/
extern void typeexpControl(int);  /* RESET/INSTALL, called by staticAnalysis */

#endif /* __TYPEEXP_H__ */
