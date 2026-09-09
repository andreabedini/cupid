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
#include "output.h"
#include "subst.h"
#include "module.h"
#include "opts.h"
#include "kind.h"
#include "typeexp.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static List local maybeAppendVar(Cell, List);
static List local otvarsZonk(Cell, List, Int);
static Cell local checkTyconAmbig(Int, Text, Cell);
List tcDeps = NIL; /* list of dependent tycons/classes*/

/* --------------------------------------------------------------------------
 * Calculate set of variables appearing in a given type expression (possibly
 * qualified) as a list of distinct values.  The order in which variables
 * appear in the list is the same as the order in which those variables
 * occur in the type expression when read from left to right.
 * ------------------------------------------------------------------------*/

List typeVarsIn(Cell ty, List us, List ws,
                List vs) /*Calculate list of type variables*/
                         /* used in type expression, reading*/
                         /* from left to right ignoring any */
                         /* listed in us.             */
{                        /* ws = explicitly quantified vars */
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
        return typeVarsIn(monotypeOf(ty), polySigOf(ty), ws, vs);

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

static List local maybeAppendVar(Cell v,
                                 List vs) /* append variable to list if not   */
                                          /* already included              */
{
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
 * Static analysis for type expressions is required to:
 *   - ensure that each type constructor or class used has been defined.
 *   - replace type variables by offsets, constructor names by Tycons.
 *   - ensure that the type is well-kinded.
 * ------------------------------------------------------------------------*/

Type checkSigType(Int line, String where, Cell e, Type type)
/* Check validity of type expr in  */
/* explicit type signature         */
{
    List tvs = NIL;
    List sunk = NIL;
    List xtvs = NIL;

    if (isPolyType(type)) {
        xtvs = fst(snd(type));
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
            List ts = tvs;
            for (; nonNull(ts); ts = tl(ts)) {
                hd(ts) = NIL;
            }
            type = mkPolyType(tvs, type);
        }
    }

    unkindTypes = NIL;
    kindType(line, "type expression", type);
    fixKinds();
    unkindTypes = sunk;
    return type;
}

Void checkOptQuantVars(Int line, List xtvs, List tvs)
/* Explicitly quantified vars      */
{ /* Implicitly quantified vars    */
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

Type depTopType(Int l, List tvs, Type t) /* Check top-level of type sig     */
{
    Type prev = NIL;
    Type t1 = t;
    Int nr2 = 0;
    Int i = 1;
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

Type depCompType(Int l, List tvs, Type t) /* Check component type for constr */
{
    Int ntvs = length(tvs);
    List nfr = NIL;
    if (isPolyType(t)) {
        List vs = fst(snd(t));
        t = monotypeOf(t);
        tvs = checkQuantVars(l, vs, tvs, t);
        nfr = replicate(length(vs), NIL);
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

Type depTypeExp(Int line, List tyvars, Type type) {
    switch (whatIs(type)) {
    case AP:
        fst(type) = depTypeExp(line, tyvars, fst(type));
        snd(type) = depTypeExp(line, tyvars, snd(type));
        break;

    case VARIDCELL:
        return depTypeVar(line, tyvars, textOf(type));

    case QUALIDENT:
        if (isQVar(type)) {
            errHead(line);
            errPrintf("Qualified type variables not allowed");
            errFail();
        }
        /* deliberate fall through */
    case CONIDCELL: {
        Tycon tc = findQualTycon(type);
        if (isNull(tc)) {
            errHead(line);
            errPrintf("Undefined type constructor \"%s\"", identToStr(type));
            errFail();
        }
        if (whatIs(type) != QUALIDENT) {
            checkTyconAmbig(line, tyconRec(tc)->text, tc);
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

Type depTypeVar(Int line, List tyvars, Text tv) {
    Int offset = 0;
    Int found = (-1);

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

List checkQuantVars(Int line, List vs, List tvs, Cell body)
/* variables to quantify over      */
/* variables already in scope      */
{ /* type/constr for scope of vars   */
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

List offsetTyvarsIn(Type t, List vs) /* add list of offset tyvars in t  */
                                     /* to list vs                    */
{
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

List otvars(Cell pi, List os) /* os is a list of offsets that         */
                              /* refer to the arguments of pi;   */
{                             /* find list of offsets in those   */
    List us = NIL;            /* positions                    */
    for (; nonNull(os); os = tl(os)) {
        us = offsetTyvarsIn(nthArg(offsetOf(hd(os)), pi), us);
    }
    return us;
}

static List local otvarsZonk(Cell pi, List os,
                             Int o) /* same as above, but zonks    */
{
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

Bool osubset(List us, List vs) /* Determine whether us is subset  */
{                              /* of vs                        */
    while (nonNull(us) && cellIsMember(hd(us), vs)) {
        us = tl(us);
    }
    return isNull(us);
}

List oclose(List fds, List vs) /* Compute closure of vs wrt to fds*/
{
    Bool changed = TRUE;
    while (changed) {
        List fds1 = NIL;
        changed = FALSE;
        while (nonNull(fds)) {
            Cell fd = hd(fds);
            List next = tl(fds);
            if (osubset(fst(fd), vs)) { /* Test if fd applies              */
                List os = snd(fd);
                for (; nonNull(os); os = tl(os)) {
                    if (!cellIsMember(hd(os), vs)) {
                        vs = cons(hd(os), vs);
                        changed = TRUE;
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

Bool isAmbiguous(Type type) /* Determine whether type is           */
{                           /* ambiguous                           */
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
    return FALSE;
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
        Int o = intOf(snd3(pi3));
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
        }
    }
    return fds;
}

Void ambigError(Int line, String where, Cell e,
                Type type) /* produce error message for    */
                           /* ambiguity                    */
{
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

Cell depPredExp(Int line, List tyvars, Cell pred) {
    Int args = 0;
    Cell prev = NIL;
    Cell h = pred;
    for (; isAp(h); args++) {
        arg(h) = depTypeExp(line, tyvars, arg(h));
        prev = h;
        h = fun(h);
    }

    if (isQCon(h)) { /* standard class constraint          */
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
static Cell local checkTyconAmbig(Int line, Text t, Cell e) {
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

Void typeexpControl(Int what) {
    switch (what) {
    case RESET:
        tcDeps = NIL;
        break;

    case INSTALL:
        registerRoots(typeexpRoots);
        typeexpControl(RESET);
        break;
    }
}

/*-------------------------------------------------------------------------*/
