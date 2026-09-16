/* --------------------------------------------------------------------------
 * Type constructor declarations (tycon.c), as the rest of the static
 * analysis sees them.  tyconDefn, setTypeIns, clearTypeIns, fullExpand and
 * hasIOResultType are declared in connect.h.
 * ------------------------------------------------------------------------*/
#ifndef __TYCON_H__
#define __TYCON_H__

#include "prelude.h"
#include "storage.h"

extern List cfunSfuns; /* [(Cfun, [SelectorVar])], for Show and Read       */

extern void checkTyconDefn(Tycon); /* validate a declaration, all now read   */
extern void checkSynonyms(List);   /* check the synonyms for cycles          */
extern int userArity(Name);     /* arity of a constructor, strictness aside */
extern Type fullerExpand(Type); /* expand synonyms and newtypes           */
extern void tyconControl(int);  /* RESET/INSTALL, called by staticAnalysis */

#endif /* __TYCON_H__ */
