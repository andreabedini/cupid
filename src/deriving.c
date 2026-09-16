/* --------------------------------------------------------------------------
 * Derived instances: working out their contexts and generating their code.
 *
 * A "deriving" clause asks for an instance the programmer did not write.
 * This file decides what context such an instance needs, by iterating to a
 * fixed point over the requests, and then writes the bindings for it: Eq,
 * Ord, Enum, Ix, Show, Read and Bounded.  tycon.c collects the requests
 * and hands them over; nothing here is reached any other way.
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
#include "opts.h"
#include "kind.h"
#include "tycon.h"
#include "class.h"
#include "deriving.h"

static void deriveContexts(List);
static void addDerivImp(Inst);

static void addDerInst(int, Class, List, List, Type, int);
static void initDerInst(Inst);
static void calcInstPreds(Inst);
static void maybeAddPred(Cell, int, int, List);
static Cell copyAdj(Cell, int, int);
static void tidyDerInst(Inst);
static List getDiVars(int);
static Cell mkBind(char *, List);
static Cell mkVarAlts(int, Cell);
static List deriveEq(Tycon);
static Pair mkAltEq(int, List);
static List deriveOrd(Tycon);
static Pair mkAltOrd(int, List);
static List makeDPats2(Cell, int);
static List deriveEnum(Tycon);
static List deriveIx(Tycon);
static bool isEnumType(Tycon);
static List mkIxBinds(int, Cell, int);
static Cell prodRange(int, List, Cell, Cell, Cell);
static Cell prodIndex(int, List, Cell, Cell, Cell);
static Cell prodInRange(int, List, Cell, Cell, Cell);
static List deriveShow(Tycon);
static Cell mkAltShow(int, Cell, int);
static Cell showsPrecRhs(Cell, Cell, int);
static List deriveRead(Cell);
static Cell mkReadCon(Name, Cell, Cell);
static Cell mkReadPrefix(Cell);
static Cell mkReadInfix(Cell);
static Cell mkReadTuple(Cell);
static Cell mkReadRecord(Cell, List);
static List deriveBounded(Tycon);
static List mkBndBinds(int, Cell, int);

/* --------------------------------------------------------------------------
 * Process derived instance requests:
 * ------------------------------------------------------------------------*/

static List derivedInsts; /* list of derived instances     */

/* Verify derived instance request for tycon t, with explicit context p,
 * component types ts and named class ct */
void checkDerive(Tycon t, List p, List ts, Cell ct) {
    int line = tyconRec(t)->line;
    Class c = findQualClass(ct);
    if (isNull(c)) {
        errHead(line);
        errPrintf("Unknown class \"%s\" in derived instance", identToStr(ct));
        errFail();
    }
    addDerInst(line, c, p, dupList(ts), t, tyconRec(t)->arity);
}

/* Add a derived instance */
static void addDerInst(int line, Class c, List p, List cts, Type t, int a) {
    Inst in;
    Cell head = t; /* Build instance head          */
    int i = 0;

    for (; i < a; i++) {
        head = ap(head, mkOffset(i));
    }
    head = ap(c, head);

    in = newInst();
    instRec(in)->c = c;
    instRec(in)->line = line;
    instRec(in)->head = head;
    instRec(in)->specifics = ap(DERIVE, pair(dupList(p), cts));
    instRec(in)->implements = NIL;
    instRec(in)->kinds = mkInt(a);
    derivedInsts = cons(in, derivedInsts);
}

/* Request derived instance of c for mkTuple(n) constructor */
void addTupInst(Class c, int n) {
    int m = n;
    List cts = NIL;
    while (0 < m--) {
        cts = cons(mkOffset(m), cts);
    }
    cts = rev(cts);
    addDerInst(0, c, NIL, cts, mkTuple(n), n);
}

/* --------------------------------------------------------------------------
 * Calculation of contexts for derived instances:
 *
 * Allowing arbitrary types to appear in contexts makes it rather harder
 * to decide what the context for a derived instance should be.  For
 * example, given:
 *
 *    data T a = MkT [a] deriving Show,
 *
 * we could have either of the following:
 *
 *    instance (Show [a]) => Show (T a) where ...
 *    instance (Show a) => Show (T a) where ...
 *
 * (assuming, of course, that instance (Show a) => Show [a]).  For now, we
 * choose to reduce contexts in the hope of detecting errors at an earlier
 * stage---in contrast with value definitions, there is no way for a user
 * to provide something analogous to a `type signature' by which they might
 * be able to control this behaviour themselves.  We eliminate tautological
 * predicates, but only allow predicates to appear in the final result if
 * they have at least one argument with a variable at its head.
 *
 * In general, we have to deal with mutually recursive instance declarations.
 * We find a solution in the obvious way by iterating to find a fixed point.
 * Of course, without restrictions on the form of instance declarations, we
 * cannot be sure that this will always terminate!
 *
 * For each instance we maintain a pair of the form DERIVE (ctxt,ps).
 * Ctxt is a list giving the parts of the context that have been produced
 * so far in the form of predicate skeletons.  During the calculation of
 * derived instances, we attach a dummy NIL value to the end of the list
 * which acts as a kind of `variable': other parts of the system maintain
 * pointers to this variable, and use it to detect when the context has
 * been extended with new elements.  Meanwhile, ps is a list containing
 * predicates (pi,o) together with (delayed) substitutions of the form
 * (o,xs) where o is an offset and xs is one of the context variables
 * described above, which may have been partially instantiated.
 * ------------------------------------------------------------------------*/

static bool instsChanged;

/* Calc contexts for derived insts */
static void deriveContexts(List is) {
    emptySubstitution();
    mapProc(initDerInst, is); /* Prepare derived instances         */

    do { /* Main calculation of contexts           */
        instsChanged = false;
        mapProc(calcInstPreds, is);
    } while (instsChanged);

    mapProc(tidyDerInst, is); /* Tidy up results                   */
}

/* Prepare instance for calculation of derived instance context */
static void initDerInst(Inst in) {
    Cell spcs = instRec(in)->specifics;
    int beta = newKindedVars(instRec(in)->kinds);
    if (whatIs(spcs) != DERIVE) {
        internal("initDerInst");
    }
    fst(snd(spcs)) = appendOnto(fst(snd(spcs)), singleton(NIL));
    for (spcs = snd(snd(spcs)); nonNull(spcs); spcs = tl(spcs)) {
        hd(spcs) = ap2(instRec(in)->c, hd(spcs), mkInt(beta));
    }
    instRec(in)->numSpecifics = beta;
}

/* Calculate next approximation of the context for a derived instance */
static void calcInstPreds(Inst in) {
    List retain = NIL;
    List ps = snd(snd(instRec(in)->specifics));
    List spcs = fst(snd(instRec(in)->specifics));
    int beta = instRec(in)->numSpecifics;
    int its = 1;
    int factor = 1 + length(ps);

    while (nonNull(ps)) {
        Cell p = hd(ps);
        ps = tl(ps);

        if (its++ >= factor * cutoff) {
            Cell bpi = instRec(in)->head;
            errHead(instRec(in)->line);
            errPrintf("\n*** Cannot derive ");
            errPred(bpi);
            errPrintf(" after %d iterations.", its - 1);
            errPrintf(
                "\n*** This may indicate that the problem is undecidable.  However,\n");
            errPrintf(
                "*** you may still try to increase the cutoff limit using the -c\n");
            errPrintf(
                "*** option and then try again.  (The current setting is -c%d)\n",
                cutoff);
            errFail();
        }
        if (isInt(fst(p))) { /* Delayed substitution?   */
            List qs = snd(p);
            for (; nonNull(hd(qs)); qs = tl(qs)) {
                ps = cons(pair(hd(qs), fst(p)), ps);
            }
            retain = cons(pair(fst(p), qs), retain);
        } else { /* Class predicate        */
            Cell pi = fst(p);
            int o = intOf(snd(p));
            Inst in1 = findInstFor(pi, o);
            if (nonNull(in1)) {
                List qs = instRec(in1)->specifics;
                int off = mkInt(typeOff);
                if (whatIs(qs) == DERIVE) { /* Still being derived         */
                    for (qs = fst(snd(qs)); nonNull(hd(qs)); qs = tl(qs)) {
                        ps = cons(pair(hd(qs), off), ps);
                    }
                    retain = cons(pair(off, qs), retain);
                } else { /* Previously def'd inst   */
                    for (; nonNull(qs); qs = tl(qs)) {
                        ps = cons(pair(hd(qs), off), ps);
                    }
                }
            } else { /* No matching instance       */
                Cell qi = pi;
                while (isAp(qi) && isOffset(getDerefHead(arg(qi), o))) {
                    qi = fun(qi);
                }
                if (isAp(qi)) {
                    Cell bpi = instRec(in)->head;
                    pi = copyPred(pi, o);
                    errHead(instRec(in)->line);
                    errPrintf("An instance of ");
                    errPred(pi);
                    errPrintf(" is required to derive ");
                    errPred(bpi);
                    errPrintf("\n");
                    errFail();
                } else {
                    maybeAddPred(pi, o, beta, spcs);
                }
            }
        }
    }
    snd(snd(instRec(in)->specifics)) = retain;
}

/* Add predicate pi to the list ps, setting the instsChanged flag if pi is not
 * already a member and using beta to adjust vars */
static void maybeAddPred(Cell pi, int o, int beta, List ps) {
    Cell c = getHead(pi);
    for (; nonNull(ps); ps = tl(ps)) {
        if (isNull(hd(ps))) { /* reached the `dummy' end of list?*/
            hd(ps) = copyAdj(pi, o, beta);
            tl(ps) = pair(NIL, NIL);
            instsChanged = true;
            return;
        } else if (c == getHead(hd(ps)) && samePred(pi, o, hd(ps), beta)) {
            return;
        }
    }
}

/* Copy (c,o), replacing vars with offsets relative to beta. */
static Cell copyAdj(Cell c, int o, int beta) {
    switch (whatIs(c)) {
    case AP: {
        Cell l = copyAdj(fst(c), o, beta);
        Cell r = copyAdj(snd(c), o, beta);
        return ap(l, r);
    }

    case OFFSET: {
        int vn = o + offsetOf(c);
        Tyvar *tyv = tyvar(vn);
        if (isBound(tyv)) {
            return copyAdj(tyv->bound, tyv->offs, beta);
        }
        vn -= beta;
        if (vn < 0 || vn >= NUM_OFFSETS) {
            internal("copyAdj");
        }
        return mkOffset(vn);
    }
    default:
        break;
    }
    return c;
}

/* Tidy up results of derived inst calculations */
static void tidyDerInst(Inst in) {
    int o = instRec(in)->numSpecifics;
    List ps = tl(rev(fst(snd(instRec(in)->specifics))));
    clearMarks();
    copyPred(instRec(in)->head, o);
    instRec(in)->specifics = simpleContext(ps, o);
    instRec(in)->numSpecifics = length(instRec(in)->specifics);
}

/* --------------------------------------------------------------------------
 * Generate code for derived instances:
 * ------------------------------------------------------------------------*/

static void addDerivImp(Inst in) {
    List imp = NIL;
    Type t = getHead(arg(instRec(in)->head));
    Class c = instRec(in)->c;
    if (c == classEq) {
        imp = deriveEq(t);
    } else if (c == classOrd) {
        imp = deriveOrd(t);
    } else if (c == classEnum) {
        imp = deriveEnum(t);
    } else if (c == classIx) {
        imp = deriveIx(t);
    } else if (c == classShow) {
        imp = deriveShow(t);
    } else if (c == classRead) {
        imp = deriveRead(t);
    } else if (c == classBounded) {
        imp = deriveBounded(t);
    } else {
        errHead(instRec(in)->line);
        errPrintf("Cannot derive instances of class \"%s\"",
                  textToStr(cclassRec(instRec(in)->c)->text));
        errFail();
    }

    kindInst(in, intOf(instRec(in)->kinds));
    insertInst(in);
    instRec(in)->builder = newInstImp(in);
    instRec(in)->implements =
        classBindings("derived instance", instRec(in)->c, imp);
}

static List diVars = NIL; /* Acts as a cache of invented vars*/
static int diNum = 0;

/* Get list of at least n vars for derived instance generation */
static List getDiVars(int n) {
    for (; diNum < n; diNum++) {
        diVars = cons(inventVar(), diVars);
    }
    return diVars;
}

/* Make a binding for a variable */
static Cell mkBind(char *s, List alts) {
    return pair(mkVar(findText(s)), pair(NIL, alts));
}

/* Make alts for binding a var to a simple expression */
static Cell mkVarAlts(int line, Cell r) {
    return singleton(pair(NIL, pair(mkInt(line), r)));
}

/* --------------------------------------------------------------------------
 * Given a datatype:   data T a b = A a b | B Int | C  deriving (Eq, Ord)
 * The derived definitions of equality and ordering are given by:
 *
 *   A a b == A x y  =  a==x && b==y
 *   B a   == B x    =  a==x
 *   C     == C      =  True
 *   _     == _      =  False
 *
 *   compare (A a b) (A x y) =  primCompAux a x (compare b y)
 *   compare (B a)   (B x)   =  compare a x
 *   compare C       C       =  EQ
 *   compare a       x       =  cmpConstr a x
 *
 * In each case, the last line is only needed if there are multiple
 * constructors in the datatype definition.
 * ------------------------------------------------------------------------*/

#define ap2(f, x, y) ap(ap(f, x), y)

/* Generate binding for derived == for some TUPLE or DATATYPE t */
static List deriveEq(Type t) {
    List alts = NIL;
    if (isTycon(t)) { /* deal with type constrs    */
        List cs = tyconRec(t)->defn;
        for (; hasCfun(cs); cs = tl(cs)) {
            alts = cons(mkAltEq(tyconRec(t)->line,
                                makeDPats2(hd(cs), userArity(hd(cs)))),
                        alts);
        }
        if (cfunOf(hd(tyconRec(t)->defn)) != 0) {
            alts = cons(pair(cons(WILDCARD, cons(WILDCARD, NIL)),
                             pair(mkInt(tyconRec(t)->line), nameFalse)),
                        alts);
        }
        alts = rev(alts);
    } else { /* special case for tuples    */
        alts = singleton(mkAltEq(0, makeDPats2(t, tupleOf(t))));
    }

    return singleton(mkBind("==", alts));
}

/* Make alt for an equation for == using patterns in pats for lhs arguments */
static Pair mkAltEq(int line, List pats) {
    Cell p = hd(pats);
    Cell q = hd(tl(pats));
    Cell e = nameTrue;

    if (isAp(p)) {
        e = ap2(nameEq, arg(p), arg(q));
        for (p = fun(p), q = fun(q); isAp(p); p = fun(p), q = fun(q)) {
            e = ap2(nameAnd, ap2(nameEq, arg(p), arg(q)), e);
        }
    }
    return pair(pats, pair(mkInt(line), e));
}

/* Make binding for derived compare for some TUPLE or DATATYPE t */
static List deriveOrd(Type t) {
    List alts = NIL;
    if (isEnumType(t)) { /* special case for enumerations   */
        alts = mkVarAlts(tyconRec(t)->line, nameConCmp);
    } else if (isTycon(t)) { /* deal with type constrs     */
        List cs = tyconRec(t)->defn;
        for (; hasCfun(cs); cs = tl(cs)) {
            alts = cons(mkAltOrd(tyconRec(t)->line,
                                 makeDPats2(hd(cs), userArity(hd(cs)))),
                        alts);
        }
        if (cfunOf(hd(tyconRec(t)->defn)) != 0) {
            Cell u = inventVar();
            Cell w = inventVar();
            alts =
                cons(pair(cons(u, singleton(w)), pair(mkInt(tyconRec(t)->line),
                                                      ap2(nameConCmp, u, w))),
                     alts);
        }
        alts = rev(alts);
    } else { /* special case for tuples    */
        alts = singleton(mkAltOrd(0, makeDPats2(t, tupleOf(t))));
    }

    return singleton(mkBind("compare", alts));
}

/* Make alt for eqn for compare using patterns in pats for lhs arguments */
static Pair mkAltOrd(int line, List pats) {
    Cell p = hd(pats);
    Cell q = hd(tl(pats));
    Cell e = nameEQ;

    if (isAp(p)) {
        e = ap2(nameCompare, arg(p), arg(q));
        for (p = fun(p), q = fun(q); isAp(p); p = fun(p), q = fun(q)) {
            e = ap(ap2(nameCompAux, arg(p), arg(q)), e);
        }
    }

    return pair(pats, pair(mkInt(line), e));
}

/* Generate pattern list by putting two new patterns with head h and new var
 * components */
static List makeDPats2(Cell h, int n) {
    List us = getDiVars(2 * n);
    List vs = NIL;
    Cell p;
    int i;

    for (i = 0, p = h; i < n; ++i) { /* make first version of pattern   */
        p = ap(p, hd(us));
        us = tl(us);
    }
    vs = cons(p, vs);

    for (i = 0, p = h; i < n; ++i) { /* make second version of pattern  */
        p = ap(p, hd(us));
        us = tl(us);
    }
    return cons(p, vs);
}

/* --------------------------------------------------------------------------
 * Deriving Ix and Enum:
 * ------------------------------------------------------------------------*/

/* Construct definition of enumeration */
static List deriveEnum(Tycon t) {
    int l = tyconRec(t)->line;

    if (!isEnumType(t)) {
        errHead(l);
        errPrintf("Can only derive instances of Enum for enumeration types");
        errFail();
    }

    return cons(
        mkBind("toEnum", mkVarAlts(l, ap(nameEnToEn, hd(tyconRec(t)->defn)))),
        cons(mkBind("fromEnum", mkVarAlts(l, nameEnFrEn)),
             cons(mkBind("enumFrom", mkVarAlts(l, nameEnFrom)),
                  cons(mkBind("enumFromTo", mkVarAlts(l, nameEnFrTo)),
                       cons(mkBind("enumFromThen", mkVarAlts(l, nameEnFrTh)),
                            NIL)))));
}

/* Construct definition of indexing */
static List deriveIx(Tycon t) {
    if (isEnumType(t)) { /* Definitions for enumerations                   */
        return cons(
            mkBind("range", mkVarAlts(tyconRec(t)->line, nameEnRange)),
            cons(mkBind("index", mkVarAlts(tyconRec(t)->line, nameEnIndex)),
                 cons(mkBind("inRange",
                             mkVarAlts(tyconRec(t)->line, nameEnInRng)),
                      NIL)));
    } else if (isTuple(t)) { /* Definitions for product types      */
        return mkIxBinds(0, t, tupleOf(t));
    } else if (isTycon(t) && cfunOf(hd(tyconRec(t)->defn)) == 0) {
        return mkIxBinds(tyconRec(t)->line, hd(tyconRec(t)->defn),
                         userArity(hd(tyconRec(t)->defn)));
    }

    errHead(tyconRec(t)->line);
    errPrintf(
        "Can only derive instances of Ix for enumeration or product types");
    errFail();
    return NIL; /* NOTREACHED*/
}

/* Determine whether t is an enumeration type (i.e. all constructors arity ==
 * 0) */
static bool isEnumType(Tycon t) {
    if (isTycon(t) &&
        (tyconRec(t)->what == DATATYPE || tyconRec(t)->what == NEWTYPE)) {
        List cs = tyconRec(t)->defn;
        for (; hasCfun(cs); cs = tl(cs)) {
            if (nameRec(hd(cs))->arity != 0) {
                return false;
            }
        }
        addCfunTable(t);
        return true;
    }
    return false;
}

/* Build bindings for derived Ix on a product type */
static List mkIxBinds(int line, Cell h, int n) {
    List vs = getDiVars(3 * n);
    Cell ls = h;
    Cell us = h;
    Cell is = h;
    Cell js = h;
    Cell pr = NIL;
    Cell pats = NIL;
    int i;

    for (i = 0; i < n; ++i, vs = tl(vs)) { /* build three patterns for values */
        ls = ap(ls, hd(vs));               /* of the datatype concerned    */
        us = ap(us, hd(vs = tl(vs)));
        is = ap(is, hd(vs = tl(vs)));
        js = ap(js, hd(vs)); /* ... and one expression     */
    }
    pr = ap2(mkTuple(2), ls, us);   /* Build (ls,us)               */
    pats = cons(pr, cons(is, NIL)); /* Build [(ls,us),is]                  */

    return cons(prodRange(line, singleton(pr), ls, us, js),
                cons(prodIndex(line, pats, ls, us, is),
                     cons(prodInRange(line, pats, ls, us, is), NIL)));
}

/* Make definition of range for a product type */
static Cell prodRange(int line, List pats, Cell ls, Cell us, Cell is) {
    /* range :: (a,a) -> [a]
     * range (X a b c, X p q r)
     *   = [ X x y z | x <- range (a,p), y <- range (b,q), z <- range (c,r) ]
     */
    Cell is1 = is;
    List e = NIL;
    for (; isAp(ls); ls = fun(ls), us = fun(us), is = fun(is)) {
        e = cons(
            ap(FROMQUAL,
               pair(arg(is), ap(nameRange, ap2(mkTuple(2), arg(ls), arg(us))))),
            e);
    }
    e = ap(COMP, pair(is1, e));
    e = singleton(pair(pats, pair(mkInt(line), e)));
    return mkBind("range", e);
}

/* Make definition of index for a product type */
static Cell prodIndex(int line, List pats, Cell ls, Cell us, Cell is) {
    /* index :: (a,a) -> a -> Bool
     * index (X a b c, X p q r) (X x y z)
     *  = index (c,r) z + rangeSize (c,r) * (
     *     index (b,q) y + rangeSize (b,q) * (
     *      index (a,x) x))
     */
    List xs = NIL;
    Cell e = NIL;
    for (; isAp(ls); ls = fun(ls), us = fun(us), is = fun(is)) {
        xs = cons(ap2(nameIndex, ap2(mkTuple(2), arg(ls), arg(us)), arg(is)),
                  xs);
    }
    for (e = hd(xs); nonNull(xs = tl(xs));) {
        Cell x = hd(xs);
        e = ap2(namePlus, x, ap2(nameMult, ap(nameRangeSize, arg(fun(x))), e));
    }
    e = singleton(pair(pats, pair(mkInt(line), e)));
    return mkBind("index", e);
}

/* Make definition of inRange for a product type */
static Cell prodInRange(int line, List pats, Cell ls, Cell us, Cell is) {
    /* inRange :: (a,a) -> a -> Bool
     * inRange (X a b c, X p q r) (X x y z)
     *          = inRange (a,p) x && inRange (b,q) y && inRange (c,r) z
     */
    Cell e = ap2(nameInRange, ap2(mkTuple(2), arg(ls), arg(us)), arg(is));
    while (ls = fun(ls), us = fun(us), is = fun(is), isAp(ls)) {
        e = ap2(nameAnd,
                ap2(nameInRange, ap2(mkTuple(2), arg(ls), arg(us)), arg(is)),
                e);
    }
    e = singleton(pair(pats, pair(mkInt(line), e)));
    return mkBind("inRange", e);
}

/* --------------------------------------------------------------------------
 * Deriving Show:
 * ------------------------------------------------------------------------*/

/* Construct definition of text conversion */
static List deriveShow(Tycon t) {
    List alts = NIL;
    if (isTycon(t)) { /* deal with type constrs    */
        List cs = tyconRec(t)->defn;
        for (; hasCfun(cs); cs = tl(cs)) {
            alts = cons(mkAltShow(tyconRec(t)->line, hd(cs), userArity(hd(cs))),
                        alts);
        }
        alts = rev(alts);
    } else { /* special case for tuples    */
        alts = singleton(mkAltShow(0, t, tupleOf(t)));
    }

    return singleton(mkBind("showsPrec", alts));
}

/* Make alt for showsPrec eqn */
static Cell mkAltShow(int line, Cell h, int a) {
    List vs = getDiVars(a + 1);
    Cell d = hd(vs);
    Cell pat = h;
    List pats = NIL;
    int i = 0;
    for (vs = tl(vs); i < a; i++) {
        pat = ap(pat, hd(vs));
        vs = tl(vs);
    }
    pats = cons(d, cons(pat, NIL));
    return pair(pats, pair(mkInt(line), showsPrecRhs(d, pat, a)));
}

#define APP_PREC 10 /* precedence of function application */
#define shows0 ap(nameShowsPrec, mkInt(0))
#define showsN(x) ap(nameShowsPrec, mkInt(x))
#define showsOP ap(nameComp, consChar('('))
#define showsOB ap(nameComp, consChar('{'))
#define showsCM ap(nameComp, consChar(','))
#define showsSP ap(nameComp, consChar(' '))
#define showsBQ ap(nameComp, consChar('`'))
#define showsCP consChar(')')
#define showsCB consChar('}')

/* Build a rhs for showsPrec for a given pattern, pat */
static Cell showsPrecRhs(Cell d, Cell pat, int a) {
    Cell h = getHead(pat);
    List cfs = cfunSfuns;

    if (isTuple(h)) {
        /* To display a tuple:
         *    showsPrec d (a,b,c,d) = showChar '(' . showsPrec 0 a .
         *                            showChar ',' . showsPrec 0 b .
         *                            showChar ',' . showsPrec 0 c .
         *                            showChar ',' . showsPrec 0 d .
         *                            showChar ')'
         */
        int i = tupleOf(h);
        Cell rhs = showsCP;
        for (; i > 1; --i) {
            rhs = ap(showsCM, ap2(nameComp, ap(shows0, arg(pat)), rhs));
            pat = fun(pat);
        }
        return ap(showsOP, ap2(nameComp, ap(shows0, arg(pat)), rhs));
    }

    for (; nonNull(cfs) && h != fst(hd(cfs)); cfs = tl(cfs)) {
    }
    if (nonNull(cfs)) {
        /* To display a value using record syntax:
         *    showsPrec d C{x=e, y=f, z=g} =
         *              showString "C"  . showString " {" .
         *              showField "x" e . showString ", " .
         *              showField "y" f . showString ", " .
         *              showField "z" g . showChar '}'
         *    showField lab val
         *      = showString lab . showString " = " . shows val
         */
        Cell rhs = showsCB;
        List vs = dupOnto(snd(hd(cfs)), NIL);
        if (isAp(pat)) {
            for (;;) {
                rhs = ap2(nameComp,
                          ap2(nameShowField, mkStr(textOf(hd(vs))), arg(pat)),
                          rhs);
                pat = fun(pat);
                vs = tl(vs);
                if (isAp(pat)) {
                    rhs = ap(showsCM, ap(showsSP, rhs));
                } else {
                    break;
                }
            }
        }
        rhs = ap2(nameComp, ap(nameApp, mkStr(nameRec(h)->text)),
                  ap(showsSP, ap(showsOB, rhs)));
        return rhs;
    } else if (a == 0) {
        /* To display a nullary constructor:
         *    showsPrec d Foo = showString "Foo"
         */
        return ap(nameApp, mkStr(nameRec(h)->text));
    } else {
        Syntax s = syntaxOf(h);
        if (a == 2 && assocOf(s) != APPLIC) {
            /* For a binary constructor with prec p:
             * showsPrec d (a :* b) = showParen (d > p)
             *                          (showsPrec lp a . showChar ' ' .
             *                           showsString s  . showChar ' ' .
             *                           showsPrec rp b)
             */
            int p = precOf(s);
            int lp =
                (p +
                 1); /* how it was in Haskell from Day 1 until 11/4/2002:  (assocOf(s)==LEFT_ASS)  ? p : (p+1); */
            int rp = (p + 1); /* ditto:  (assocOf(s)==RIGHT_ASS) ? p : (p+1); */
            Cell rhs = ap(showsSP, ap2(nameShowsPrec, mkInt(rp), arg(pat)));
            if (defaultSyntax(nameRec(h)->text) == APPLIC) {
                rhs = ap(showsBQ,
                         ap2(nameComp,
                             ap(nameApp, mkStr(fixLitText(nameRec(h)->text))),
                             ap(showsBQ, rhs)));
            } else {
                rhs =
                    ap2(nameComp,
                        ap(nameApp, mkStr(fixLitText(nameRec(h)->text))), rhs);
            }

            rhs = ap2(nameComp, ap2(nameShowsPrec, mkInt(lp), arg(fun(pat))),
                      ap(showsSP, rhs));
            rhs = ap2(nameShowParen, ap2(nameGt, d, mkInt(p)), rhs);
            return rhs;
        } else {
            /* To display a non-nullary constructor with applicative syntax:
             *    showsPrec d (Foo x y) = showParen (d>APP_PREC)
             *                             (showString "Foo" .
             *                              showChar ' ' . showsPrec (APP_PREC+1) x .
             *                              showChar ' ' . showsPrec (APP_PREC+1) y)
             */
            Cell rhs = ap(showsSP, ap(showsN(APP_PREC + 1), arg(pat)));
            for (pat = fun(pat); isAp(pat); pat = fun(pat)) {
                rhs = ap(showsSP, ap2(nameComp,
                                      ap(showsN(APP_PREC + 1), arg(pat)), rhs));
            }
            rhs = ap2(nameComp, ap(nameApp, mkStr(nameRec(h)->text)), rhs);
            rhs = ap2(nameShowParen, ap2(nameGt, d, mkInt(APP_PREC)), rhs);
            return rhs;
        }
    }
}
#undef showsN
#undef shows0
#undef showsOP
#undef showsOB
#undef showsCM
#undef showsSP
#undef showsBQ
#undef showsCP
#undef showsCB

/* --------------------------------------------------------------------------
 * Deriving Read:
 * ------------------------------------------------------------------------*/

#define Tuple2(f, s) ap2(mkTuple(2), f, s)
#define Lex(r) ap(nameLex, r)
#define ZFexp(h, q) ap(FROMQUAL, pair(h, q))
#define ReadsPrec(n, e) ap2(nameReadsPrec, n, e)
#define Lambda(v, e) ap(LAMBDA, pair(v, pair(mkInt(0), e)))
#define ReadParen(a, b, c) ap(ap2(nameReadParen, a, b), c)
#define ReadField(f, s) ap2(nameReadField, f, s)
#define GT(l, r) ap2(nameGt, l, r)
#define Append(a, b) ap2(nameApp, a, b)

/*  Construct the readsPrec function of the form:
 *
 *    readsPrec d r = (readParen (d>p1) (\r -> [ (C1 ...,s) | ... ]) r ++
 *                    (readParen (d>p2) (\r -> [ (C2 ...,s) | ... ]) r ++
 *                    ...
 *                    (readParen (d>pn) (\r -> [ (Cn ...,s) | ... ]) r) ... ))
 */
/* Construct definition of text reader */
static List deriveRead(Cell t) {
    Cell alt = NIL;
    Cell exp = NIL;
    Cell d = inventVar();
    Cell r = inventVar();
    List pat = cons(d, cons(r, NIL));
    int line = 0;

    if (isTycon(t)) {
        List cs = tyconRec(t)->defn;
        List exps = NIL;
        for (; hasCfun(cs); cs = tl(cs)) {
            exps = cons(mkReadCon(hd(cs), d, r), exps);
        }
        /* reverse concatenate list of subexpressions */
        exp = hd(exps);
        for (exps = tl(exps); nonNull(exps); exps = tl(exps)) {
            exp = ap2(nameApp, hd(exps), exp);
        }
        line = tyconRec(t)->line;
    } else { /* Tuples */
        exp = ap(mkReadTuple(t), r);
    }
    /* printExp(stdout,exp); putc('\n',stdout); */
    alt = pair(pat, pair(mkInt(line), exp));
    return singleton(mkBind("readsPrec", singleton(alt)));
}

/* Generate an expression of the form:
 *
 *   readParen (d > p) <derived expression> r
 *
 * for a (non-tuple) constructor "con" of precedence "p".
 */

/* Generate reader for a constructor */
static Cell mkReadCon(Name con, Cell d, Cell r) {
    Cell exp = NIL;
    int p = 0;
    Syntax s = syntaxOf(con);
    List cfs = cfunSfuns;

    for (; nonNull(cfs) && con != fst(hd(cfs)); cfs = tl(cfs)) {
    }
    if (nonNull(cfs)) {
        exp = mkReadRecord(con, snd(hd(cfs)));
        return ReadParen(nameFalse, exp, r);
    }

    if (userArity(con) == 2 && assocOf(s) != APPLIC) {
        exp = mkReadInfix(con);
        p = precOf(s);
    } else {
        exp = mkReadPrefix(con);
        p = APP_PREC;
    }
    return ReadParen(userArity(con) == 0 ? nameFalse : GT(d, mkInt(p)), exp, r);
}

/* Given an n-ary prefix constructor, generate a single lambda
 * expression, such that
 *
 *   data T ... = Constr a1 a2 .. an | ....
 *
 * derives
 *
 *   \ r -> [ (Constr t1 t2 ... tn, sn) | ("Constr",s0) <- lex r,
 *                                        (t1,s1) <- readsPrec (APP_PREC+1) s0,
 *                                        (t2,s2) <- readsPrec (APP_PREC+1) s1,
 *                                        ...,
 *                                        (tn,sn) <- readsPrec (APP_PREC+1) sn-1 ]
 *
 */
/* ReadsPrec for prefix constructor */
static Cell mkReadPrefix(Cell con) {
    int arity = userArity(con);
    Cell cn = mkStr(nameRec(con)->text);
    Cell r = inventVar();
    Cell prev_s = inventVar();
    Cell exp = con;
    List quals = NIL;
    int i;

    /* build (reversed) list of qualifiers and constructor */
    quals = cons(ZFexp(Tuple2(cn, prev_s), Lex(r)), quals);
    for (i = 0; i < arity; i++) {
        Cell t = inventVar();
        Cell s = inventVar();
        quals = cons(
            ZFexp(Tuple2(t, s), ReadsPrec(mkInt(APP_PREC + 1), prev_s)), quals);
        exp = ap(exp, t);
        prev_s = s;
    }

    /* \r -> [ (exp, prev_s) | quals ] */
    return Lambda(singleton(r),
                  ap(COMP, pair(Tuple2(exp, prev_s), rev(quals))));
}

/* Given a binary infix constructor of precedence p
 *
 *   ... | T1 `con` T2 | ...
 *
 * generate the lambda expression
 *
 *   \ r -> [ (u `con` v, s2) | (u,s0)     <- readsPrec lp r,
 *                              ("con",s1) <- lex s0,
 *                              (v,s2)     <- readsPrec rp s1 ]
 *
 * where lp and rp are either p or p+1 depending on associativity
 */
static Cell mkReadInfix(Cell con) {
    Syntax s = syntaxOf(con);
    int p = precOf(s);
    int lp =
        (p +
         1); /* how it was in Haskell from Day 1 until 11/4/2002: assocOf(s)==LEFT_ASS  ? p : (p+1); */
    int rp = (p + 1); /* ditto: assocOf(s)==RIGHT_ASS ? p : (p+1); */
    Cell cn = mkStr(nameRec(con)->text);
    Cell r = inventVar();
    Cell s0 = inventVar();
    Cell s1 = inventVar();
    Cell s2 = inventVar();
    Cell u = inventVar();
    Cell v = inventVar();
    List quals = NIL;

    quals = cons(ZFexp(Tuple2(u, s0), ReadsPrec(mkInt(lp), r)), quals);
    quals = cons(ZFexp(Tuple2(cn, s1), Lex(s0)), quals);
    quals = cons(ZFexp(Tuple2(v, s2), ReadsPrec(mkInt(rp), s1)), quals);

    return Lambda(singleton(r),
                  ap(COMP, pair(Tuple2(ap2(con, u, v), s2), rev(quals))));
}

/* Given the n-ary tuple constructor return a lambda expression:
 *
 *   \ r -> [ ((t1,t2,...tn),s(2n+1)) | ("(",s0)      <- lex r,
 *                                      (t1, s1)      <- readsPrec 0 s0,
 *                                      ...
 *                                      (",",s(2n-1)) <- lex s(2n-2),
 *                                      (tn, s(2n))   <- readsPrec 0 s(2n-1),
 *                                      (")",s(2n+1)) <- lex s(2n) ]
 */
/* ReadsPrec for n-tuple */
static Cell mkReadTuple(Cell tup) {
    int arity = tupleOf(tup);
    Cell lp = mkStr(findText("("));
    Cell rp = mkStr(findText(")"));
    Cell co = mkStr(findText(","));
    Cell sep = lp;
    Cell r = inventVar();
    Cell prev_s = r;
    Cell s = inventVar();
    Cell exp = tup;
    List quals = NIL;
    int i;

    /* build (reversed) list of qualifiers and constructor */
    for (i = 0; i < arity; i++) {
        Cell t = inventVar();
        Cell si = inventVar();
        Cell sj = inventVar();
        quals = cons(ZFexp(Tuple2(sep, si), Lex(prev_s)), quals);
        quals = cons(ZFexp(Tuple2(t, sj), ReadsPrec(mkInt(0), si)), quals);
        exp = ap(exp, t);
        prev_s = sj;
        sep = co;
    }
    quals = cons(ZFexp(Tuple2(rp, s), Lex(prev_s)), quals);

    /* \ r -> [ (exp,s) | quals ] */
    return Lambda(singleton(r), ap(COMP, pair(Tuple2(exp, s), rev(quals))));
}

/* Given a record constructor
 *
 *   ... | C { f1 :: T1, ... fn :: Tn } | ...
 *
 * generate the expression:
 *
 *   \ r -> [(C t1 t2 ... tn,s(2n+1)) | ("C", s0)    <- lex r,
 *                                      ("{", s1)    <- lex s0,
 *                                      (t1,  s2)    <- readField "f1" s1,
 *                                      ...
 *                                      (",", s(2n-1)) <- lex s(2n),
 *                                      (tn,  s(2n)) <- readField "fn" s(2n+1),
 *                                      ("}", s(2n+1)) <- lex s(2n+2) ]
 *
 * where
 *
 *   readField    :: Read a => String -> ReadS a
 *   readField m s0 = [ r | (t,  s1) <- lex s0, t == m,
 *                          ("=",s2) <- lex s1,
 *                          r        <- reads s2 ]
 */
/* ReadsPrec for record constructor */
static Cell mkReadRecord(Cell con, List fs) {
    Cell cn = mkStr(nameRec(con)->text);
    Cell lb = mkStr(findText("{"));
    Cell rb = mkStr(findText("}"));
    Cell co = mkStr(findText(","));
    Cell sep = lb;
    Cell r = inventVar();
    Cell s0 = inventVar();
    Cell prev_s = s0;
    Cell s = inventVar();
    Cell exp = con;
    List quals = NIL;

    /* build (reversed) list of qualifiers and constructor */
    quals = cons(ZFexp(Tuple2(cn, s0), Lex(r)), quals);
    for (; nonNull(fs); fs = tl(fs)) {
        Cell f = mkStr(textOf(hd(fs)));
        Cell t = inventVar();
        Cell si = inventVar();
        Cell sj = inventVar();
        quals = cons(ZFexp(Tuple2(sep, si), Lex(prev_s)), quals);
        quals = cons(ZFexp(Tuple2(t, sj), ReadField(f, si)), quals);
        exp = ap(exp, t);
        prev_s = sj;
        sep = co;
    }
    quals = cons(ZFexp(Tuple2(rb, s), Lex(prev_s)), quals);

    /* \ r -> [ (exp,s) | quals ] */
    return Lambda(singleton(r), ap(COMP, pair(Tuple2(exp, s), rev(quals))));
}

#undef Tuple2
#undef Lex
#undef ZFexp
#undef ReadsPrec
#undef Lambda
#undef ReadParen
#undef ReadField
#undef GT
#undef Append

/* --------------------------------------------------------------------------
 * Deriving Bounded:
 * ------------------------------------------------------------------------*/

/* Construct definition of bounds */
static List deriveBounded(Tycon t) {
    if (isEnumType(t)) {
        Cell last = tyconRec(t)->defn;
        Cell first = hd(last);
        while (hasCfun(tl(last))) {
            last = tl(last);
        }
        return cons(
            mkBind("minBound", mkVarAlts(tyconRec(t)->line, first)),
            cons(mkBind("maxBound", mkVarAlts(tyconRec(t)->line, hd(last))),
                 NIL));
    } else if (isTuple(t)) { /* Definitions for product types      */
        return mkBndBinds(0, t, tupleOf(t));
    } else if (isTycon(t) && cfunOf(hd(tyconRec(t)->defn)) == 0) {
        return mkBndBinds(tyconRec(t)->line, hd(tyconRec(t)->defn),
                          userArity(hd(tyconRec(t)->defn)));
    }

    errHead(tyconRec(t)->line);
    errPrintf(
        "Can only derive instances of Bounded for enumeration and product types");
    errFail();
    return NIL;
}

/* Build bindings for derived Bounded on a product type */
static List mkBndBinds(int line, Cell h, int n) {
    Cell minB = h;
    Cell maxB = h;
    while (n-- > 0) {
        minB = ap(minB, nameMinBnd);
        maxB = ap(maxB, nameMaxBnd);
    }
    return cons(mkBind("minBound", mkVarAlts(line, minB)),
                cons(mkBind("maxBound", mkVarAlts(line, maxB)), NIL));
}

/* Implement every instance asked for by a deriving clause and hand the list
 * back to the caller.  The three steps have to happen in this order, which
 * is why the caller is given one call and not the list. */
List derivedInstances(void) {
    mapProc(addDerivImp, derivedInsts); /* Add impls for derived instances */
    deriveContexts(derivedInsts);       /* Calculate derived inst contexts */
    return derivedInsts;
}

/* --------------------------------------------------------------------------
 * Control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const derivingRoots[] = {
    &derivedInsts,
    &diVars,
};

void derivingControl(int what) {
    switch (what) {
    case RESET:
        derivedInsts = NIL;
        diVars = NIL;
        diNum = 0;
        break;

    case INSTALL:
        registerRoots(derivingRoots);
        derivingControl(RESET);
        break;
    default:
        break;
    }
}

/*-------------------------------------------------------------------------*/
