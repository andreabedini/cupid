/* --------------------------------------------------------------------------
 * Definitions for substitution data structure and operations.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/
#ifndef __SUBST_H__
#define __SUBST_H__

#include "prelude.h"
#include "storage.h"

/* #define DEBUG_SELS     1 */
/* #define DEBUG_DEPENDS  1 */

typedef struct { /* Each type variable contains:           */
    Type bound;  /* A type skeleton (unbound==NIL)  */
    int offs;    /* Offset for skeleton            */
    Kind kind;   /* kind annotation                */
} Tyvar;

extern Tyvar *tyvars; /* storage for type variables        */

/* What a check infers: a type skeleton and the offset its variables are
 * read at in tyvars.  instantiate also says how many variables it made
 * from offs and which context it peeled off; every other producer says
 * 0 and NIL. */
typedef struct Typing {
    Type type;  /* the skeleton, or NIL                            */
    int offs;   /* offset of its variables in tyvars               */
    int free;   /* variables instantiate created from offs, else 0 */
    List preds; /* context instantiate peeled off, else NIL        */
} Typing;

/* The monotype (t,o): no fresh variables, no context */
CUDDLES_inline Typing typing(Type t, int o) {
    Typing ty = {t, o, 0, NIL};
    return ty;
}

extern List genericVars; /* list of generic vars                   */
extern List btyvars;     /* explicitly scoped type vars    */

CUDDLES_inline Tyvar *tyvar(int n) {
    return tyvars + n;
} /* nth type variable     */
CUDDLES_inline int tyvNum(Tyvar *t) {
    return (int)(t - tyvars);
} /* and its inverse       */
CUDDLES_inline bool isBound(Tyvar *t) {
    return t->bound && t->bound != SKOLEM;
}
#define aVar mkOffset(0) /* Simple skeletons for type vars  */
#define bVar mkOffset(1)
CUDDLES_inline void enterBtyvs(void) {
    btyvars = cons(NIL, btyvars);
}
CUDDLES_inline void leaveBtyvs(void) {
    btyvars = tl(btyvars);
}

/* offs values when isNull(bound): */
#define FIXED_TYVAR 0    /* fixed in current assumption    */
#define UNUSED_GENERIC 1 /* not fixed, not yet encountered  */
#define GENERIC 2        /* GENERIC+n==nth generic var found*/

extern char *unifyFails; /* Unification error message      */

extern void installSubstitution(void); /* register roots, reset, clear caches */
extern void resetSubstitution(void);   /* clear the current substitution */
extern void markSubstitution(void);    /* mark the tuple/kind caches      */
extern void exitSubstitution(void);    /* free the type-variable table    */
extern void emptySubstitution(void);
extern int newTyvars(int);
CUDDLES_inline int newKindvars(int n) {
    return newTyvars(n);
}
extern int newKindedVars(Kind);
extern Typing instantiate(Type, int);

extern Pair findBtyvs(Text);
extern void markBtyvs(void);
extern Type localizeBtyvs(Type);

extern Tyvar *getTypeVar(Type, int);

/* Follow the substitution from (t,o) to the first type that is not a
 * bound variable, updating t and o, and return that variable if it is
 * one, else NULL */
CUDDLES_inline Tyvar *deRef(Type *t, int *o) {
    Tyvar *tyv;
    while ((tyv = getTypeVar(*t, *o)) && isBound(tyv)) {
        *t = tyv->bound;
        *o = tyv->offs;
    }
    return tyv;
}
extern Typing tyvarType(int);
extern void bindTv(int, Type, int);
extern Cell getDerefHead(Type, int);
extern void expandSyn(Tycon, int, Type *, int *);

extern void clearMarks(void);
extern void markAllVars(void);
extern void resetGenerics(void);
extern void markTyvar(int);
extern void markType(Type, int);
extern void markPred(Cell);

extern Type copyTyvar(int);
extern Type copyType(Type, int);
extern Cell copyPred(Cell, int);
extern Type dropRank2(Type, int, int);
extern Type dropRank1(Type, int, int);
extern void liftRank2Args(List, int, int);
extern Type liftRank2(Type, int, int);
extern Type liftRank1(Type, int, int);
extern Kind copyKindvar(int);
extern Kind copyKind(Kind, int);

extern bool eqKind(Kind, Kind);
extern Kind getKind(Cell, int);

extern List genvarTyvar(int, List);
extern List genvarType(Type, int, List);

extern bool doesntOccurIn(Tyvar *, Type, int);
extern bool unify(Type, int, Type, int);
extern bool kunify(Kind, int, Kind, int);

extern Typing typeTuple(Cell);
extern Kind simpleKind(int);
extern Typing varKind(int);

extern bool samePred(Cell, int, Cell, int);
extern bool matchPred(Cell, int, Cell, int);
extern bool unifyPred(Cell, int, Cell, int);
extern Inst findInstFor(Cell, int, int *);

extern void improve(int, List, List);
extern void improve1(int, List, Cell, int);

extern bool sameSchemes(Type, Type);
extern bool sameType(Type, int, Type, int);
extern bool matchType(Type, int, Type, int);

/*-------------------------------------------------------------------------*/

#endif /* __SUBST_H__ */
