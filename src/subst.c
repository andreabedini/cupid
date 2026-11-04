/* --------------------------------------------------------------------------
 * Provides an implementation for the `current substitution' used during
 * type and kind inference in both static analysis and type checking.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "opts.h"
#include "subst.h"
#include "tagcheck.h"

static int numTyvars; /* no. type vars currently in use  */
static int maxTyvars = 0;
static int nextGeneric; /* number of generics found so far */

Tyvar *tyvars = 0;  /* storage for type variables          */
List genericVars;   /* list of generic vars                */
List btyvars = NIL; /* explicitly scoped type vars         */

static void expandSubst(int);
static int findBtyvsInt(Text);
static Type makeTupleType(int);
static Kind makeSimpleKind(int);
static Kind makeVarKind(int);
static void expandSyn1(Tycon, Type *, int *);
static List listTyvar(int, List);
static List listTyvars(Type, int, List);
static Cell dupTyvar(int, List);
static Cell dupTyvars(Cell, int, List);
static Pair copyNoMark(Cell, int);
static Type dropRank1Body(Type, int, int);
static Type liftRank1Body(Type, int);

static bool matchTypeAbove(Type, int, Type, int, int);
static bool varToVarBind(Tyvar *, Tyvar *);
static bool varToTypeBind(Tyvar *, Type, int);
static Type occursExpand(int, Type, int);
static void expandSynFully(Type *, int *);

static bool improveAgainst(int, List, Cell, int);
static bool instImprove(int, Class, Cell, int);
static bool pairImprove(int, Class, Cell, int, Cell, int, int);

static bool kvarToVarBind(Tyvar *, Tyvar *);
static bool kvarToTypeBind(Tyvar *, Type, int);

/* --------------------------------------------------------------------------
 * The substitution, types, and kinds:
 *
 * In the same way that values have types, type constructors (and more
 * generally, expressions built from such constructors) have kinds.
 * The syntax of kinds in the current implementation is very simple:
 *
 *        kind ::= STAR         -- the kind of types
 *              |  CONSTRAINT   -- the kind of a class
 *              |  kind -> kind -- constructors
 *              |  variables    -- either INTCELL or OFFSET
 *
 * For various reasons, this implementation uses structure sharing, instead
 * of a copying approach.  In principle, this is fast and avoids the need to
 * build new type expressions.  Unfortunately, this implementation will not
 * be able to handle *very* large expressions.
 *
 * The substitution is represented by an array of type variables each of
 * which is a triple:
 *      bound   a (skeletal) type expression, or NIL if the variable
 *              is not bound, or SKOLEM for a Skolem constant (i.e., an
 *              uninstantiable variable).
 *      offs    offset of skeleton in bound.  If isNull(bound), then offs is
 *              used to indicate whether that variable is generic (i.e. not
 *              free in the current assumption set, so it may be quantified)
 *              or fixed (i.e. free in it).  Generic variables are assigned
 *              offset numbers whilst copying type expressions (t,o) to
 *              obtain their most general form.
 *      kind    kind of value bound to type variable (`type variable' is
 *              rather inaccurate -- `constructor variable' would be better).
 * ------------------------------------------------------------------------*/

void emptySubstitution(void) {
    numTyvars = 0;
    /* If the table has grown larger than NUM_TYVARS, free 'tyvars'.
     * If not, keep it around as it's bound to get used again.
     */
    if (maxTyvars > NUM_TYVARS) {
        maxTyvars = 0;
        if (tyvars) {
            free(tyvars);
            tyvars = 0;
        }
    }
    nextGeneric = 0;
    genericVars = NIL;
    btyvars = NIL;
}

/* Make room for n further type variables; this may move tyvars. */
static void expandSubst(int n) {
    if (numTyvars + n > maxTyvars) { /* need to expand substitution */
        int newMax = maxTyvars + NUM_TYVARS;
        Tyvar *newTvs;
        int i;

        if (numTyvars + n > newMax) { /* safety precaution                 */
            errHead(0);
            errPrintf("Substitution expanding too quickly");
            errFail();
        }

        newTvs = (Tyvar *)calloc(newMax, sizeof(Tyvar));
        if (!newTvs) {
            errHead(0);
            errPrintf("Too many variables (%d) in type checker", newMax);
            errFail();
        }
        for (i = 0; i < numTyvars; ++i) { /* copy substitution     */
            newTvs[i].offs = tyvars[i].offs;
            newTvs[i].bound = tyvars[i].bound;
            newTvs[i].kind = tyvars[i].kind;
        }
        maxTyvars = 0;
        if (tyvars)
            free(tyvars);
        tyvars = newTvs;
        maxTyvars = newMax;
    }
}

/* Allocate new type variables all of kind STAR */
int newTyvars(int n) {
    int beta = numTyvars;

    expandSubst(n);
    for (numTyvars += n; n > 0; n--) {
        tyvars[numTyvars - n].offs = UNUSED_GENERIC;
        tyvars[numTyvars - n].bound = NIL;
        tyvars[numTyvars - n].kind = STAR;
    }
    return beta;
}

/* If k = k0 -> k1 -> ... -> kn then allocate n vars with kinds k0, k1, ...,
 * k(n-1). */
int newKindedVars(Kind k) {
    assertKinds(k);
    int beta = numTyvars;
    for (; isPair(k); k = snd(k)) {
        expandSubst(1);
        tyvars[numTyvars].offs = UNUSED_GENERIC;
        tyvars[numTyvars].bound = NIL;
        tyvars[numTyvars].kind = fst(k);
        numTyvars++;
    }
    return beta;
}

/* Instantiate type, read at offset o: a polymorphic type gets fresh
 * variables for its quantified ones and the result is read at theirs, a
 * qualified type has its context peeled off, and anything else is the
 * monotype (type,o). */
Typing instantiate(Type type, int o) {
    Typing ty = typing(type, o);
    if (nonNull(type))
        assertType(type);

    if (nonNull(ty.type)) { /* instantiate type expression ?    */

        if (isPolyType(ty.type)) { /* Polymorphic type scheme ?     */
            Kinds ks = polySigOf(ty.type);
            ty.offs = newKindedVars(ks);
            ty.type = monotypeOf(ty.type);
            for (; isAp(ks); ks = arg(ks))
                ty.free++;
        }

        if (isQualType(ty.type)) { /* Qualified type?                       */
            ty.preds = fst(snd(ty.type));
            ty.type = snd(snd(ty.type));
        }
    }
    return ty;
}

/* --------------------------------------------------------------------------
 * Bound type variables:
 * ------------------------------------------------------------------------*/

Pair findBtyvs(Text t) {
    List bts = btyvars;
    for (; nonNull(bts); bts = tl(bts)) {
        List bts1 = hd(bts);
        for (; nonNull(bts1); bts1 = tl(bts1))
            if (t == textOf(fst(hd(bts1))))
                return hd(bts1);
    }
    return NIL;
}

/* Look for bound type variable expecting to find an integer */
static int findBtyvsInt(Text t) {
    Pair p = findBtyvs(t);
    if (isNull(p))
        internal("findBtyvsInt");
    return intOf(snd(p));
}

void markBtyvs(void) {
    List bts = btyvars;
    for (; nonNull(bts); bts = tl(bts)) {
        List bts1 = hd(bts);
        for (; nonNull(bts1); bts1 = tl(bts1))
            markTyvar(intOf(snd(hd(bts1))));
    }
}

/* Localize type to eliminate refs to explicitly scoped vars */
Type localizeBtyvs(Type t) {
    switch (whatIs(t)) {
    case RANK2:
    case POLYTYPE:
        snd(snd(t)) = localizeBtyvs(snd(snd(t)));
        break;

    case QUAL:
        fst(snd(t)) = localizeBtyvs(fst(snd(t)));
        snd(snd(t)) = localizeBtyvs(snd(snd(t)));
        break;

    case AP:
        fst(t) = localizeBtyvs(fst(t));
        snd(t) = localizeBtyvs(snd(t));
        break;

    case VARIDCELL:
    case VAROPCELL:
        return mkInt(findBtyvsInt(textOf(t)));
    default:
        break;
    }
    return t;
}

/* --------------------------------------------------------------------------
 * Dereference or bind types in substitution:
 * ------------------------------------------------------------------------*/

/* The type variable represented by (t,o), or null if it is not one. */
Tyvar *getTypeVar(Type t, int o) {
    switch (whatIs1(t)) {
    case INTCELL:
        return tyvar(intOf(t));
    case OFFSET:
        return tyvar(o + offsetOf(t));
    case VARIDCELL:
    case VAROPCELL:
        return tyvar(findBtyvsInt(textOf(t)));
    default:
        break;
    }
    return ((Tyvar *)0);
}

/* The type held in type variable vn, followed through the variables it
 * is bound to; the last variable itself if the chain ends unbound */
Typing tyvarType(int vn) {
    Tyvar *tyv;

    while ((tyv = tyvar(vn)), isBound(tyv))
        switch (whatIs(tyv->bound)) {
        case INTCELL:
            vn = intOf(tyv->bound);
            break;

        case OFFSET:
            vn = offsetOf(tyv->bound) + (tyv->offs);
            break;

        case VARIDCELL:
        case VAROPCELL:
            vn = findBtyvsInt(textOf(tyv->bound));
            break;

        default:
            return typing(tyv->bound, tyv->offs);
        }
    return typing(aVar, vn);
}

void bindTv(int vn, Type t, int o) {
    Tyvar *tyv = tyvar(vn);
    tyv->bound = t;
    tyv->offs = o;
}

/* Get value at head of type exp.; leaves its number of args in argCount. */
Cell getDerefHead(Type t, int o) {
    Tyvar *tyv;
    argCount = 0;
    for (;;) {
        while (isAp(t)) {
            argCount++;
            t = fun(t);
        }
        if ((tyv = getTypeVar(t, o)) && isBound(tyv)) {
            t = tyv->bound;
            o = tyv->offs;
        } else
            break;
    }
    return t;
}

/* --------------------------------------------------------------------------
 * Expand type synonyms:
 * ------------------------------------------------------------------------*/

/* Expand type synonym with head h and ar args (NB. ar>=tyconRec(h)->arity):
 * the original expression is (*at,*ao), and the expansion is returned there. */
void expandSyn(Tycon h, int ar, Type *at, int *ao) {
    ar -= tyconRec(h)->arity;
    if (ar == 0)
        expandSyn1(h, at, ao);
    else {
        /* With more args than the arity, isolate the args that the expansion
         * will not change, then embed its result back in (*at,*ao). */
        Type t = *at;
        int o = *ao;
        Type args = NIL;
        int i;
        while (ar-- > 0) {
            args = cons(arg(t), args);
            t = fun(t);
            deRef(&t, &o);
        }
        expandSyn1(h, &t, &o);
        bindTv((i = newTyvars(1)), t, o);
        tyvar(i)->kind = getKind(t, o);
        *at = applyToArgs(mkInt(i), args);
    }
}

/* Expand type synonym with head h and exactly tyconRec(h)->arity args: the
 * original expression is (*at,*ao), and the expansion is returned there. */
static void expandSyn1(Tycon h, Type *at, int *ao) {
    int n = tyconRec(h)->arity;
    Type t = *at;
    int o = *ao;
    Tyvar *tyv;

    *at = tyconRec(h)->defn;
    *ao = newKindedVars(tyconRec(h)->kind);
    for (; 0 < n--; t = fun(t)) {
        tyv = deRef(&t, &o);
        if (tyv || !isAp(t))
            internal("expandSyn1");
        bindTv(*ao + n, arg(t), o);
    }
}

/* --------------------------------------------------------------------------
 * Marking fixed variables in type expressions:
 * ------------------------------------------------------------------------*/

/* Set all unbound type vars to unused generic variables */
void clearMarks(void) {
    int i;
    for (i = 0; i < numTyvars; ++i)
        if (!isBound(tyvar(i)))
            tyvar(i)->offs = UNUSED_GENERIC;
    genericVars = NIL;
    nextGeneric = 0;
}

/* Set all unbound type vars to be fixed vars */
void markAllVars(void) {
    int i;
    for (i = 0; i < numTyvars; ++i)
        if (!isBound(tyvar(i)))
            tyvar(i)->offs = FIXED_TYVAR;
    genericVars = NIL;
    nextGeneric = 0;
}

void resetGenerics(void) {
    int i;
    for (i = 0; i < numTyvars; ++i)
        if (!isBound(tyvar(i)) && tyvar(i)->offs >= GENERIC)
            tyvar(i)->offs = UNUSED_GENERIC;
    genericVars = NIL;
    nextGeneric = 0;
}

void markTyvar(int vn) {
    Tyvar *tyv = tyvar(vn);

    if (isBound(tyv))
        markType(tyv->bound, tyv->offs);
    else
        (tyv->offs) = FIXED_TYVAR;
}

void markType(Type t, int o) {
    stackCheck();
    allowBreak();
    switch (whatIs(t)) {
    case POLYTYPE:
    case QUAL:
    case TYCON:
    case TUPLE:
        return;

    case AP:
        markType(fst(t), o);
        markType(snd(t), o);
        return;

    case OFFSET:
        markTyvar(o + offsetOf(t));
        return;

    case INTCELL:
        markTyvar(intOf(t));
        return;

    case VARIDCELL:
    case VAROPCELL:
        markTyvar(findBtyvsInt(textOf(t)));
        return;

    case RANK2:
        markType(snd(snd(t)), o);
        return;

    default:
        internal("markType");
    }
}

void markPred(Cell pi) {
    Cell cl = fst3(pi);
    int o = intOf(snd3(pi));

    for (; isAp(cl); cl = fun(cl))
        markType(arg(cl), o);
}

/* --------------------------------------------------------------------------
 * Copy type expression from substitution to make a single type expression:
 * ------------------------------------------------------------------------*/

/* Calculate most general form of type bound to given type var */
Type copyTyvar(int vn) {
    Tyvar *tyv = tyvar(vn);

    if ((tyv->bound) == SKOLEM) {
        return mkInt(vn);
    } else if (tyv->bound) {
        return copyType(tyv->bound, tyv->offs);
    }

    switch (tyv->offs) {
    case FIXED_TYVAR:
        return mkInt(vn);

    case UNUSED_GENERIC:
        (tyv->offs) = GENERIC + nextGeneric++;
        if (nextGeneric >= NUM_OFFSETS) {
            errHead(0);
            errPrintf("Too many quantified type variables");
            errFail();
        }
        genericVars = cons(mkInt(vn), genericVars);

    default:
        return mkOffset(tyv->offs - GENERIC);
    }
}

/* Calculate most general form of type expression (t,o) */
Type copyType(Type t, int o) {
    assertType(t);
    stackCheck();
    allowBreak();
    switch (whatIs(t)) {
    case AP: {
        Type l = copyType(fst(t), o); /* ensure correct */
        Type r = copyType(snd(t), o); /* eval. order    */
        return ap(l, r);
    }
    case OFFSET:
        return copyTyvar(o + offsetOf(t));
    case INTCELL:
        return copyTyvar(intOf(t));
    case VARIDCELL:
    case VAROPCELL:
        return copyTyvar(findBtyvsInt(textOf(t)));
    default:
        break;
    }

    return t;
}

Cell copyPred(Cell pi, int o) {
    if (isAp(pi)) {
        Cell temp = copyPred(fun(pi), o); /* to ensure correct order of eval. */
        return ap(temp, copyType(arg(pi), o));
    } else
        return pi;
}

/* Flatten type by chasing all references and collapsing OFFSETS to absolute
 * indexes */
Type zonkTyvar(int vn) {
    Tyvar *tyv = tyvar(vn);

    if (tyv->bound)
        return zonkType(tyv->bound, tyv->offs);
    else
        return mkInt(vn);
}

Type zonkType(Type t, int o) {
    stackCheck();
    allowBreak();
    switch (whatIs(t)) {
    case AP: {
        Type l = zonkType(fst(t), o); /* ensure correct */
        Type r = zonkType(snd(t), o); /* eval. order    */
        return ap(l, r);
    }
    case OFFSET:
        return zonkTyvar(o + offsetOf(t));
    case INTCELL:
        return zonkTyvar(intOf(t));
    default:
        break;
    }

    return t;
}

Kind copyKindvar(int vn) {
    Tyvar *tyv = tyvar(vn);
    if (tyv->bound)
        return copyKind(tyv->bound, tyv->offs);
    return STAR; /* any unbound variable defaults to the kind of all types */
}

Kind copyKind(Kind k, int o) {
    switch (whatIs(k)) {
    case AP: {
        Kind l = copyKind(fst(k), o); /* ensure correct */
        Kind r = copyKind(snd(k), o); /* eval. order    */
        return ap(l, r);
    }
    case OFFSET:
        return copyKindvar(o + offsetOf(k));
    case INTCELL:
        return copyKindvar(intOf(k));
    default:
        break;
    }
    return k;
}

/* --------------------------------------------------------------------------
 * Copy type expression from substitution without marking:
 * ------------------------------------------------------------------------*/

static List listTyvar(int vn, List ns) {
    Tyvar *tyv = tyvar(vn);

    if (isBound(tyv)) {
        return listTyvars(tyv->bound, tyv->offs, ns);
    } else if (!intIsMember(vn, ns)) {
        ns = cons(mkInt(vn), ns);
    }
    return ns;
}

static List listTyvars(Cell t, int o, List ns) {
    switch (whatIs(t)) {
    case AP:
        return listTyvars(fst(t), o, listTyvars(snd(t), o, ns));
    case OFFSET:
        return listTyvar(o + offsetOf(t), ns);
    case INTCELL:
        return listTyvar(intOf(t), ns);
    default:
        break;
    }
    return ns;
}

static Cell dupTyvar(int vn, List ns) {
    Tyvar *tyv = tyvar(vn);

    if (isBound(tyv)) {
        return dupTyvars(tyv->bound, tyv->offs, ns);
    } else {
        int i = 0;
        for (; nonNull(ns) && vn != intOf(hd(ns)); ns = tl(ns)) {
            i++;
        }
        return mkOffset(i);
    }
}

static Cell dupTyvars(Cell t, int o, List ns) {
    switch (whatIs(t)) {
    case AP: {
        Type l = dupTyvars(fst(t), o, ns);
        Type r = dupTyvars(snd(t), o, ns);
        return ap(l, r);
    }
    case OFFSET:
        return dupTyvar(o + offsetOf(t), ns);
    case INTCELL:
        return dupTyvar(intOf(t), ns);
    default:
        break;
    }
    return t;
}

/* Copy a type or predicate without changing marks */
static Cell copyNoMark(Cell t, int o) {
    List ns = listTyvars(t, o, NIL);
    Cell result = pair(ns, dupTyvars(t, o, ns));
    for (; nonNull(ns); ns = tl(ns)) {
        hd(ns) = tyvar(intOf(hd(ns)))->kind;
    }
    return result;
}

/* --------------------------------------------------------------------------
 * Dropping and lifting of type schemes that appear in rank 2 position:
 * ------------------------------------------------------------------------*/

Type dropRank2(Type t, int alpha, int n) {
    if (whatIs(t) == RANK2) {
        Cell r = fst(snd(t));
        int i = intOf(r);
        Type as = NIL;
        for (t = snd(snd(t)); i > 0; i--) {
            Type a = arg(fun(t));
            if (isPolyType(a))
                a = dropRank1(a, alpha, n);
            as = fn(a, as);
            t = arg(t);
        }
        t = ap(RANK2, pair(r, revOnto(as, t)));
    }
    return t;
}

/* Copy rank1 argument type t to make a rank1 type scheme */
Type dropRank1(Type t, int alpha, int n) {
    if (n > 0 && isPolyType(t))
        t = mkPolyType(polySigOf(t), dropRank1Body(monotypeOf(t), alpha, n));
    return t;
}

static Type dropRank1Body(Type t, int alpha, int n) {
    switch (whatIs(t)) {
    case OFFSET: {
        int m = offsetOf(t);
        return (m >= n) ? mkOffset(m - n) : mkInt(alpha + m);
    }

    case POLYTYPE:
        return mkPolyType(polySigOf(t), dropRank1Body(monotypeOf(t), alpha, n));

    case QUAL:
        return ap(QUAL, dropRank1Body(snd(t), alpha, n));

    case RANK2:
        return ap(RANK2,
                  pair(fst(snd(t)), dropRank1Body(snd(snd(t)), alpha, n)));

    case AP:
        return ap(dropRank1Body(fun(t), alpha, n),
                  dropRank1Body(arg(t), alpha, n));

    default:
        return t;
    }
}

void liftRank2Args(List as, int alpha, int m) {
    int i = 0;
    for (; i < m; i++)
        copyTyvar(alpha + i);
    for (m = nextGeneric; nonNull(as); as = tl(as)) {
        Type ta = arg(fun(as));
        ta = isPolyType(ta) ? liftRank1Body(ta, m) : copyType(ta, alpha);
        arg(fun(as)) = ta;
    }
}

Type liftRank2(Type t, int alpha, int m) {
    if (whatIs(t) == RANK2) {
        Cell r = fst(snd(t));
        int i = 0;
        Type as = NIL;
        for (; i < m; i++)
            copyTyvar(alpha + i);
        m = nextGeneric;
        t = snd(snd(t));
        for (i = intOf(r); i > 0; i--) {
            Type a = arg(fun(t));
            a = isPolyType(a) ? liftRank1Body(a, m) : copyType(a, alpha);
            as = fn(a, as);
            t = arg(t);
        }
        t = ap(RANK2, pair(r, revOnto(as, copyType(t, alpha))));
    } else
        t = copyType(t, alpha);
    return t;
}

Type liftRank1(Type t, int alpha, int m) {
    if (m > 0 && isPolyType(t)) {
        int i = 0;
        resetGenerics();
        for (; i < m; i++)
            copyTyvar(alpha + i);
        t = liftRank1Body(t, nextGeneric);
    }
    return t;
}

static Type liftRank1Body(Type t, int n) {
    switch (whatIs(t)) {
    case OFFSET:
        return mkOffset(n + offsetOf(t));

    case INTCELL:
        return copyTyvar(intOf(t));

    case VARIDCELL:
    case VAROPCELL:
        return copyTyvar(findBtyvsInt(textOf(t)));

    case POLYTYPE:
        return mkPolyType(polySigOf(t), liftRank1Body(monotypeOf(t), n));

    case QUAL:
        return ap(QUAL, liftRank1Body(snd(t), n));

    case RANK2:
        return ap(RANK2, pair(fst(snd(t)), liftRank1Body(snd(snd(t)), n)));

    case AP:
        return ap(liftRank1Body(fun(t), n), liftRank1Body(arg(t), n));

    default:
        return t;
    }
}

/* --------------------------------------------------------------------------
 * Support for `kind preserving substitutions' from unification:
 * ------------------------------------------------------------------------*/

/* Check that two (mono)kinds are equal */
bool eqKind(Kind k1, Kind k2) {
    return k1 == k2 || (isPair(k1) && isPair(k2) && eqKind(fst(k1), fst(k2)) &&
                        eqKind(snd(k1), snd(k2)));
}

Kind getKind(Cell c, int o) {
    assertType(c);
    if (isAp(c)) /* application    */
        return snd(getKind(fst(c), o));
    switch (whatIs(c)) {
    case TUPLE:
        return simpleKind(tupleOf(c)); /*(,)::* -> * -> * */
    case OFFSET:
        return tyvar(o + offsetOf(c))->kind;
    case INTCELL:
        return tyvar(intOf(c))->kind;
    case VARIDCELL:
    case VAROPCELL:
        return tyvar(findBtyvsInt(textOf(c)))->kind;
    case TYCON:
        return tyconRec(c)->kind;
    default:
        break;
    }
    internal("getKind");
    return STAR; /* not reached */
}

/* --------------------------------------------------------------------------
 * Find generic variables in a type:
 * ------------------------------------------------------------------------*/

/* Calculate list of generic vars thru variable vn, prepended to list vs */
Type genvarTyvar(int vn, List vs) {
    Tyvar *tyv = tyvar(vn);

    if (isBound(tyv))
        return genvarType(tyv->bound, tyv->offs, vs);
    else if (tyv->offs == UNUSED_GENERIC) {
        tyv->offs += GENERIC + nextGeneric++;
        return cons(mkInt(vn), vs);
    } else if (tyv->offs >= GENERIC && !intIsMember(vn, vs))
        return cons(mkInt(vn), vs);
    else
        return vs;
}

/* Calculate list of generic vars in type expression (t,o), prepended to vs */
List genvarType(Type t, int o, List vs) {
    switch (whatIs(t)) {
    case AP:
        return genvarType(snd(t), o, genvarType(fst(t), o, vs));
    case OFFSET:
        return genvarTyvar(o + offsetOf(t), vs);
    case INTCELL:
        return genvarTyvar(intOf(t), vs);
    case VARIDCELL:
    case VAROPCELL:
        return genvarTyvar(findBtyvsInt(textOf(t)), vs);
    default:
        break;
    }
    return vs;
}

/* --------------------------------------------------------------------------
 * Occurs check:
 * ------------------------------------------------------------------------*/

bool doesntOccurIn(Tyvar *lookFor, Type t, int o) {
    Tyvar *tyv;

    stackCheck();
    allowBreak();
    for (;;) {
        tyv = deRef(&t, &o);
        if (tyv) /* type variable                  */
            return tyv != lookFor;
        else if (isAp(t)) { /* application                         */
            if (doesntOccurIn(lookFor, snd(t), o))
                t = fst(t);
            else
                return false;
        } else /* no variable found                */
            break;
    }
    return true;
}

/* --------------------------------------------------------------------------
 * Unification algorithm:
 * ------------------------------------------------------------------------*/

char *unifyFails = 0;     /* Unification error message     */
static int bindAbove = 0; /* Used to restrict var binding          */

static void bindOnlyAbove(int beta) {
    bindAbove = beta;
}
static void noBind(void) {
    bindAbove = MAXPOSINT;
}
static void unrestrictBind(void) {
    bindAbove = 0;
}

static bool varToVarBind(Tyvar *tyv1, Tyvar *tyv2) {
    if (tyv1 != tyv2) { /* If vars are same, nothing to do!*/

        /* Check that either tyv1 or tyv2 is in allowed range for binding  */
        /* and is not a Skolem constant, and swap vars if nec. so we can   */
        /* bind to tyv1.                                                   */

        if (tyvNum(tyv1) < bindAbove || tyv1->bound == SKOLEM) {
            if (tyvNum(tyv2) < bindAbove || tyv2->bound == SKOLEM) {
                unifyFails = "types do not match";
                return false;
            } else {
                Tyvar *tyv = tyv1;
                tyv1 = tyv2;
                tyv2 = tyv;
            }
        }
        if (!eqKind(tyv1->kind, tyv2->kind)) {
            unifyFails = "constructor variable kinds do not match";
            return false;
        }
        tyv1->bound = aVar;
        tyv1->offs = tyvNum(tyv2);
    }
    return true;
}

/* Make binding tyv := (t,o), where t is guaranteed not to be a variable or to
 * have a synonym as outermost constructor */
static bool varToTypeBind(Tyvar *tyv, Type t, int o) {
    if (tyvNum(tyv) < bindAbove) { /* Check that tyv is in range       */
        unifyFails = "types do not match";
        return false;
    } else if (tyv->bound == SKOLEM) { /* Check that it is not Skolemized */
        unifyFails = "cannot instantiate Skolem constant";
        return false;
    }
    /* Carry out occurs check, seeing through synonyms that drop it */
    if (!doesntOccurIn(tyv, t, o)) {
        int vn = tyvNum(tyv);
        t = occursExpand(vn, t, o);
        o = 0;
        tyv = tyvar(vn); /* expanding may have moved the tyvars */
        if (isNull(t)) {
            unifyFails = "unification would give infinite type";
            return false;
        }
    }
    if (!eqKind(tyv->kind, getKind(t, o)))
        unifyFails = "kinds do not match";
    else {
        tyv->bound = t;
        tyv->offs = o;
        return true;
    }
    return false;
}

/* The type (t,o), with no offsets in it, and the synonyms in it expanded
 * wherever that leaves the variable numbered vn out; NIL if vn is in it
 * even so.  With type K a = Int, a occurs in [K a] but not in its
 * expansion [Int], so a ~ [K a] is not an infinite type. */
static Type occursExpand(int vn, Type t, int o) {
    Tyvar *tyv;

    stackCheck();
    allowBreak();
    tyv = deRef(&t, &o);
    if (tyv) {
        return tyvNum(tyv) == vn ? NIL : mkInt(tyvNum(tyv));
    }
    if (!doesntOccurIn(tyvar(vn), t, o)) {
        Cell h = getDerefHead(t, o);
        if (isSynonym(h) && argCount >= tyconRec(h)->arity) {
            expandSyn(h, argCount, &t, &o);
            return occursExpand(vn, t, o);
        }
    }
    if (isAp(t)) {
        Type f = occursExpand(vn, fun(t), o);
        Type x;
        if (isNull(f)) {
            return NIL;
        }
        x = occursExpand(vn, arg(t), o);
        return isNull(x) ? NIL : ap(f, x);
    }
    return t;
}

bool unify(Type t1, int o1, Type t2, int o2) {
    assertType(t1);
    assertType(t2);
    Tyvar *tyv1, *tyv2;

    stackCheck();
    allowBreak();
    tyv1 = deRef(&t1, &o1);
    tyv2 = deRef(&t2, &o2);

un:
    if (tyv1)
        if (tyv2)
            return varToVarBind(tyv1, tyv2); /* t1, t2 variables    */
        else {
            Cell h2 = getDerefHead(t2, o2); /* t1 variable, t2 not */
            if (isSynonym(h2) && argCount >= tyconRec(h2)->arity) {
                /* may grow (and realloc) tyvars, so re-derive tyv1 too */
                expandSyn(h2, argCount, &t2, &o2);
                tyv1 = deRef(&t1, &o1);
                tyv2 = deRef(&t2, &o2);
                goto un;
            }
            return varToTypeBind(tyv1, t2, o2);
        }
    else if (tyv2) {
        Cell h1 = getDerefHead(t1, o1); /* t2 variable, t1 not */
        if (isSynonym(h1) && argCount >= tyconRec(h1)->arity) {
            /* may grow (and realloc) tyvars, so re-derive tyv2 too */
            expandSyn(h1, argCount, &t1, &o1);
            tyv1 = deRef(&t1, &o1);
            tyv2 = deRef(&t2, &o2);
            goto un;
        }
        return varToTypeBind(tyv2, t1, o1);
    } else { /* t1, t2 not vars    */
        Type h1 = getDerefHead(t1, o1);
        int a1 = argCount;
        Type h2 = getDerefHead(t2, o2);
        int a2 = argCount;

        if (isOffset(h1) || isInt(h1))
            h1 = NIL; /* represent var by NIL*/
        if (isOffset(h2) || isInt(h2))
            h2 = NIL;

        /* The same synonym on both sides does not make its arguments    */
        /* equal, as it may drop them (type K a = Int): expand it below  */
        if (nonNull(h1) && h1 == h2 &&
            !(isSynonym(h1) && a1 >= tyconRec(h1)->arity)) {
            /* Assuming well-formed types, both must have same no of args */
            if (a1 != a2) {
                unifyFails = "incompatible constructors";
                return false;
            }
            while (isAp(t1)) {
                if (!unify(arg(t1), o1, arg(t2), o2))
                    return false;
                t1 = fun(t1);
                tyv1 = deRef(&t1, &o1);
                t2 = fun(t2);
                tyv2 = deRef(&t2, &o2);
            }
            unifyFails = 0;
            return true;
        }

        /* Types do not match -- look for type synonyms to expand */

        if (isSynonym(h1) && a1 >= tyconRec(h1)->arity) {
            expandSyn(h1, a1, &t1, &o1);
            tyv1 = deRef(&t1, &o1);
            goto un;
        }
        if (isSynonym(h2) && a2 >= tyconRec(h2)->arity) {
            expandSyn(h2, a2, &t2, &o2);
            tyv2 = deRef(&t2, &o2);
            goto un;
        }

        if ((isNull(h1) && a1 <= a2) || /* maybe one head is a variable? */
            (isNull(h2) && a2 <= a1)) {
            for (;;) {
                tyv1 = deRef(&t1, &o1);
                tyv2 = deRef(&t2, &o2);

                if (tyv1) /* unify heads!          */
                    if (tyv2)
                        return varToVarBind(tyv1, tyv2);
                    else
                        return varToTypeBind(tyv1, t2, o2);
                else if (tyv2)
                    return varToTypeBind(tyv2, t1, o1);

                /* at this point, neither t1 nor t2 is a variable. In  */
                /* addition, they must both be APs unless one of the   */
                /* head variables has been bound during unification of */
                /* the arguments.                                          */

                if (!isAp(t1) || !isAp(t2)) { /* might not be APs*/
                    unifyFails = 0;
                    return t1 == t2;
                }
                if (!unify(arg(t1), o1, arg(t2), o2)) /* o/w must be APs */
                    return false;
                t1 = fun(t1);
                t2 = fun(t2);
            }
        }
    }
    unifyFails = 0;
    return false;
}

/* Repeatedly expand synonyms at the head of (*at,*ao), in place */
static void expandSynFully(Type *at, int *ao) {
    Type h;
    int a;
    for (;;) {
        h = getDerefHead(*at, *ao);
        a = argCount;
        if (!isSynonym(h) || a < tyconRec(h)->arity)
            return;
        expandSyn(h, a, at, ao);
    }
}

/* Extract t, if type is of the form IO t, otherwise return NULL cell */
Type getProgType(List ks, Type type) {
    bool result;
    int alpha;
    int beta;
    if (isPolyOrQualType(type))
        return 0;
    emptySubstitution();
    alpha = newKindedVars(ks);
    expandSynFully(&type, &alpha);
    beta = newTyvars(1);
    bindOnlyAbove(beta);
    result = unify(type, alpha, typeProgIO, beta);
    unrestrictBind();
    emptySubstitution();
    return result ? arg(type) : 0;
}

/* --------------------------------------------------------------------------
 * Matching predicates:
 *
 * There are (at least) four situations where we need to match up pairs
 * of predicates:
 *
 *   1) Testing to see if two predicates are the same (ignoring differences
 *      caused by the use of type synonyms, for example).
 *
 *   2) Matching a predicate with the head of its class so that we can
 *      find the corresponding superclass predicates.  If the predicates
 *      have already been kind-checked, and the classes are known to be
 *      the same, then this should never fail.
 *
 *   3) Matching a predicate against the head of an instance to see if
 *      that instance is applicable.
 *
 *   4) Matching two instance heads to see if there is an overlap.
 *
 * For (1), we need a matching process that does not bind any variables.
 * For (2) and (3), we need to use one-way matching, only allowing
 * variables in the class or instance head to be instantiated.  For
 * (4), we need two-way unification.
 * ------------------------------------------------------------------------*/

/* Test to see if predicates are the same, with no binding of the variables in
 * either one. Assumes preds are kind correct with the same class. */
bool samePred(Cell pi1, int o1, Cell pi, int o) {
    bool result;
    noBind();
    result = unifyPred(pi1, o1, pi, o);
    unrestrictBind();
    return result;
}

/* One way match predicate (pi1,o1) against (pi,o), allowing only vars in 2nd
 * pred to be bound.  Assumes preds are kind correct with the same class and
 * that no vars have been alloc'd since o. */
bool matchPred(Cell pi1, int o1, Cell pi, int o) {
    assertPred(pi1);
    assertPred(pi);
    bool result;
    bindOnlyAbove(o);
    result = unifyPred(pi1, o1, pi, o);
    unrestrictBind();
    return result;
}

/* Assumes preds are kind correct with the same class. */
bool unifyPred(Cell pi1, int o1, Cell pi, int o) {
    assertPred(pi1);
    assertPred(pi);
    for (; isAp(pi1); pi1 = fun(pi1), pi = fun(pi)) {
        if (!isAp(pi) || !unify(arg(pi1), o1, arg(pi), o))
            return false;
    }
    /* pi1 has exhausted its argument chain, we also need to check that
     pi has no remaining arguments.  However, under this condition,
     the pi1 == pi will always return false, giving the desired
     result. */

    return pi1 == pi;
}

/* Find matching instance for pred (pi,o), or otherwise NIL. If a match is
 * found, the instance's variables have been allocated from *beta, so its
 * specifics can be read at that offset. */
Inst findInstFor(Cell pi, int o, int *beta) {
    assertPred(pi);
    Class c = getHead(pi);
    List ins;
    Cell kspi = NIL;

    if (!isClass(c))
        return NIL;

    for (ins = cclassRec(c)->instances; nonNull(ins); ins = tl(ins)) {
        Inst in = hd(ins);
        int vars = newKindedVars(instRec(in)->kinds);
        if (matchPred(pi, o, instRec(in)->head, vars)) {
            *beta = vars;
            return in;
        } else {
            numTyvars = vars;
            if (allowOverlap && !allowUnsafeOverlap) {
                int alpha = newKindedVars(instRec(in)->kinds);
                if (isNull(kspi)) {
                    kspi = copyNoMark(pi, o);
                }
                vars = newKindedVars(fst(kspi));
                if (matchPred(instRec(in)->head, alpha, snd(kspi), vars)) {
                    numTyvars = alpha;
                    return NIL;
                }
                numTyvars = alpha;
            }
        }
    }
    unrestrictBind();

    return NIL;
}

/* --------------------------------------------------------------------------
 * Improvement:
 * ------------------------------------------------------------------------*/

void improve(int line, List sps, List ps) {
    bool improved;
    List ps1;
    do {
        improved = false;
        for (ps1 = ps; nonNull(ps1); ps1 = tl(ps1)) {
            Cell pi = fst3(hd(ps1));
            int o = intOf(snd3(hd(ps1)));
            Cell c = getHead(pi);
            if (isClass(c) && nonNull(cclassRec(c)->xfds)) {
                improved |= improveAgainst(line, sps, pi, o);
                improved |= instImprove(line, c, pi, o);
                improved |= improveAgainst(line, tl(ps1), pi, o);
            }
        }
    } while (improved);
}

void improve1(int line, List sps, Cell pi, int o) {
    bool improved;
    Cell c = getHead(pi);
    do {
        improved = false;
        if (isClass(c) && nonNull(cclassRec(c)->xfds)) {
            improved |= improveAgainst(line, sps, pi, o);
            improved |= instImprove(line, c, pi, o);
        }
    } while (improved);
}

bool improveAgainst(int line, List ps, Cell pi, int o) {
    bool improved = false;
    Cell h = getHead(pi);
    for (; nonNull(ps); ps = tl(ps)) {
        Cell pr = hd(ps);
        Cell pi1 = fst3(pr);
        int o1 = intOf(snd3(pr));
        Cell h1 = getHead(pi1);
        /* it would be nice to optimize for the common case
           where h == h1 */
        if (isClass(h) && isClass(h1)) {
            improved |= pairImprove(line, h, pi, o, pi1, o1, numTyvars);
            if (h != h1)
                improved |= pairImprove(line, h1, pi1, o1, pi, o, numTyvars);
        }
    }
    return improved;
}

/* should emulate findInstFor behaviour of shorting out if the
   predicate would match a more general signature... */
bool instImprove(int line, Class c, Cell pi, int o) {
    bool improved = false;
    List ins = cclassRec(c)->instances;
    for (; nonNull(ins); ins = tl(ins)) {
        Cell in = hd(ins);
        int alpha = newKindedVars(instRec(in)->kinds);
        improved |=
            pairImprove(line, c, pi, o, instRec(in)->head, alpha, alpha);
    }
    return improved;
}

/* Look for improvement of (pi1,o1) against (pi2,o2) */
bool pairImprove(int line, Class c, Cell pi1, int o1, Cell pi2, int o2,
                 int above) {
    bool improved = false;
    List xfds = cclassRec(c)->xfds;
    for (; nonNull(xfds); xfds = tl(xfds)) {
        Cell xfd = hd(xfds);
        Cell hs = fst(xfd);
        int alpha = (-1);
        for (; nonNull(hs); hs = tl(hs)) {
            Cell h = hd(hs);
            Class d = getHead(h);
            alpha = newKindedVars(cclassRec(d)->kinds);
            if (matchPred(pi2, o2, h, alpha))
                break;
            numTyvars = alpha;
        }
        if (nonNull(hs)) {
            List fds = snd(xfd);
            for (; nonNull(fds); fds = tl(fds)) {
                List as = fst(hd(fds));
                bool same = true;
                for (; same && nonNull(as); as = tl(as)) {
                    int n = offsetOf(hd(as));
                    same &= matchTypeAbove(nthArg(n, pi1), o1, mkOffset(n),
                                           alpha, above);
                }
                if (isNull(as) && same) {
                    for (as = snd(hd(fds)); same && nonNull(as); as = tl(as)) {
                        int n = offsetOf(hd(as));
                        Type t1 = nthArg(n, pi1);
                        Type t2 = mkOffset(n);
                        if (!matchTypeAbove(t1, o1, t2, alpha, above)) {
                            same &= unify(t1, o1, t2, alpha);
                            improved = true;
                        }
                    }
                    if (!same) {
                        errHead(line);
                        errPrintf(
                            "Constraints are not consistent with functional dependency");

                        errPrintf("\n*** Constraint       : ");
                        errPred(copyPred(pi1, o1));
                        errPrintf("\n*** And constraint   : ");
                        errPred(copyPred(pi2, o2));
                        errPrintf("\n*** For class        : ");
                        errPred(cclassRec(c)->head);
                        errPrintf("\n*** Break dependency : ");
                        errFD(hd(fds));
                        errPrintf("\n");
                        errFail();
                    }
                }
            }
            numTyvars = alpha;
        }
    }
    return improved;
}

/* --------------------------------------------------------------------------
 * Compare type schemes:
 * ------------------------------------------------------------------------*/

bool sameSchemes(Type s, Type s1) {
    int o = 0;
    int m = 0;
    int nr2 = 0;
    bool b = isPolyType(s); /* Check quantifiers are the same  */
    bool b1 = isPolyType(s1);
    if (b || b1) {
        if (b && b1 && eqKind(polySigOf(s), polySigOf(s1))) {
            Kind k = polySigOf(s);
            s = monotypeOf(s);
            s1 = monotypeOf(s1);
            o = newKindedVars(k);
            for (; isAp(k); k = arg(k))
                m++;
        } else
            return false;
    }

    b = (whatIs(s) == QUAL); /* Check that contexts are the same*/
    b1 = (whatIs(s1) == QUAL);
    if (b || b1) {
        if (b && b1) {
            List ps = fst(snd(s));
            List ps1 = fst(snd(s1));
            noBind();
            while (nonNull(ps) && nonNull(ps1)) {
                Cell pi = hd(ps);
                Cell pi1 = hd(ps1);
                if (getHead(pi) != getHead(pi1) || !unifyPred(pi, o, pi1, o))
                    break;
                ps = tl(ps);
                ps1 = tl(ps1);
            }
            unrestrictBind();
            if (nonNull(ps) || nonNull(ps1))
                return false;
            s = snd(snd(s));
            s1 = snd(snd(s1));
        } else
            return false;
    }

    b = (whatIs(s) == RANK2); /* Check any rank 2 annotations      */
    b1 = (whatIs(s1) == RANK2);
    if (b || b1) {
        if (b && b1 && intOf(fst(snd(s))) == intOf(fst(snd(s1)))) {
            nr2 = intOf(fst(snd(s)));
            s = snd(snd(s));
            s1 = snd(snd(s1));
        } else
            return false;
    }

    for (; nr2 > 0; nr2--) { /* Deal with rank 2 arguments         */
        Type t = arg(fun(s));
        Type t1 = arg(fun(s1));
        b = isPolyOrQualType(t);
        b1 = isPolyOrQualType(t1);
        if (b || b1) {
            if (b && b1) {
                t = dropRank1(t, o, m);
                t1 = dropRank1(t1, o, m);
                if (!sameSchemes(t, t1))
                    return false;
            } else
                return false;
        } else {
            if (!sameType(t, o, t1, o)) {
                return false;
            }
        }
        s = arg(s);
        s1 = arg(s1);
    }

    return sameType(s, o, s1, o); /* Ensure body types are the same  */
}

/* Test to see if types are the same, with no binding of the variables in
 * either one. Assumes types are kind correct with the same kind. */
bool sameType(Type t1, int o1, Cell t, int o) {
    assertType(t1);
    assertType(t);
    bool result;
    noBind();
    result = unify(t1, o1, t, o);
    unrestrictBind();
    return result;
}

/* One way match type (t1,o1) against (t,o), allowing only vars in 2nd type to
 * be bound. Assumes types are kind correct and that no vars have been alloc'd
 * since o. */
bool matchType(Type t1, int o1, Type t, int o) {
    bool result;
    bindOnlyAbove(o);
    result = unify(t1, o1, t, o);
    unrestrictBind();
    return result;
}

/* Match, allowing only vars allocated since `a' to be bound.  This is deeply
 * hacky, since it relies on careful use of the substitution stack. */
static bool matchTypeAbove(Type t1, int o1, Type t, int o, int a) {
    bool result;
    bindOnlyAbove(a);
    result = unify(t1, o1, t, o);
    unrestrictBind();
    return result;
}

/* --------------------------------------------------------------------------
 * Unify kind expressions:
 * ------------------------------------------------------------------------*/

static bool kvarToVarBind(Tyvar *tyv1, Tyvar *tyv2) {
    if (tyv1 != tyv2) {
        tyv1->bound = aVar;
        tyv1->offs = tyvNum(tyv2);
    }
    return true;
}

/* Make binding tyv := (t,o) for kind variable bindings, where t is guaranteed
 * not to be a variable or to have a synonym as outermost constructor */
static bool kvarToTypeBind(Tyvar *tyv, Type t, int o) {
    if (doesntOccurIn(tyv, t, o)) {
        tyv->bound = t;
        tyv->offs = o;
        return true;
    }
    unifyFails = "unification would give infinite kind";
    return false;
}

bool kunify(Kind k1, int o1, Kind k2, int o2) {
    assertKind(k1);
    assertKind(k2);
    Tyvar *kyv1, *kyv2;

    kyv1 = deRef(&k1, &o1);
    kyv2 = deRef(&k2, &o2);

    if (kyv1)
        if (kyv2)
            return kvarToVarBind(kyv1, kyv2); /* k1, k2 variables    */
        else
            return kvarToTypeBind(kyv1, k2, o2); /* k1 variable, k2 not */
    else if (kyv2)
        return kvarToTypeBind(kyv2, k1, o1); /* k2 variable, k1 not */
    else {
        if (!isAp(k1) && !isAp(k2)) /* k1, k2 not vars: STAR or   */
            return k1 == k2;        /* CONSTRAINT                 */
        else if (isAp(k1) && isAp(k2))
            return kunify(fst(k1), o1, fst(k2), o2) &&
                   kunify(snd(k1), o1, snd(k2), o2);
    }
    unifyFails = 0;
    return false;
}

/* --------------------------------------------------------------------------
 * Tuple type constructors: are generated as necessary.  The most common
 * n-tuple constructors (n<MAXTUPCON) are held in a cache to avoid
 * repeated generation of the constructor types.
 * ------------------------------------------------------------------------*/

#define MAXTUPCON 10
static Type tupleConTypes[MAXTUPCON];

/* The type of tuple constructor e at fresh variables, using tupleConTypes
 * to cache previously calculated tuple constr. types. */
Typing typeTuple(Cell e) {
    int n = tupleOf(e);
    int offs = newTyvars(n);
    if (n >= MAXTUPCON)
        return typing(makeTupleType(n), offs);
    if (isNull(tupleConTypes[n]))
        tupleConTypes[n] = makeTupleType(n);
    return typing(tupleConTypes[n], offs);
}

/* Construct type for tuple constr. t1 -> ... -> tn -> (t1,...,tn) */
static Type makeTupleType(int n) {
    Type h = mkTuple(n);
    int i;

    for (i = 0; i < n; ++i)
        h = ap(h, mkOffset(i));
    while (0 < n--)
        h = fn(mkOffset(n), h);
    return h;
}

/* --------------------------------------------------------------------------
 * Two forms of kind expression are used quite frequently:
 *      *  -> *  -> ... -> *  -> *      for kinds of ->, [], (,) etc...
 *      v1 -> v2 -> ... -> vn -> vn+1   skeletons for constructor kinds
 * Expressions of these forms are produced by the following functions which
 * use a cache to avoid repeated construction of commonly used values.
 * ------------------------------------------------------------------------*/

#define MAXKINDFUN 10
static Kind simpleKindCache[MAXKINDFUN];
static Kind varKindCache[MAXKINDFUN];

/* Construct * -> ... -> * (n args) */
static Kind makeSimpleKind(int n) {
    Kind k = STAR;
    while (n-- > 0)
        k = ap(STAR, k);
    return k;
}

Kind simpleKind(int n) {
    if (n >= MAXKINDFUN)
        return makeSimpleKind(n);
    else if (nonNull(simpleKindCache[n]))
        return simpleKindCache[n];
    else if (n == 0)
        return simpleKindCache[0] = STAR;
    else
        return simpleKindCache[n] = ap(STAR, simpleKind(n - 1));
}

/* Construct v0 -> .. -> vn */
static Kind makeVarKind(int n) {
    Kind k = mkOffset(n);
    while (n-- > 0)
        k = ap(mkOffset(n), k);
    return k;
}

/* The (possibly cached) var function kind v0 -> .. -> vn at n+1 fresh
 * kind variables */
Typing varKind(int n) {
    int offs = newKindvars(n + 1);
    if (n >= MAXKINDFUN)
        return typing(makeVarKind(n), offs);
    if (isNull(varKindCache[n]))
        varKindCache[n] = makeVarKind(n);
    return typing(varKindCache[n], offs);
}

static Cell *const substRoots[] = {
    &btyvars,
    &genericVars,
};

void resetSubstitution(void) {
    emptySubstitution();
    unrestrictBind();
    btyvars = NIL;
}

/* Mark the tuple-constructor and kind caches, and every bound type
 * variable; the rest of the module's roots are in substRoots. */
void markSubstitution(void) {
    int i;

    for (i = 0; i < MAXTUPCON; ++i)
        mark(tupleConTypes[i]);
    for (i = 0; i < MAXKINDFUN; ++i) {
        mark(simpleKindCache[i]);
        mark(varKindCache[i]);
    }
    for (i = 0; i < numTyvars; ++i)
        mark(tyvars[i].bound);
}

void installSubstitution(void) {
    int i;

    registerRoots(substRoots);
    resetSubstitution();
    for (i = 0; i < MAXTUPCON; ++i)
        tupleConTypes[i] = NIL;
    for (i = 0; i < MAXKINDFUN; ++i) {
        simpleKindCache[i] = NIL;
        varKindCache[i] = NIL;
    }
}

void exitSubstitution(void) {
    free(tyvars);
}

/*-------------------------------------------------------------------------*/
