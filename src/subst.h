/* --------------------------------------------------------------------------
 * Definitions for substitution data structure and operations.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: subst.h,v $
 * $Revision: 1.11 $
 * $Date: 2004/11/04 00:01:31 $
 * ------------------------------------------------------------------------*/
#ifndef __SUBST_H__
#define __SUBST_H__

/* #define DEBUG_SELS     1 */
/* #define DEBUG_DEPENDS  1 */

typedef struct {			/* Each type variable contains:	   */
    Type bound;				/* A type skeleton (unbound==NIL)  */
    Int  offs;				/* Offset for skeleton		   */
    Kind kind;				/* kind annotation		   */
} Tyvar;

extern  Tyvar		*tyvars;	/* storage for type variables	   */
extern	Int		typeOff;	/* offset of result type 	   */
extern	Type		typeIs;		/* skeleton of result type	   */
CUDDLES_inline Void inferType(Type t, Int o) { typeIs = t; typeOff = o; }
extern  Int		typeFree;	/* freedom in instantiated type	   */
extern	List		predsAre;	/* list of predicates in type	   */
extern  List		genericVars;	/* list of generic vars		   */
extern  List		btyvars;	/* explicitly scoped type vars	   */

CUDDLES_inline Tyvar *tyvar (Int n)    { return tyvars+n; }	/* nth type variable	   */
CUDDLES_inline Int    tyvNum(Tyvar *t) { return (Int)(t-tyvars); }	/* and its inverse	   */
CUDDLES_inline Bool   isBound(Tyvar *t) { return t->bound && t->bound!=SKOLEM; }
#define aVar		mkOffset(0)	/* Simple skeletons for type vars  */
#define bVar		mkOffset(1)
CUDDLES_inline Void enterBtyvs(Void) { btyvars = cons(NIL,btyvars); }
CUDDLES_inline Void leaveBtyvs(Void) { btyvars = tl(btyvars); }


					/* offs values when isNull(bound): */
#define FIXED_TYVAR	0	        /* fixed in current assumption	   */
#define UNUSED_GENERIC	1	        /* not fixed, not yet encountered  */
#define GENERIC		2	        /* GENERIC+n==nth generic var found*/

extern  char		*unifyFails;	/* Unification error message	   */

extern Void  emptySubstitution	(Void);
extern Int   newTyvars		(Int);
CUDDLES_inline Int newKindvars(Int n) { return newTyvars(n); }
extern Int   newKindedVars	(Kind);
extern Void  instantiate	(Type);

extern Pair  findBtyvs		(Text);
extern Void  markBtyvs		(Void);
extern Type  localizeBtyvs	(Type);

extern Tyvar *getTypeVar	(Type,Int);

/* Follow the substitution from (t,o) to the first type that is not a
 * bound variable, updating t and o, and return that variable if it is
 * one, else NULL */
CUDDLES_inline Tyvar *deRef(Type *t, Int *o)
{
    Tyvar *tyv;
    while ((tyv=getTypeVar(*t,*o)) && isBound(tyv)) {
	*t = tyv->bound;
	*o = tyv->offs;
    }
    return tyv;
}
extern Void  tyvarType		(Int);
extern Void  bindTv		(Int,Type,Int);
extern Cell  getDerefHead	(Type,Int);
extern Void  expandSyn		(Tycon, Int, Type *, Int *);

extern Void  clearMarks		(Void);
extern Void  markAllVars	(Void);
extern Void  resetGenerics	(Void);
extern Void  markTyvar		(Int);
extern Void  markType		(Type,Int);
extern Void  markPred		(Cell);

extern Type  copyTyvar		(Int);
extern Type  copyType		(Type,Int);
extern Cell  copyPred		(Cell,Int);
extern Type  dropRank2		(Type,Int,Int);
extern Type  dropRank1		(Type,Int,Int);
extern Void  liftRank2Args	(List,Int,Int);
extern Type  liftRank2		(Type,Int,Int);
extern Type  liftRank1		(Type,Int,Int);
extern Kind  copyKindvar	(Int);
extern Kind  copyKind		(Kind,Int);

extern Bool  eqKind		(Kind,Kind);
extern Kind  getKind		(Cell,Int);

extern List  genvarTyvar	(Int,List);
extern List  genvarType		(Type,Int,List);

extern Bool  doesntOccurIn	(Tyvar*,Type,Int);
extern Bool  unify		(Type,Int,Type,Int);
extern Bool  kunify		(Kind,Int,Kind,Int);

extern Void  typeTuple		(Cell);
extern Kind  simpleKind		(Int);
extern Void  varKind		(Int);

extern Bool  samePred		(Cell,Int,Cell,Int);
extern Bool  matchPred		(Cell,Int,Cell,Int);
extern Bool  unifyPred		(Cell,Int,Cell,Int);
extern Inst  findInstFor	(Cell,Int);

extern Void  improve		(Int,List,List);
extern Void  improve1		(Int,List,Cell,Int);

extern Bool  sameSchemes	(Type,Type);
extern Bool  sameType		(Type,Int,Type,Int);
extern Bool  matchType		(Type,Int,Type,Int);

/*-------------------------------------------------------------------------*/

#endif /* __SUBST_H__ */
