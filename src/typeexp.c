/* --------------------------------------------------------------------------
 * Static analysis of type expressions.
 *
 * Resolves the type variables of a type expression to offsets and its
 * constructors to tycons, records which tycons and classes a declaration
 * depends on, checks the type of a signature or a foreign declaration,
 * and decides whether a qualified type is ambiguous, which is also where
 * the functional dependency closure lives.
 *
 * Split out of static.c.
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
#include "subst.h"
#include "kind.h"
#include "typeexp.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static List maybeAppendVar(Cell, List);
static List otvarsZonk(Cell, List, int);
static Cell checkTyconAmbig(int, Text, Cell);
List tcDeps = NIL; /* list of dependent tycons/classes*/

/* --------------------------------------------------------------------------
 * Calculate set of variables appearing in a given type expression (possibly
 * qualified) as a list of distinct values.  The order in which variables
 * appear in the list is the same as the order in which those variables
 * occur in the type expression when read from left to right.
 * ------------------------------------------------------------------------*/

/* Calculate list of type variables used in type expression, reading from left
 * to right ignoring any listed in us. ws = explicitly quantified vars */
List typeVarsIn(Cell ty, List us, List ws, List vs) {
    switch (whatIs(ty)) {
    case AP:
        return typeVarsIn(snd(ty), us, ws, typeVarsIn(fst(ty), us, ws, vs));

    case VARIDCELL:
    case VAROPCELL:
        if ((nonNull(findBtyvs(textOf(ty))) && !varIsMember(textOf(ty), ws)) ||
            varIsMember(textOf(ty), us)) {
            return vs;
        } else {
            return maybeAppendVar(ty, vs);
        }

    case POLYTYPE:
        return typeVarsIn(monotypeOf(ty), boundVars(polySigOf(ty)), ws, vs);

    case QUAL: {
        vs = typeVarsIn(fst(snd(ty)), us, ws, vs);
        return typeVarsIn(snd(snd(ty)), us, ws, vs);
    }

    case BANG:
        return typeVarsIn(snd(ty), us, ws, vs);

    case LABC: {
        List fs = snd(snd(ty));
        for (; nonNull(fs); fs = tl(fs)) {
            vs = typeVarsIn(snd(hd(fs)), us, ws, vs);
        }
        return vs;
    }
    default:
        break;
    }
    return vs;
}

/* Append variable to list if not already included */
static List maybeAppendVar(Cell v, List vs) {
    Text t = textOf(v);
    List p = NIL;
    List c = vs;

    while (nonNull(c)) {
        if (textOf(hd(c)) == t) {
            return vs;
        }
        p = c;
        c = tl(c);
    }

    if (nonNull(p)) {
        tl(p) = cons(v, NIL);
    } else {
        vs = cons(v, NIL);
    }

    return vs;
}

/* --------------------------------------------------------------------------
 * The variables a forall binds:
 *
 * The parser keeps a variable given its kind, forall (m :: Type -> Type).
 * ..., as KINDED (m, kind) in the list of variables a POLYTYPE binds.  The
 * checks of a forall want the variables, and the kind checker wants the
 * kinds, each in the slot for its variable, or NIL for one inferred.
 * ------------------------------------------------------------------------*/

/* The variables the forall binders vs bind, without their kinds */
List boundVars(List vs) {
    List us = NIL;
    for (; nonNull(vs); vs = tl(vs)) {
        us = cons(whatIs(hd(vs)) == KINDED ? fst(snd(hd(vs))) : hd(vs), us);
    }
    return rev(us);
}

/* The kind slots for the variables tvs: the kind the binders bs give each,
 * or NIL */
List kindSlots(List tvs, List bs) {
    List ks = NIL;
    for (; nonNull(tvs); tvs = tl(tvs)) {
        Kind k = NIL;
        List us = bs;
        for (; nonNull(us); us = tl(us)) {
            if (whatIs(hd(us)) == KINDED &&
                textOf(fst(snd(hd(us)))) == textOf(hd(tvs))) {
                k = snd(snd(hd(us)));
            }
        }
        ks = cons(k, ks);
    }
    return rev(ks);
}

/* --------------------------------------------------------------------------
 * Polymorphic type synonyms:
 *
 *   type Lens s t a b = forall f. Functor f => (a -> f b) -> s -> f t
 *
 * The type checker expands a synonym where it meets one, in the middle of
 * unification, where a forall cannot be.  So a synonym for a polymorphic
 * type, or one with such a synonym in its body, is a POLYSYN, which the
 * type checker never expands; static analysis expands each use of it
 * before anything else looks at the type, and the type checker sees only
 * what it would have seen had the expansion been written out.
 *
 * Expansion replaces the synonym's parameters by its arguments and gives
 * the variables its foralls bind fresh names, with their kinds.  An
 * argument is expanded where the body puts it, so it may be polymorphic if
 * that is a place for one, as in GenericQ GenericT, which is forall a b.
 * (Data a, Data b) => a -> b -> b; an ordinary synonym given such an
 * argument is expanded here too.  A forall
 * that expansion leaves to the right of an arrow moves, with its context,
 * to the front of the type around it, as in GHC before 9.0:
 *
 *   lens :: (s -> a) -> (s -> b -> t) -> Lens s t a b
 *   lens :: Functor f => (s -> a) -> (s -> b -> t) -> (a -> f b) -> s -> f t
 *
 * A polymorphic type may then be where the type checker takes one: at the
 * front of a signature, of a function argument in a signature (rank 2),
 * or of a constructor field.  One anywhere else, as a type constructor's
 * argument or an argument's argument, is an error.  The expansion of a
 * forall written out is checked the same way, so these rules hold for
 * every signature.
 * ------------------------------------------------------------------------*/

typedef enum {
    PLACE_MONO, /* a monotype: no forall or context in it               */
    PLACE_ARG,  /* a function argument or a field: a forall at its      */
                /* front, and monotypes as its own arguments            */
    PLACE_TOP   /* a signature: a forall at its front, and arguments    */
                /* that may be polymorphic                              */
} Place;

static bool polySynsAllowed = false; /* resolving a synonym's own body */

static Type expandPolys(int, Type, Place);
static Type expandArgs(int, Type, Place);
static Type expandPolySyn(int, Tycon, Type);
static bool hasPolyArg(Type);
static bool isPolyArg(Type);
static Type fromSynBody(Type, List *);
static Type copyTypeSyntax(Type);
static Type prenex(List, List, Type);
static void monoOnly(int, Type, char *);

/* The type t, as the parser built it, at place p, with its polymorphic
 * synonyms expanded and its foralls moved to the front */
static Type expandPolys(int line, Type t, Place p) {
    Cell h;
    int n;
    Tycon tc = NIL;

    switch (whatIs(t)) {
    case POLYTYPE:
    case QUAL:
        if (p == PLACE_MONO) {
            errHead(line);
            errPrintf("A forall or a context cannot be here: a polymorphic "
                      "type may be the whole of a signature, an argument of "
                      "a function in one, or a constructor's field");
            errFail();
        }
        if (isPolyType(t)) {
            return prenex(polySigOf(t), NIL,
                          expandPolys(line, monotypeOf(t), p));
        } else {
            List ps = NIL;
            List qs = fst(snd(t));
            for (; nonNull(qs); qs = tl(qs)) {
                ps = cons(expandArgs(line, hd(qs), PLACE_MONO), ps);
            }
            return prenex(NIL, rev(ps), expandPolys(line, snd(snd(t)), p));
        }
    case BANG:
        return ap(BANG, expandPolys(line, snd(t), p));
    default:
        break;
    }

    h = getHead(t);
    n = argCount;
    if (h == typeArrow && n == 2) { /* a function: a forall in its result */
        Type a = expandPolys(line, arg(fun(t)), /* moves to its front */
                             p == PLACE_TOP ? PLACE_ARG : PLACE_MONO);
        Type r = expandPolys(line, arg(t), p);
        List vs = NIL;
        List ps = NIL;
        if (isPolyType(r)) {
            vs = polySigOf(r);
            r = monotypeOf(r);
        }
        if (isQualType(r)) {
            ps = fst(snd(r));
            r = snd(snd(r));
        }
        return prenex(vs, ps, fn(a, r));
    }
    if (isQCon(h)) {
        tc = findQualTycon(h);
    } else if (isTycon(h)) { /* from the body of a synonym expanded */
        tc = h;
    }
    if (nonNull(tc) &&
        (isPolySyn(tc) || /* or a synonym given a polymorphic type */
         (isSynonym(tc) && n >= tyconRec(tc)->arity && hasPolyArg(t)))) {
        if (isQCon(h) && whatIs(h) != QUALIDENT) {
            checkTyconAmbig(line, tyconRec(tc)->text, tc);
        }
        return expandPolys(line, expandPolySyn(line, tc, t), p);
    }
    return expandArgs(line, t, PLACE_MONO);
}

/* Is an argument of the application t, as the parser built it, a
 * polymorphic type, or one with a polymorphic synonym in it? */
static bool hasPolyArg(Type t) {
    for (; isAp(t); t = fun(t)) {
        if (isPolyArg(arg(t))) {
            return true;
        }
    }
    return false;
}

static bool isPolyArg(Type t) {
    Cell h;
    if (isPolyOrQualType(t)) {
        return true;
    }
    h = getHead(t);
    if (isQCon(h)) {
        h = findQualTycon(h);
    }
    return isPolySyn(h) || hasPolyArg(t);
}

/* The application t with each argument expanded at place p */
static Type expandArgs(int line, Type t, Place p) {
    if (isAp(t)) {
        Type l = expandArgs(line, fun(t), p);
        return ap(l, expandPolys(line, arg(t), p));
    }
    return t;
}

/* The expansion of t, an application of the synonym tc */
static Type expandPolySyn(int line, Tycon tc, Type t) {
    int arity = tyconRec(tc)->arity;
    List args = NIL;
    List env = NIL;
    Type u = t;
    Type e;
    int i;

    for (; isAp(u); u = fun(u)) { /* expanded where the body puts them */
        args = cons(arg(u), args);
    }
    if (length(args) < arity) {
        errHead(line);
        errPrintf("Not enough arguments for type synonym \"%s\"",
                  textToStr(tyconRec(tc)->text));
        errFail();
    }
    if (cellIsMember(tc, tyconDefns) && !cellIsMember(tc, tcDeps)) {
        tcDeps = cons(tc, tcDeps);
    }
    for (i = 0, u = args; i < arity; i++, u = tl(u)) {
        env = cons(hd(u), env);
    }
    env = rev(env);
    e = fromSynBody(tyconRec(tc)->defn, &env);
    for (; nonNull(u); u = tl(u)) { /* arguments beyond its parameters */
        e = ap(e, hd(u));
    }
    return e;
}

/* The body d of a synonym, as static analysis left it, made back into a
 * type as the parser builds one: each offset is the type at its position in
 * env, the synonym's arguments and then the variables of the foralls
 * around it, which get fresh names */
static Type fromSynBody(Type d, List *env) {
    switch (whatIs(d)) {
    case AP: {
        Type l = fromSynBody(fun(d), env);
        return ap(l, fromSynBody(arg(d), env));
    }
    case OFFSET:
        return copyTypeSyntax(nth(offsetOf(d), *env));
    case POLYTYPE: {
        int n = length(*env);
        List bs = NIL;
        List ks = polySigOf(d);
        Type b;
        for (; isAp(ks); ks = tl(ks)) {
            Cell v = inventVar();
            bs = cons(nonNull(hd(ks)) ? ap(KINDED, pair(v, hd(ks))) : v, bs);
            *env = appendOnto(*env, singleton(v));
        }
        b = fromSynBody(monotypeOf(d), env);
        *env = take(n, *env);
        return ap(POLYTYPE, pair(rev(bs), b));
    }
    case QUAL: {
        List ps = NIL;
        List qs = fst(snd(d));
        for (; nonNull(qs); qs = tl(qs)) {
            ps = cons(fromSynBody(hd(qs), env), ps);
        }
        return ap(QUAL, pair(rev(ps), fromSynBody(snd(snd(d)), env)));
    }
    case RANK2: /* worked out again where it is used */
        return fromSynBody(snd(snd(d)), env);
    default:
        return d;
    }
}

/* A copy of the type t as the parser builds one, for a second place that
 * static analysis may overwrite separately */
static Type copyTypeSyntax(Type t) {
    switch (whatIs(t)) {
    case AP: {
        Type l = copyTypeSyntax(fun(t));
        return ap(l, copyTypeSyntax(arg(t)));
    }
    case POLYTYPE:
        return ap(POLYTYPE,
                  pair(dupList(polySigOf(t)), copyTypeSyntax(monotypeOf(t))));
    case QUAL: {
        List ps = NIL;
        List qs = fst(snd(t));
        for (; nonNull(qs); qs = tl(qs)) {
            ps = cons(copyTypeSyntax(hd(qs)), ps);
        }
        return ap(QUAL, pair(rev(ps), copyTypeSyntax(snd(snd(t)))));
    }
    default:
        return t;
    }
}

/* The type forall vs. ps => b, joined with the forall and context at the
 * front of b, if any */
static Type prenex(List vs, List ps, Type b) {
    if (isPolyType(b)) {
        vs = appendOnto(dupList(vs), polySigOf(b));
        b = monotypeOf(b);
    }
    if (isQualType(b)) {
        ps = appendOnto(dupList(ps), fst(snd(b)));
        b = snd(snd(b));
    }
    if (nonNull(ps)) {
        b = ap(QUAL, pair(ps, b));
    }
    if (nonNull(vs)) {
        b = ap(POLYTYPE, pair(vs, b));
    }
    return b;
}

/* The type t, a signature as the parser built it, or a constructor's field
 * if field, with its polymorphic synonyms expanded and each forall moved
 * to the front of the signature, argument or field it is in */
Type expandPolyTypes(int line, Type t, bool field) {
    return expandPolys(line, t, field ? PLACE_ARG : PLACE_TOP);
}

/* Check that t, part of a synonym's body, has no forall or context in it */
static void monoOnly(int line, Type t, char *what) {
    if (isAp(t)) {
        monoOnly(line, fun(t), what);
        monoOnly(line, arg(t), what);
    } else if (isPolyOrQualType(t)) {
        errHead(line);
        errPrintf("The body of %s may have a forall and a context only at "
                  "its front",
                  what);
        errFail();
    }
}

/* Resolve rhs, the body of a type synonym with parameters tyvars.  One that
 * starts with a forall is the body of a POLYSYN, which may have no other
 * forall in it; any other may use the POLYSYNs in scope. */
Type depSynonymBody(int line, List tyvars, Type rhs) {
    bool saved = polySynsAllowed;
    polySynsAllowed = true;
    if (isPolyType(rhs)) {
        Type b = monotypeOf(rhs);
        if (isQualType(b)) {
            List ps = fst(snd(b));
            for (; nonNull(ps); ps = tl(ps)) {
                monoOnly(line, hd(ps), "a type synonym");
            }
            b = snd(snd(b));
        }
        monoOnly(line, b, "a type synonym");
        rhs = depCompType(line, tyvars, rhs);
    } else {
        rhs = depTopType(line, tyvars, rhs);
    }
    polySynsAllowed = saved;
    return rhs;
}

/* --------------------------------------------------------------------------
 * Static analysis for type expressions is required to:
 *   - ensure that each type constructor or class used has been defined.
 *   - replace type variables by offsets, constructor names by Tycons.
 *   - ensure that the type is well-kinded.
 * ------------------------------------------------------------------------*/

/* Check validity of type expr in explicit type signature */
Type checkSigType(int line, char *where, Cell e, Type type) {
    List tvs = NIL;
    List sunk = NIL;
    List xtvs = NIL;
    List bs = NIL; /* the binders of the forall at the front, if any */
    bool explicit = isPolyType(type);

    type = expandPolys(line, type, PLACE_TOP);
    if (isPolyType(type)) { /* a forall expansion brought is implicit */
        bs = polySigOf(type);
        if (explicit) {
            xtvs = boundVars(bs);
        }
        type = monotypeOf(type);
    }
    tvs = typeVarsIn(type, NIL, xtvs, NIL);
    sunk = unkindTypes;
    checkOptQuantVars(line, xtvs, tvs);

    if (isQualType(type)) {
        map2Over(depPredExp, line, tvs, fst(snd(type)));
        snd(snd(type)) = depTopType(line, tvs, snd(snd(type)));

        if (isAmbiguous(type)) {
            ambigError(line, where, e, type);
        }
    } else {
        type = depTopType(line, tvs, type);
    }

    if (nonNull(tvs)) {
        if (length(tvs) >= NUM_OFFSETS) {
            errHead(line);
            errPrintf("Too many type variables in %s\n", where);
            errFail();
        } else {
            type = mkPolyType(kindSlots(tvs, bs), type);
        }
    }

    unkindTypes = NIL;
    kindType(line, "type expression", type);
    fixKinds();
    unkindTypes = sunk;
    return type;
}

/* Explicitly quantified vars Implicitly quantified vars */
void checkOptQuantVars(int line, List xtvs, List tvs) {
    if (nonNull(xtvs)) {
        List vs = tvs;
        for (; nonNull(vs); vs = tl(vs)) {
            if (!varIsMember(textOf(hd(vs)), xtvs)) {
                errHead(line);
                errPrintf("Quantifier does not mention type variable \"%s\"",
                          textToStr(textOf(hd(vs))));
                errFail();
            }
        }
        for (vs = xtvs; nonNull(vs); vs = tl(vs)) {
            if (!varIsMember(textOf(hd(vs)), tvs)) {
                errHead(line);
                errPrintf("Quantified type variable \"%s\" is not used",
                          textToStr(textOf(hd(vs))));
                errFail();
            }
            if (varIsMember(textOf(hd(vs)), tl(vs))) {
                errHead(line);
                errPrintf("Quantified type variable \"%s\" is repeated",
                          textToStr(textOf(hd(vs))));
                errFail();
            }
        }
    }
}

/* Check top-level of type sig */
Type depTopType(int l, List tvs, Type t) {
    Type prev = NIL;
    Type t1 = t;
    int nr2 = 0;
    int i = 1;
    for (; getHead(t1) == typeArrow && argCount == 2; ++i) {
        arg(fun(t1)) = depCompType(l, tvs, arg(fun(t1)));
        if (isPolyOrQualType(arg(fun(t1)))) {
            nr2 = i;
        }
        prev = t1;
        t1 = arg(t1);
    }
    if (nonNull(prev)) {
        arg(prev) = depTypeExp(l, tvs, t1);
    } else {
        t = depTypeExp(l, tvs, t1);
    }

    if (nr2 > 0) {
        t = ap(RANK2, pair(mkInt(nr2), t));
    }

    return t;
}

/* Check component type for constr */
Type depCompType(int l, List tvs, Type t) {
    int ntvs = length(tvs);
    List nfr = NIL;
    if (isPolyType(t)) {
        List bs = fst(snd(t));
        List vs = boundVars(bs);
        t = monotypeOf(t);
        tvs = checkQuantVars(l, vs, tvs, t);
        nfr = kindSlots(vs, bs);
    }

    if (isQualType(t)) {
        map2Over(depPredExp, l, tvs, fst(snd(t)));
        snd(snd(t)) = depTypeExp(l, tvs, snd(snd(t)));
        /* it's premature to judge ambiguity (e.g. given functional deps)
        if (isAmbiguous(t)) {
            ambigError(l,"type component",NIL,t);
        }
        */
    } else {
        t = depTypeExp(l, tvs, t);
    }

    if (isNull(nfr)) {
        return t;
    }

    take(ntvs, tvs);
    return mkPolyType(nfr, t);
}

Type depTypeExp(int line, List tyvars, Type type) {
    switch (whatIs(type)) {
    case AP:
        fst(type) = depTypeExp(line, tyvars, fst(type));
        snd(type) = depTypeExp(line, tyvars, snd(type));
        break;

    case VARIDCELL:
        return depTypeVar(line, tyvars, textOf(type));

    case KINDED: /* (a :: k) anywhere but a declaration head */
        errHead(line);
        errPrintf("Kind annotation on \"%s\": a kind may be given only to the "
                  "variables a declaration binds",
                  textToStr(textOf(fst(snd(type)))));
        errFail();

    case QUALIDENT:
        if (isQVar(type)) {
            errHead(line);
            errPrintf("Qualified type variables not allowed");
            errFail();
        }
        /* deliberate fall through */
    case CONIDCELL:
    case CONOPCELL: { /* a type operator, f :+: g */
        Tycon tc = findQualTycon(type);
        if (isNull(tc)) {
            errHead(line);
            errPrintf("Undefined type constructor \"%s\"", identToStr(type));
            errFail();
        }
        if (whatIs(type) != QUALIDENT) {
            checkTyconAmbig(line, tyconRec(tc)->text, tc);
        }
        if (isPolySyn(tc) && !polySynsAllowed) {
            errHead(line);
            errPrintf("The polymorphic type synonym \"%s\" cannot be used "
                      "here: it may be used in a signature or a constructor's "
                      "field",
                      textToStr(tyconRec(tc)->text));
            errFail();
        }
        if (cellIsMember(tc, tyconDefns) && !cellIsMember(tc, tcDeps)) {
            tcDeps = cons(tc, tcDeps);
        }
        return tc;
    }

    case TYCON:
    case TUPLE:
        break;

    default:
        internal("depTypeExp");
    }
    return type;
}

Type depTypeVar(int line, List tyvars, Text tv) {
    int offset = 0;
    int found = (-1);

    for (; nonNull(tyvars); offset++) {
        if (tv == textOf(hd(tyvars))) {
            found = offset;
        }
        tyvars = tl(tyvars);
    }
    if (found < 0) {
        Cell vt = findBtyvs(tv);
        if (nonNull(vt)) {
            return fst(vt);
        }
        errHead(line);
        errPrintf("Undefined type variable \"%s\"", textToStr(tv));
        errFail();
    }
    return mkOffset(found);
}

/* Variables to quantify over variables already in scope type/constr for scope
 * of vars */
List checkQuantVars(int line, List vs, List tvs, Cell body) {
    if (nonNull(vs)) {
        List bvs = typeVarsIn(body, NIL, NIL, NIL);
        List us = vs;
        for (; nonNull(us); us = tl(us)) {
            Text u = textOf(hd(us));
            if (varIsMember(u, tl(us))) {
                errHead(line);
                errPrintf("Repeated quantified variable %s", textToStr(u));
                errFail();
            }
            if (!varIsMember(u, bvs)) {
                errHead(line);
                errPrintf("Locally quantified variable %s is not used",
                          textToStr(u));
                errFail();
            }
        }
        tvs = appendOnto(tvs, vs);
    }
    return tvs;
}

/* --------------------------------------------------------------------------
 * Check for ambiguous types:
 * A type  Preds => type  is ambiguous if not (TV(P) `subset` TV(type))
 * ------------------------------------------------------------------------*/

/* Add list of offset tyvars in t to list vs */
List offsetTyvarsIn(Type t, List vs) {
    switch (whatIs(t)) {
    case AP:
        return offsetTyvarsIn(fun(t), offsetTyvarsIn(arg(t), vs));

    case OFFSET:
        if (cellIsMember(t, vs))
            return vs;
        else
            return cons(t, vs);

    case QUAL:
        return offsetTyvarsIn(snd(t), vs);

    case POLYTYPE:
        return offsetTyvarsIn(monotypeOf(t), vs);
        /* slightly inaccurate, but won't matter here      */

    case EXIST:
    case RANK2:
        return offsetTyvarsIn(snd(snd(t)), vs);

    default:
        return vs;
    }
}

List zonkTyvarsIn(Type t, List vs) {
    switch (whatIs(t)) {
    case AP:
        return zonkTyvarsIn(fun(t), zonkTyvarsIn(arg(t), vs));

    case INTCELL:
        if (cellIsMember(t, vs))
            return vs;
        else
            return cons(t, vs);

        /* this case will lead to a type error --
           much better than reporting an internal error ;-) */
        /* case OFFSET   : internal("zonkTyvarsIn"); */

    default:
        return vs;
    }
}

/* Os is a list of offsets that refer to the arguments of pi; find list of
 * offsets in those positions */
List otvars(Cell pi, List os) {
    List us = NIL;
    for (; nonNull(os); os = tl(os)) {
        us = offsetTyvarsIn(nthArg(offsetOf(hd(os)), pi), us);
    }
    return us;
}

/* Same as above, but zonks */
static List otvarsZonk(Cell pi, List os, int o) {
    List us = NIL;
    for (; nonNull(os); os = tl(os)) {
        Type t = zonkType(nthArg(offsetOf(hd(os)), pi), o);
        us = zonkTyvarsIn(t, us);
    }
    return us;
}

List odiff(List us, List vs) {
    while (nonNull(us) && cellIsMember(hd(us), vs)) {
        us = tl(us);
    }
    return us;
}

/* Determine whether us is subset of vs */
bool osubset(List us, List vs) {
    while (nonNull(us) && cellIsMember(hd(us), vs)) {
        us = tl(us);
    }
    return isNull(us);
}

/* Compute closure of vs wrt to fds */
List oclose(List fds, List vs) {
    bool changed = true;
    while (changed) {
        List fds1 = NIL;
        changed = false;
        while (nonNull(fds)) {
            Cell fd = hd(fds);
            List next = tl(fds);
            if (osubset(fst(fd), vs)) { /* Test if fd applies              */
                List os = snd(fd);
                for (; nonNull(os); os = tl(os)) {
                    if (!cellIsMember(hd(os), vs)) {
                        vs = cons(hd(os), vs);
                        changed = true;
                    }
                }
            } else { /* Didn't apply this time, so keep */
                tl(fds) = fds1;
                fds1 = fds;
            }
            fds = next;
        }
        fds = fds1;
    }
    return vs;
}

/* Determine whether type is ambiguous */
bool isAmbiguous(Type type) {
    if (isPolyType(type)) {
        type = monotypeOf(type);
    }
    if (isQualType(type)) {       /* only qualified types can be           */
        List ps = fst(snd(type)); /* ambiguous                     */
        List tvps = offsetTyvarsIn(ps, NIL);
        List tvts = offsetTyvarsIn(snd(snd(type)), NIL);
        List fds = calcFunDeps(ps);

        tvts = oclose(fds, tvts); /* Close tvts under fds                  */
        return !osubset(tvps, tvts);
    }
    return false;
}

/* The dependencies of an equality: each side determines the other.  us and
 * vs are the variables of its two sides. */
static List equalityFunDeps(List us, List vs, List fds) {
    return cons(pair(us, vs), cons(pair(vs, us), fds));
}

List calcFunDeps(List ps) {
    List fds = NIL;
    for (; nonNull(ps); ps = tl(ps)) { /* Calc functional dependencies     */
        Cell pi = hd(ps);
        Cell c = getHead(pi);
        if (isClass(c)) {
            List xfs = cclassRec(c)->xfds;
            for (; nonNull(xfs); xfs = tl(xfs)) {
                List fs = snd(hd(xfs));
                for (; nonNull(fs); fs = tl(fs)) {
                    fds = cons(
                        pair(otvars(pi, fst(hd(fs))), otvars(pi, snd(hd(fs)))),
                        fds);
                }
            }
        } else if (isEquality(pi)) {
            fds = equalityFunDeps(offsetTyvarsIn(arg(fun(pi)), NIL),
                                  offsetTyvarsIn(arg(pi), NIL), fds);
        }
    }
    return fds;
}

List calcFunDepsPreds(List ps) {
    List fds = NIL;
    for (; nonNull(ps); ps = tl(ps)) { /* Calc functional dependencies     */
        Cell pi3 = hd(ps);
        Cell pi = fst3(pi3);
        Cell c = getHead(pi);
        int o = intOf(snd3(pi3));
        if (isClass(c)) {
            List xfs = cclassRec(c)->xfds;
            for (; nonNull(xfs); xfs = tl(xfs)) {
                List fs = snd(hd(xfs));
                for (; nonNull(fs); fs = tl(fs)) {
                    fds = cons(pair(otvarsZonk(pi, fst(hd(fs)), o),
                                    otvarsZonk(pi, snd(hd(fs)), o)),
                               fds);
                }
            }
        } else if (isEquality(pi)) {
            fds = equalityFunDeps(zonkTyvarsIn(zonkType(arg(fun(pi)), o), NIL),
                                  zonkTyvarsIn(zonkType(arg(pi), o), NIL), fds);
        }
    }
    return fds;
}

/* Produce error message for ambiguity */
void ambigError(int line, char *where, Cell e, Type type) {
    errHead(line);
    errPrintf("Ambiguous type signature in %s", where);
    errPrintf("\n*** ambiguous type : ");
    errType(type);
    if (nonNull(e)) {
        errPrintf("\n*** assigned to    : ");
        errExpr(e);
    }
    errPrintf("\n");
    errFail();
}

Cell depPredExp(int line, List tyvars, Cell pred) {
    int args = 0;
    Cell prev = NIL;
    Cell h = pred;
    for (; isAp(h); args++) {
        arg(h) = depTypeExp(line, tyvars, arg(h));
        prev = h;
        h = fun(h);
    }

    if (h == EQUALITY) { /* an equality: its sides are resolved above */
        if (args != 2)
            internal("depPredExp");
    } else if (isClass(h)) { /* resolved: from a synonym's body */
        if (args != cclassRec(h)->arity) {
            internal("depPredExp");
        }
        if (cellIsMember(h, classDefns) && !cellIsMember(h, tcDeps)) {
            tcDeps = cons(h, tcDeps);
        }
    } else if (isQCon(h)) { /* standard class constraint          */
        Class c = findQualClass(h);
        if (isNull(c)) {
            errHead(line);
            errPrintf("Undefined class \"%s\"", identToStr(h));
            errFail();
        }
        if (!isQualIdent(h) && nonNull(cclassRec(c)->clashes)) {
            List ls = cclassRec(c)->clashes;
            errHead(line);
            errPrintf("Ambiguous class occurrence \"%s\"",
                      textToStr(cclassRec(c)->text));
            errPrintf("\n*** Could refer to: ");
            errPrintf("%s.%s ", textToStr(moduleRec(cclassRec(c)->mod)->text),
                      textToStr(cclassRec(c)->text));
            for (; nonNull(ls); ls = tl(ls)) {
                errPrintf("%s.%s ",
                          textToStr(moduleRec(cclassRec(hd(ls))->mod)->text),
                          textToStr(cclassRec(hd(ls))->text));
            }
            errPrintf("\n");
            errFail();
        }
        if (isNull(prev)) {
            pred = c;
        } else {
            fun(prev) = c;
        }
        if (args != cclassRec(c)->arity) {
            errHead(line);
            errPrintf("Wrong number of arguments for class \"%s\"",
                      textToStr(cclassRec(c)->text));
            errFail();
        }
        if (cellIsMember(c, classDefns) && !cellIsMember(c, tcDeps)) {
            tcDeps = cons(c, tcDeps);
        }
    } else {
        internal("depPredExp");
    }
    return pred;
}

/* A type constructor that several imports bring into scope unqualified is
 * ambiguous, and only an error until it is qualified. */
static Cell checkTyconAmbig(int line, Text t, Cell e) {
    Tycon tc;

    if (isTycon(e)) {
        tc = e;
    } else {
        tc = findTycon(t);
    }

    if (!isNull(tc) && nonNull(tyconRec(tc)->clashes)) {
        Text t = tyconRec(tc)->text;
        List ls = tyconRec(tc)->clashes;
        errHead(line);
        errPrintf("Ambiguous type constructor occurrence \"%s\"", textToStr(t));
        errPrintf("\n*** Could refer to: ");
        errPrintf("%s.%s ", textToStr(moduleRec(tyconRec(tc)->mod)->text),
                  textToStr(tyconRec(tc)->text));
        for (; nonNull(ls); ls = tl(ls)) {
            errPrintf("%s.%s ",
                      textToStr(moduleRec(tyconRec(hd(ls))->mod)->text),
                      textToStr(tyconRec(hd(ls))->text));
        }
        errPrintf("\n");
        errFail();
    }
    return e;
}

/* --------------------------------------------------------------------------
 * Control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const typeexpRoots[] = {
    &tcDeps,
};

/* Clear the tycons and classes the declaration being checked uses. */
void resetTypeexp(void) {
    tcDeps = NIL;
}

/* Register the roots and reset; done once, before the Prelude is read. */
void installTypeexp(void) {
    registerRoots(typeexpRoots);
    resetTypeexp();
}

/*-------------------------------------------------------------------------*/
