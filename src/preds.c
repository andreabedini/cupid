/* --------------------------------------------------------------------------
 * Part of the type checker dealing with predicates and entailment
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: preds.c,v $
 * $Revision: 1.36 $
 * $Date: 2003/11/01 17:02:47 $
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "output.h" /* IWYU pragma: keep -- printPred, under EXPLAIN_INSTANCE_RESOLUTION */
#include "subst.h"
#include "opts.h" /* allowOverlap, EXPLAIN_INSTANCE_RESOLUTION */
#include "preds.h"
#include "tagcheck.h"

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static Void local overEvid(Cell, Cell);

static Void local cutoffExceeded(Cell, Int, List);
static Cell local scFind(Cell, Cell, Int, Cell, Int, Int);
static Cell local entail(List, Cell, Int, Int);

static Bool local anyGenerics(Type, Int);

static Bool local resolveVar(Int, Bool);
static Class local classConstraining(Int, Cell, Int);

/* --------------------------------------------------------------------------
 * Predicate assignments:
 *
 * A predicate assignment is represented by a list of triples (pi,o,ev)
 * where o is the offset for types in pi, with evidence required at the
 * node pointed to by ev (which is taken as a dictionary parameter if
 * no other evidence is available).  Note that the ev node will be
 * overwritten at a later stage if evidence for that predicate is found
 * subsequently.
 * ------------------------------------------------------------------------*/

List preds; /* Current predicate assignment        */

Cell assumeEvid(Cell pi, Int o) /* Add predicate pi (offset o) to  */
                                /* preds with new dict var nd      */
{
    assertPred(pi);
    Cell nd = inventDictVar();
    preds = cons(triple(pi, mkInt(o), nd), preds);
    return nd;
}

List makePredAss(List qs, Int o) /* Make list of predicate assumps. */
                                 /* from qs (offset o), w/ new dict */
{                                /* vars for each predicate        */
    assertPredList(qs);
    List result = NIL;
    for (; nonNull(qs); qs = tl(qs))
        result = cons(triple(hd(qs), mkInt(o), inventDictVar()), result);
    return rev(result);
}

List copyPreds(List qs) /* Copy list of predicates         */
{
    List result = NIL;
    for (; nonNull(qs); qs = tl(qs)) {
        Cell pi = hd(qs);
        result = cons(copyPred(fst3(pi), intOf(snd3(pi))), result);
    }
    return rev(result);
}

Void qualify(List qs, Cell alt) /* Add extra dictionary args to    */
                                /* qualify alt by predicates in qs */
{                               /* :: ([Pat],Rhs)                  */
    List ds;
    for (ds = NIL; nonNull(qs); qs = tl(qs))
        ds = cons(thd3(hd(qs)), ds);
    fst(alt) = revOnto(ds, fst(alt));
}

Void qualifyBinding(List qs, Cell b) /* Add extra dict args to each        */
                                     /* alternative in function binding */
{
    if (!isVar(fst(b))) /* check for function binding      */
        internal("qualifyBinding");
    map1Proc(qualify, qs, snd(snd(b)));
}

Cell qualifyExpr(Int l, List ps, Cell e) /* Add dictionary params to expr   */
{
    if (nonNull(ps)) {           /* Qualify input expression with   */
        if (whatIs(e) != LAMBDA) /* additional dictionary params           */
            e = ap(LAMBDA, pair(NIL, pair(mkInt(l), e)));
        qualify(ps, snd(e));
    }
    return e;
}

static Void local overEvid(Cell dv,
                           Cell ev) /* Overwrite dict var dv with          */
                                    /* evidence ev                         */
{
    fst(dv) = nameInd;
    snd(dv) = ev;
}

/* --------------------------------------------------------------------------
 * Predicate entailment:
 *
 * Entailment plays a prominent role in the theory of qualified types, and
 * so, unsurprisingly, in the implementation too.  For practical reasons,
 * we break down entailment into two pieces.  The first, scEntail, uses
 * only the information provided by class declarations, while the second,
 * entail, also uses the information in instance declarations.
 *
 * scEntail uses the following auxiliary function to do its work:
 *
 *   scFind (e : pi') pi : Find evidence for predicate pi using only
 *                           equality of predicates, superclass entailment,
 *                           and the evidence e for pi'.
 *
 *   scFind (e : pi') pi =
 *
 *      if pi = pi' then
 *          return e
 *
 *      if (pi.class.level < pi'.class.level)
 *          get superclass entailment pi' ||- P
 *          for each (sc, pi0) in P
 *              if (ev := scFind (sc e : pi0) pi) /= NIL
 *                  return ev
 *
 *      return NIL
 *
 * This code assumes that the class hierarchy is acyclic, and that
 * each class has been assigned a `level', which is its height in
 * the hierachy.  The first of the assumptions guarantees that the
 * algorithm will terminate.  The comparison of levels is an
 * optimization that cuts down the search space: given that superclass
 * entailments can only be used to descend the hierarchy, there is no
 * way we can reach a higher level than the one that we start with,
 * and hence there is no point in looking if we reach such a position.
 *
 * scEntail extends scFind to work on whole predicate assignments:
 *
 *   scEntail P pi : Find evidence for predicate pi using the evidence
 *                   provided by the predicate assignment P, and using
 *                   only superclass entailments.
 *
 *   scEntail P pi =
 *
 *       for each (v:pi') in P
 *           if (ev := scFind (v:pi') pi) /= NIL
 *               return ev;
 *       return NIL
 *
 * ------------------------------------------------------------------------*/

Int cutoff = 40; /* Used to limit depth of recursion*/

static Void local cutoffExceeded(Cell pi, Int o, List ps)
/* Display error msg when cutoff   */
{
    clearMarks();
    errHead(0);
    errPrintf(
        "\n*** The type checker has reached the cutoff limit while trying to\n");
    errPrintf("*** determine whether:\n***     ");
    errPred(copyPred(pi, o));
    ps = copyPreds(ps);
    errPrintf("\n*** can be deduced from:\n***     ");
    errContext(ps);
    errPrintf(
        "\n*** This may indicate that the problem is undecidable.  However,\n");
    errPrintf(
        "*** you may still try to increase the cutoff limit using the -c\n");
    errPrintf("*** option and then try again.  (The current setting is -c%d)\n",
              cutoff);
    errFail();
}

static Cell local scFind(Cell e, Cell pi1, Int o1, Cell pi, Int o,
                         Int d) /* Use superclass entailment to   */
                                /* find evidence for (pi,o) using  */
                                /* the evidence e for (pi1,o1).    */
{
    Class h1 = getHead(pi1);
    Class h = getHead(pi);
    Cell ev = NIL;

    if (samePred(pi1, o1, pi, o))
        return e;

    if (isClass(h1) &&
        (!isClass(h) || cclassRec(h)->level < cclassRec(h1)->level)) {
        Int beta = newKindedVars(cclassRec(h1)->kinds);
        List scs = cclassRec(h1)->supers;
        List dsels = cclassRec(h1)->dsels;
        List ps = NIL;
        if (!matchPred(pi1, o1, cclassRec(h1)->head, beta))
            internal("scFind");

        for (; nonNull(scs); scs = tl(scs), dsels = tl(dsels))
            ps = cons(triple(hd(scs), mkInt(beta), ap(hd(dsels), e)), ps);
        ps = rev(ps);

#if EXPLAIN_INSTANCE_RESOLUTION
        if (showInstRes) {
            int i;
            for (i = 0; i < d; i++)
                fputc(' ', stdout);
            fputs("scEntail(scFind): ", stdout);
            printContext(stdout, copyPreds(ps));
            fputs(" ||- ", stdout);
            printPred(stdout, copyPred(pi, o));
            fputc('\n', stdout);
        }
#endif
        improve1(0, ps, pi, o);
        ev = scEntail(ps, pi, o, d);
#if EXPLAIN_INSTANCE_RESOLUTION
        if (showInstRes && nonNull(ev)) {
            int i;
            for (i = 0; i < d; i++)
                fputc(' ', stdout);
            fputs("scSat.\n", stdout);
        }
#endif
        return ev;
    }
    return NIL;
}

Cell scEntail(List ps, Cell pi, Int o,
              Int d) /* Calc evidence for (pi,o) from ps*/
                     /* Using superclasses and equality.*/
{
    if (d++ >= cutoff)
        cutoffExceeded(pi, o, ps);

    for (; nonNull(ps); ps = tl(ps)) {
        Cell pi1 = hd(ps);
        Cell ev = scFind(thd3(pi1), fst3(pi1), intOf(snd3(pi1)), pi, o, d);
        if (nonNull(ev))
            return ev;
    }
    return NIL;
}

/* --------------------------------------------------------------------------
 * Now we reach the main entailment routine:
 *
 *   entail P pi : Find evidence for predicate pi using the evidence
 *                 provided by the predicate assignment P.
 *
 *   entail P pi =
 *
 *       if (ev := scEntail P pi) /= NIL
 *           return ev;
 *
 *       if there is an instance entailment i : Q ||- pi
 *           for each pi' in Q
 *               if (ev := entail P pi') /= NIL
 *                   i := ap(i,ev)
 *               else
 *                   return NIL
 *           return i
 *
 *       return NIL;
 *
 * The form of evidence expressions produced by scEntail can be described
 * by the grammar:
 *
 *    e  =  v  |  sc e            (v = evidence var, sc = superclass sel)
 *
 * while entail extends this to include dictionary expressions given by:
 *
 *    d  =  e  |  mki d1 ... dn   (mki = dictionary constructor)
 *
 * A full grammar for evidence expressions is:
 *
 *    d   =   v   |   sc d   |   mki d1 ... dn
 *
 * and this includes evidence expressions of the form  sc (mki d1 ... dn)
 * that can never be produced by either of the entail functions described
 * above.  This is good, from a practical perspective, because t means
 * that we won't waste effort building a dictionary (mki d1 ... dn) only
 * to extract just one superclass component and throw the rest away.
 * Moreover, conditions on instance decls already guarantee that any
 * expression of this form can be rewritten in the form  mki' d1' ... dn'.
 * (Minor point: they don't guarantee that such rewritings will lead to
 * smaller terms, and hence to termination.  However, we have already
 * accepted the benefits of an undecidable entailment relation over
 * guarantees of termination, and this additional quirk is unlikely
 * to cause any further concern, except in pathological cases.)
 * ------------------------------------------------------------------------*/

static Cell local entail(List ps, Cell pi, Int o,
                         Int d) /* Calc evidence for (pi,o) from ps*/
                                /* Uses superclasses, equality,    */
                                /* tautology, and construction     */
{
    assertPred(pi);
    Cell ev = NIL;

#if EXPLAIN_INSTANCE_RESOLUTION
    if (showInstRes) {
        int i;
        for (i = 0; i < d; i++)
            fputc(' ', stdout);
        fputs("entail: ", stdout);
        printContext(stdout, copyPreds(ps));
        fputs(" ||- ", stdout);
        printPred(stdout, copyPred(pi, o));
        fputc('\n', stdout);
    }
#endif

    ev = scEntail(ps, pi, o, d);
    if (nonNull(ev)) {
#if EXPLAIN_INSTANCE_RESOLUTION
        if (showInstRes) {
            int i;
            for (i = 0; i < d; i++)
                fputc(' ', stdout);
            fputs("scSat.\n", stdout);
        }
#endif
    } else {
        ev = inEntail(ps, pi, o, d);
#if EXPLAIN_INSTANCE_RESOLUTION
        if (nonNull(ev) && showInstRes) {
            int i;
            for (i = 0; i < d; i++)
                fputc(' ', stdout);
            fputs("inSat.\n", stdout);
        }
#endif
    }
    return ev;
}

Cell inEntail(List ps, Cell pi, Int o,
              Int d) /* Calc evidence for (pi,o) from ps*/
                     /* using a top-level instance         */
                     /* entailment                         */
{
#if EXPLAIN_INSTANCE_RESOLUTION
    int i;
#endif
    Inst in;

    if (d++ >= cutoff)
        cutoffExceeded(pi, o, ps);

    in = findInstFor(pi, o); /* Class predicates                   */
    if (nonNull(in)) {
        Int beta = typeOff;
        Cell e = instRec(in)->builder;
        List es = instRec(in)->specifics;
        List fs = NIL;
        for (; nonNull(es); es = tl(es))
            fs = cons(triple(hd(es), mkInt(beta), NIL), fs);
        fs = rev(fs);
        improve(0, ps, fs);
#if EXPLAIN_INSTANCE_RESOLUTION
        if (showInstRes) {
            for (i = 0; i < d; i++)
                fputc(' ', stdout);
            fputs("try ", stdout);
            printContext(stdout, copyPreds(fs));
            fputs(" => ", stdout);
            printPred(stdout, copyPred(instRec(in)->head, beta));
            fputc('\n', stdout);
        }
#endif
        for (es = instRec(in)->specifics; nonNull(es); es = tl(es)) {
            Cell ev;
            improve1(0, ps, hd(es), beta);
            ev = entail(ps, hd(es), beta, d);
            if (nonNull(ev))
                e = ap(e, ev);
            else
                return NIL;
        }
        return e;
    }
#if EXPLAIN_INSTANCE_RESOLUTION
    else {
        if (showInstRes) {
            for (i = 0; i < d; i++)
                fputc(' ', stdout);
            fputs("No instance found for ", stdout);
            printPred(stdout, copyPred(pi, o));
            fputc('\n', stdout);
        }
    }
#endif
    return NIL;
}

Cell provePred(Kinds ks, List ps, Cell pi) /* Find evidence for predicate pi  */
                                           /* assuming ps.  If ps is null,    */
                                           /* then we get to decide whether   */
{                                          /* is tautological, and we can use */
    assertKinds(ks);
    assertPred(pi);
    Int beta; /* the evidence as a dictionary.   */
    Cell ev;
    emptySubstitution();
    beta = newKindedVars(ks);   /* (ks provides kinds for any      */
    ps = makePredAss(ps, beta); /*  vars that appear in pi.)       */
    ev = entail(ps, pi, beta, 0);
    emptySubstitution();
    return ev;
}

Cell resolvePred(Kinds ks, Cell pi) /* Find evidence for predicate pi. */
                                    /* Unlike `provePred', also uses   */
{                                   /* desparate measures like context */
    assertKinds(ks);
    assertPred(pi);
    Int beta; /* reduction and defaulting.       */
    List qs;

    emptySubstitution();
    beta = newKindedVars(ks);
    qs = makePredAss(cons(pi, NIL), beta);
    preds = qs;
    elimTauts();
    if (resolveDefs(NIL, TRUE))
        elimTauts();
    emptySubstitution();
    return (nonNull(preds) ? NIL : thd3(hd(qs)));
}

/* --------------------------------------------------------------------------
 * Predicate set Simplification:
 *
 * Calculate a minimal equivalent subset of a given set of predicates.
 * ------------------------------------------------------------------------*/

List scSimplify(List qs) /* Simplify predicates in qs,      */
{                        /* returning equiv minimal subset  */
    Int n = length(qs);

    while (0 < n--) {
        Cell pi = hd(qs);
        Cell ev = NIL;
#if EXPLAIN_INSTANCE_RESOLUTION
        if (showInstRes) {
            fputs("scSimplify: ", stdout);
            printContext(stdout, copyPreds(tl(qs)));
            fputs(" ||- ", stdout);
            printPred(stdout, copyPred(fst3(pi), intOf(snd3(pi))));
            fputc('\n', stdout);
        }
#endif
        ev = scEntail(tl(qs), fst3(pi), intOf(snd3(pi)), 0);
        if (nonNull(ev)) {
#if EXPLAIN_INSTANCE_RESOLUTION
            if (showInstRes)
                fputs("Simplified!\n", stdout);
#endif
            overEvid(thd3(pi), ev); /* Overwrite dict var with evidence*/
            qs = tl(qs);            /* ... and discard predicate           */
        } else {                    /* Otherwise, retain predicate         */
            Cell tmp = tl(qs);
            tl(qs) = NIL;
            qs = appendOnto(tmp, qs);
        }
    }
    return qs;
}

List simpleContext(List ps, Int o) /* Simplify context of skeletons   */
                                   /* skeletons, offset o, using           */
{                                  /* superclass hierarchy                 */
    return copyPreds(scSimplify(makePredAss(ps, o)));
}

/* --------------------------------------------------------------------------
 * Context splitting --- tautological and locally tautological predicates:
 * ------------------------------------------------------------------------*/

Void elimTauts() { /* Remove tautological constraints */
    if (!allowOverlap && !allowUnsafeOverlap) { /* from preds              */
        reducePreds(); /* Context reduction to head normal*/
    } else {           /* form is unsound with overlapping*/
        /* instances, so only entailed     */
        /* predicates are removed then     */
        List ps = preds;
        preds = NIL;
        while (nonNull(ps)) {
            Cell pi = hd(ps);
            Cell ev = entail(NIL, fst3(pi), intOf(snd3(pi)), 0);
            if (nonNull(ev)) {
                overEvid(thd3(pi), ev);
                ps = tl(ps);
            } else {
                List tmp = tl(ps);
                tl(ps) = preds;
                preds = ps;
                ps = tmp;
            }
        }
    }
}

static Int numFixedVars = 0; /* Number of fixed vars found         */

static Bool local anyGenerics(Type t,
                              Int o) /* Test for generic vars, and count*/
                                     /* fixed variables            */
{
    Type h = getDerefHead(t, o); /* This code is careful to expand  */
    Int a = argCount;            /* synonyms; mark* & copy* do not. */
    if (isSynonym(h) && a >= tyconRec(h)->arity) {
        expandSyn(h, a, &t, &o);
        return anyGenerics(t, o);
    } else {
        Tyvar *tyv;
        for (; 0 < a--; t = fun(t)) { /* cycle through any arguments       */
            tyv = deRef(&t, &o);
            if (anyGenerics(arg(t), o))
                return TRUE;
        }
        tyv = deRef(&t, &o);
        if (tyv)
            if (tyv->offs == FIXED_TYVAR) {
                numFixedVars++;
                return FALSE;
            } else
                return TRUE;
        else
            return FALSE;
    }
}

List elimOuterPreds(List sps) /* Simplify and defer any remaining*/
{                             /* preds that contain no generics. */
    List qs = NIL;
    elimTauts();
    for (preds = scSimplify(preds); nonNull(preds);) {
        Cell pi = hd(preds);
        Cell nx = tl(preds);
        if (anyGenerics(fst3(pi), intOf(snd3(pi))) || !isAp(fst3(pi))) {
            tl(preds) = qs; /* Retain predicate*/
            qs = preds;
        } else { /* Defer predicate */
            tl(preds) = sps;
            sps = preds;
        }
        preds = nx;
    }
    preds = qs;
    return sps;
}

List elimPredsUsing(List ps, List sps) /* Try to discharge or defer preds,*/
                                       /* splitting if necessary to match */
{                                      /* context ps.  sps = savePreds.   */
    List rems = NIL;
    while (nonNull(preds)) { /* Pick a predicate from preds        */
        Cell p = preds;
        Cell pi = fst3(hd(p));
        Int o = intOf(snd3(hd(p)));
        Cell ev = entail(ps, pi, o, 0);
        preds = tl(preds);

        if (nonNull(ev)) { /* Discharge if ps ||- (pi,o)           */
            overEvid(thd3(hd(p)), ev);
        } else if (!isAp(pi) || !anyGenerics(pi, o)) {
            tl(p) = sps; /* Defer if no generics                   */
            sps = p;
        } else { /* Try to split generics and fixed */
            Inst in;
            if (numFixedVars > 0 && nonNull(in = findInstFor(pi, o))) {
                List qs = instRec(in)->specifics;
                for (ev = instRec(in)->builder; nonNull(qs); qs = tl(qs))
                    ev = ap(ev, assumeEvid(hd(qs), typeOff));
                overEvid(thd3(hd(p)), ev);
            } else { /* No worthwhile progress possible */
                tl(p) = rems;
                rems = p;
            }
        }
    }
    preds = rems; /* Return any remaining predicates */
    return sps;
}

Void reducePreds() {         /* Context reduce predicates: uggh!*/
    List rems = NIL;         /* (A last resort for defaulting)  */
    while (nonNull(preds)) { /* Pick a predicate from preds        */
        Cell p = preds;
        Cell pi = fst3(hd(p));
        Int o = intOf(snd3(hd(p)));
        Inst in = NIL;
        in = findInstFor(pi, o);
        preds = tl(preds);
        if (nonNull(in)) {
            List qs = instRec(in)->specifics;
            Cell ev = instRec(in)->builder;
            for (; nonNull(qs); qs = tl(qs))
                ev = ap(ev, assumeEvid(hd(qs), typeOff));
            overEvid(thd3(hd(p)), ev);
        } else { /* No worthwhile progress possible */
            tl(p) = rems;
            rems = p;
        }
    }
    preds = scSimplify(rems); /* Return any remaining predicates */
}

Void normPreds(Int line) /* Normalize each element of preds */
{                        /* in some appropriate manner     */
}

/* --------------------------------------------------------------------------
 * Mechanisms for dealing with defaults:
 * ------------------------------------------------------------------------*/

Bool resolveDefs(List vs, Bool interactive) /* Attempt to resolve defaults */
                                            /* for variables vs subject to  */
{ /* constraints in preds             */
    List pvs = NIL;
    List qs = preds;
    Bool defaulted = FALSE;

    resetGenerics(); /* find type variables in ps       */
    for (; nonNull(qs); qs = tl(qs)) {
        Cell pi = fst3(hd(qs));
        Int o = intOf(snd3(hd(qs)));
        for (; isAp(pi); pi = fun(pi))
            pvs = genvarType(arg(pi), o, pvs);
    }

    for (; nonNull(pvs); pvs = tl(pvs)) { /* now try defaults           */
        Int vn = intOf(hd(pvs));

        if (!intIsMember(vn, vs))
            defaulted |= resolveVar(vn, interactive);
    }

    return defaulted;
}

static Bool local resolveVar(Int vn,
                             Bool interactive) /* Determine whether an ambig. */
/* variable vn can be resolved  */
{ /* by default in the context of */
    List ps = preds;
    List cs = NIL; /* the predicates in ps              */
    Bool aNumClass = FALSE;

    if (tyvar(vn)->bound == SKOLEM)
        return FALSE;

    /* According to the Haskell definition, we can only default an ambiguous
     * variable if the set of classes that constrain it:
     *   (a) includes at least one numeric class.
     *       (However if interactive is TRUE, we also allow Show, Eq or Ord)
     *   (b) includes only numeric or standard classes.
     * In addition, we will not allow a variable to be defaulted unless it
     * appears only in predicates of the form (Class var).
     */

    for (; nonNull(ps); ps = tl(ps)) {
        Cell pi = hd(ps);
        Class c = classConstraining(vn, fst3(pi), intOf(snd3(pi)));
        if (nonNull(c)) {
            if (c == classRealFrac || c == classRealFloat ||
                c == classFractional || c == classFloating || c == classReal ||
                c == classIntegral || c == classNum ||
                (interactive &&
                 (c == classEq || c == classOrd || c == classShow)))
                aNumClass = TRUE;
            else if (c != classEq && c != classOrd && c != classShow &&
                     c != classRead && c != classIx && c != classEnum &&
                     c != classBounded)
                return FALSE;

            {
                Type t = arg(fst3(pi)); /* Check for single var as arg     */
                Int o = intOf(snd3(pi));
                Tyvar *tyv;
                tyv = deRef(&t, &o);
                if (!tyv || tyvNum(tyv) != vn)
                    return FALSE;
            }
            if (!cellIsMember(c, cs))
                cs = cons(c, cs);
        }
    }

    /* Now find the first class (if any) in the list of defaults that
     * is an instance of all of the required classes.
     *
     * If we get this far, then cs only mentions classes from the list
     * above, all of which have only a single parameter of kind *.
     */

    if (aNumClass) {
        List ds = defaultDefns; /* N.B. guaranteed to be monotypes */
        for (; nonNull(ds); ds = tl(ds)) {
            List cs1 = cs;
            while (nonNull(cs1) &&
                   nonNull(entail(NIL, ap(hd(cs1), hd(ds)), 0, 0)))
                cs1 = tl(cs1);
            if (isNull(cs1)) {
                bindTv(vn, hd(ds), 0);
                return TRUE;
            }
        }
    }

    return FALSE;
}

static Class local classConstraining(Int vn, Cell pi, Int o)
/* Return class constraining var*/
/* vn in predicate pi, or NIL if*/
{ /* vn is not involved         */
    for (; isAp(pi); pi = fun(pi))
        if (!doesntOccurIn(tyvar(vn), arg(pi), o))
            return getHead(pi);
    return NIL;
}

/*-------------------------------------------------------------------------*/
