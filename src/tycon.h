/* --------------------------------------------------------------------------
 * Type constructor declarations (tycon.c), as the rest of the static
 * analysis sees them.  tyconDefn, setTypeIns, clearTypeIns, fullExpand and
 * hasIOResultType are declared in connect.h.
 * ------------------------------------------------------------------------*/
#ifndef __TYCON_H__
#define __TYCON_H__

extern List cfunSfuns; /* [(Cfun, [SelectorVar])], for Show and Read       */

extern Void checkTyconDefn(Tycon); /* validate a declaration, all now read   */
extern Void checkSynonyms(List);   /* check the synonyms for cycles          */
extern Int userArity(Name);     /* arity of a constructor, strictness aside */
extern Type fullerExpand(Type); /* expand synonyms and newtypes           */
extern Void tyconControl(Int);  /* RESET/INSTALL, called by staticAnalysis */

#endif /* __TYCON_H__ */
