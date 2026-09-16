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
int typeOff;        /* offset of result type       */
Type typeIs;        /* skeleton of result type     */
int typeFree;       /* freedom in instantiated type        */
List predsAre;      /* list of predicates in type          */
List genericVars;   /* list of generic vars                */
List btyvars = NIL; /* explicitly scoped type vars         */

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

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
static void expandSynFully(Type *, int *);

static bool improveAgainst(int, List, Cell, int);
static bool instImprove(int, Class, Cell, int);
static bool pairImprove(int, Class, Cell, int, Cell, int, int);

static bool kvarToVarBind(Tyvar *, Tyvar *);
static bool kvarToTypeBind(Tyvar *, Type, int);

/* --------------------------------------------------------------------------
 * The substitution, types, and kinds:
 *
 * In early versions of Gofer, the `substitution' data structure was only
 * used by the type checker, so it made sense to include support for it in
 * type.c.  This changed when kinds and kind inference where introduced,
 * which required access to the substitution during static analysis.  The
 * links between type.c and static.c that were intially used to accomplish
 * this have now been avoided by making the substitution visible as an
 * independent data structure in storage.c.
 *
 * In the same way that values have types, type constructors (and more
 * generally, expressions built from such constructors) have kinds.
 * The syntax of kinds in the current implementation is very simple:
 *
 *        kind ::= STAR         -- the kind of types
 *              |  kind => kind -- constructors
 *              |  variables    -- either INTCELL or OFFSET
 *
 * For various reasons, this implementation uses structure sharing, instead
 * of a copying approach.  In principal, this is fast and avoids the need to
 * build new type expressions.  Unfortunately, this implementation will not
 * be able to handle *very* large expressions.
 *
 * The substitution is represented by an array of type variables each of
 * which is a triple:
 *      bound   a (skeletal) type expression, or NIL if the variable
 *              is not bound, or SKOLEM for a Skolem constant (i.e., an
 *              uninstantiable variable).
 *      offs    offset of skeleton in bound.  If isNull(bound), then offs is
 *              used to indicate whether that variable is generic (i.e. free
 *              in the current assumption set) or fixed (i.e. bound in the
 *              current assumption set).  Generic variables are assigned
 *              offset numbers whilst copying type expressions (t,o) to
 *              obtain their most general form.
 *      kind    kind of value bound to type variable (`type variable' is
 *              rather inaccurate -- `constructor variable' would be better).
 * ------------------------------------------------------------------------*/

/* Clear current substitution */
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
    typeIs = NIL;
    predsAre = NIL;
    btyvars = NIL;
}

/* Add further n type variables to current substitution */
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

        /* It would be better to realloc() here, but that isn't portable
         * enough for calloc()ed arrays.  The following code could cause
         * a space leak if an interrupt occurs while we're copying the
         * array ... we won't worry about this for the time being because
         * we don't expect to have to go through this process much (if at
         * all) in normal use of the type checker.
         */

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
        maxTyvars = 0; /* protection from SIGINT? */
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

/* Allocate new variables with specified kinds */
int newKindedVars(Kind k) {
    assertKinds(k);
    int beta = numTyvars;           /* if k = k0 -> k1 -> ... -> kn        */
    for (; isPair(k); k = snd(k)) { /* then allocate n vars with kinds */
        expandSubst(1);             /* k0, k1, ..., k(n-1)                 */
        tyvars[numTyvars].offs = UNUSED_GENERIC;
        tyvars[numTyvars].bound = NIL;
        tyvars[numTyvars].kind = fst(k);
        numTyvars++;
    }
    return beta;
}

/* Instantiate type, if nonNull */
void instantiate(Type type) {
    if (nonNull(type))
        assertType(type);
    predsAre = NIL;
    typeIs = type;
    typeFree = 0;

    if (nonNull(typeIs)) { /* instantiate type expression ?    */

        if (isPolyType(typeIs)) { /* Polymorphic type scheme ?     */
            Kinds ks = polySigOf(typeIs);
            typeOff = newKindedVars(ks);
            typeIs = monotypeOf(typeIs);
            for (; isAp(ks); ks = arg(ks))
                typeFree++;
        }

        if (isQualType(typeIs)) { /* Qualified type?                       */
            predsAre = fst(snd(typeIs));
            typeIs = snd(snd(typeIs));
        }
    }
}

/* --------------------------------------------------------------------------
 * Bound type variables:
 * ------------------------------------------------------------------------*/

/* Look for bound tyvar */
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

/* Mark explicitly scoped vars */
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
 * Dereference or bind types in subsitution:
 * ------------------------------------------------------------------------*/

/* Get number of type variable represented by (t,o) [if any]. */
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

/* Load type held in type variable vn into (typeIs,typeOff) */
void tyvarType(int vn) {
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
            typeIs = tyv->bound;
            typeOff = tyv->offs;
            return;
        }
    typeIs = aVar;
    typeOff = vn;
}

/* Set type variable vn to (t,o) */
void bindTv(int vn, Type t, int o) {
    Tyvar *tyv = tyvar(vn);
    tyv->bound = t;
    tyv->offs = o;
}

/* Get value at head of type exp. */
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

/* Expand type synonym with: head h ar args (NB. ar>=tyconRec(h)->arity)
 * original expression (*at,*ao) expansion returned in (*at,*ao) */
void expandSyn(Tycon h, int ar, Type *at, int *ao) {
    ar -= tyconRec(h)->arity; /* calculate surplus arguments           */
    if (ar == 0)
        expandSyn1(h, at, ao);
    else {               /* if there are more args than the */
        Type t = *at;    /* arity, we have to do a little   */
        int o = *ao;     /* bit of work to isolate args that*/
        Type args = NIL; /* will not be changed by expansion*/
        int i;
        while (ar-- > 0) {             /* find part to expand, and the     */
            args = cons(arg(t), args); /* unused arguments                 */
            t = fun(t);
            deRef(&t, &o);
        }
        expandSyn1(h, &t, &o);            /* do the expansion              */
        bindTv((i = newTyvars(1)), t, o); /* and embed the results back in   */
        tyvar(i)->kind = getKind(t, o);   /* (*at, *ao) as required        */
        *at = applyToArgs(mkInt(i), args);
    }
}

/* Expand type synonym with: head h, tyconRec(h)->arity args, original
 * expression (*at,*ao) expansion returned in (*at,*ao) */
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

/* Reset all generic vars to unused */
void resetGenerics(void) {
    int i;
    for (i = 0; i < numTyvars; ++i)
        if (!isBound(tyvar(i)) && tyvar(i)->offs >= GENERIC)
            tyvar(i)->offs = UNUSED_GENERIC;
    genericVars = NIL;
    nextGeneric = 0;
}

/* Mark fixed vars in type bound to given type variable */
void markTyvar(int vn) {
    Tyvar *tyv = tyvar(vn);

    if (isBound(tyv))
        markType(tyv->bound, tyv->offs);
    else
        (tyv->offs) = FIXED_TYVAR;
}

/* Mark fixed vars in type (t,o) */
void markType(Type t, int o) {
    stackCheck();
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

/* Marked fixed type vars in pi */
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

/* Copy single predicate (or part thereof) ... */
Cell copyPred(Cell pi, int o) {
    if (isAp(pi)) {
        Cell temp = copyPred(fun(pi), o); /* to ensure correct order of eval.*/
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

/* Flatten type by chasing all references and collapsing OFFSETS to absolute
 * indexes */
Type zonkType(Type t, int o) {
    stackCheck();
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

/* Build kind attatched to variable */
Kind copyKindvar(int vn) {
    Tyvar *tyv = tyvar(vn);
    if (tyv->bound)
        return copyKind(tyv->bound, tyv->offs);
    return STAR; /* any unbound variable defaults to*/
} /* the kind of all types         */

/* Build kind expression from given skeleton */
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
 * Droping and lifting of type schemes that appear in rank 2 position:
 * ------------------------------------------------------------------------*/

/* Drop a (potentially) rank2 type */
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

/* Find kind of constr during type checking process */
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

/* Calculate list of generic vars in type expression (t,o) results are
 * prepended to vs */
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

/* Return true if var lookFor isn't referenced in (t,o) */
bool doesntOccurIn(Tyvar *lookFor, Type t, int o) {
    Tyvar *tyv;

    stackCheck();
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

/* Make binding tyv1 := tyv2 */
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

/* Make binding tyv := (t,o) guaranteed not to be a v'ble or have synonym as
 * outermost constr */
static bool varToTypeBind(Tyvar *tyv, Type t, int o) {
    if (tyvNum(tyv) < bindAbove) { /* Check that tyv is in range       */
        unifyFails = "types do not match";
        return false;
    } else if (tyv->bound == SKOLEM) { /* Check that it is not Skolemized */
        unifyFails = "cannot instantiate Skolem constant";
        return false;
    } else if (!doesntOccurIn(tyv, t, o)) /* Carry out occurs check        */
        unifyFails = "unification would give infinite type";
    else if (!eqKind(tyv->kind, getKind(t, o)))
        unifyFails = "kinds do not match";
    else {
        tyv->bound = t;
        tyv->offs = o;
        return true;
    }
    return false;
}

/* Main unification routine unify (t1,o1) with (t2,o2) */
bool unify(Type t1, int o1, Type t2, int o2) {
    assertType(t1);
    assertType(t2);
    Tyvar *tyv1, *tyv2;

    stackCheck();
    tyv1 = deRef(&t1, &o1);
    tyv2 = deRef(&t2, &o2);

un:
    if (tyv1)
        if (tyv2)
            return varToVarBind(tyv1, tyv2); /* t1, t2 variables    */
        else {
            Cell h2 = getDerefHead(t2, o2); /* t1 variable, t2 not */
            if (isSynonym(h2) && argCount >= tyconRec(h2)->arity) {
                expandSyn(h2, argCount, &t2, &o2); /* may grow (and re-   */
                tyv1 = deRef(&t1, &o1);            /* alloc) tyvars, so   */
                tyv2 = deRef(&t2, &o2);            /* re-derive tyv1 too  */
                goto un;
            }
            return varToTypeBind(tyv1, t2, o2);
        }
    else if (tyv2) {
        Cell h1 = getDerefHead(t1, o1); /* t2 variable, t1 not */
        if (isSynonym(h1) && argCount >= tyconRec(h1)->arity) {
            expandSyn(h1, argCount, &t1, &o1); /* may grow (and re-   */
            tyv1 = deRef(&t1, &o1);            /* alloc) tyvars, so   */
            tyv2 = deRef(&t2, &o2);            /* re-derive tyv2 too  */
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

        if (nonNull(h1) && h1 == h2) { /* Assuming well-formed types, both*/
            if (a1 != a2) {            /* t1, t2 must have same no of args*/
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

        if ((isNull(h1) && a1 <= a2) || /* last attempt -- maybe   */
            (isNull(h2) && a2 <= a1)) { /* one head is a variable? */
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

/* Repeatedly expand synonyms original expression (*at,*ao) expansion returned
 * in (*at,*ao) */
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
        return 0; /* ZZ what's the correct name for the empty Cell? */
    /* see below as well... */
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
 *
 * Another situation in which both one-way and two-way unification might
 * be used is in an implementation of improvement.  Here, a one-way match
 * would be used to determine applicability of a rule for improvement
 * that would then be followed by unification with another predicate.
 * One possible syntax for this might be:
 *
 *     instance P => pi [improves pi'] where ...
 *
 * The intention here is that any predicate matching pi' can be unified
 * with pi to get more accurate types.  A simple example of this is:
 *
 *   instance Collection [a] a improves Collection [a] b where ...
 *
 * As soon as we know what the collection type is (in this case, a list),
 * we will also know what the element type is.  To ensure that the rule
 * for improvement is valid, the compilation system will also need to use
 * a one-way matching process to ensure that pi is a (substitution) instance
 * of pi'.  Another extension would be to allow more than one predicate pi'
 * in an improving rule.  Read the paper on simplification and improvement
 * for technical background.  Watch this space for implementation news!
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
 * pred to be bound. Assumes preds are kind correct with the same class and
 * that no */
bool matchPred(Cell pi1, int o1, Cell pi, int o) {
    assertPred(pi1);
    assertPred(pi);
    bool result; /* vars have been alloc'd since o. */
    bindOnlyAbove(o);
    result = unifyPred(pi1, o1, pi, o);
    unrestrictBind();
    return result;
}

/* Unify two predicates Assumes preds are kind correct with the same class. */
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
 * found, then tyvars from typeOff have been initialized to allow direct use of
 * specifics. */
Inst findInstFor(Cell pi, int o) {
    assertPred(pi);
    Class c = getHead(pi);
    List ins;
    Cell kspi = NIL;

    if (!isClass(c))
        return NIL;

    for (ins = cclassRec(c)->instances; nonNull(ins); ins = tl(ins)) {
        Inst in = hd(ins);
        int beta = newKindedVars(instRec(in)->kinds);
        if (matchPred(pi, o, instRec(in)->head, beta)) {
            typeOff = beta;
            return in;
        } else {
            numTyvars = beta;
            if (allowOverlap && !allowUnsafeOverlap) {
                int alpha = newKindedVars(instRec(in)->kinds);
                if (isNull(kspi)) {
                    kspi = copyNoMark(pi, o);
                }
                beta = newKindedVars(fst(kspi));
                if (matchPred(instRec(in)->head, alpha, snd(kspi), beta)) {
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

/* Improve a list of predicates */
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

/* Improve a single predicate */
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

/* should emulate findInsts behavior of shorting out if the
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

/* Test to see whether two type schemes are the same */
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

/* Match, allowing only vars allocated since `a' to be bound this is deeply
 * hacky, since it relies on careful use of the substitution stack */
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

/* Make binding tyv1 := tyv2 for kind variable bindings */
static bool kvarToVarBind(Tyvar *tyv1, Tyvar *tyv2) {
    if (tyv1 != tyv2) {
        tyv1->bound = aVar;
        tyv1->offs = tyvNum(tyv2);
    }
    return true;
}

/* Make binding tyv := (t,o) for kind variable bindings guaranteed not to be a
 * v'ble or have synonym as outermost constr */
static bool kvarToTypeBind(Tyvar *tyv, Type t, int o) {
    if (doesntOccurIn(tyv, t, o)) {
        tyv->bound = t;
        tyv->offs = o;
        return true;
    }
    unifyFails = "unification would give infinite kind";
    return false;
}

/* Unify kind expr (k1,o1) with (k2,o2) */
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
        if (k1 == STAR && k2 == STAR) /* k1, k2 not vars           */
            return true;
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

/* Find type for tuple constr, using tupleConTypes to cache previously
 * calculated tuple constr. types. */
void typeTuple(Cell e) {
    int n = tupleOf(e);
    typeOff = newTyvars(n);
    if (n >= MAXTUPCON)
        typeIs = makeTupleType(n);
    else if (tupleConTypes[n])
        typeIs = tupleConTypes[n];
    else
        typeIs = tupleConTypes[n] = makeTupleType(n);
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
 *      *  -> *  -> ... -> *  -> *      for kinds of ->, [], ->, (,) etc...
 *      v1 -> v2 -> ... -> vn -> vn+1   skeletons for constructor kinds
 * Expressions of these forms are produced by the following functions which
 * use a cache to avoid repeated construction of commonly used values.
 * A similar approach is used to store the types of tuple constructors in the
 * main type checker.
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

/* Return (possibly cached) simple function kind */
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

/* Return (possibly cached) var function kind */
void varKind(int n) {
    typeOff = newKindvars(n + 1);
    if (n >= MAXKINDFUN)
        typeIs = makeVarKind(n);
    else if (nonNull(varKindCache[n]))
        typeIs = varKindCache[n];
    else
        typeIs = varKindCache[n] = makeVarKind(n);
}

/* --------------------------------------------------------------------------
 * Substitutution control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const substRoots[] = {
    &btyvars,
    &typeIs,
    &predsAre,
    &genericVars,
};

void substitution(int what) {
    int i;

    switch (what) {
    case RESET:
        emptySubstitution();
        unrestrictBind();
        btyvars = NIL;
        break;

    case MARK:
        for (i = 0; i < MAXTUPCON; ++i)
            mark(tupleConTypes[i]);
        for (i = 0; i < MAXKINDFUN; ++i) {
            mark(simpleKindCache[i]);
            mark(varKindCache[i]);
        }
        for (i = 0; i < numTyvars; ++i)
            mark(tyvars[i].bound);
        break;

    case INSTALL:
        registerRoots(substRoots);
        substitution(RESET);
        for (i = 0; i < MAXTUPCON; ++i)
            tupleConTypes[i] = NIL;
        for (i = 0; i < MAXKINDFUN; ++i) {
            simpleKindCache[i] = NIL;
            varKindCache[i] = NIL;
        }
        break;

    case EXIT:
        free(tyvars);
        break;
    default:
        break;
    }
}

/*-------------------------------------------------------------------------*/
