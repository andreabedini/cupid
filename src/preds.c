/* --------------------------------------------------------------------------
 * Part of the type checker dealing with predicates and entailment
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
#include "output.h" /* printContext, for the +x trace */
#include "subst.h"
#include "opts.h" /* allowOverlap, showInstRes */
#include "preds.h"
#include "tagcheck.h"

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static void overEvid(Cell, Cell);

static void cutoffExceeded(Cell, int, List);
static Cell scFind(Cell, Cell, int, Cell, int, int);
static Cell entail(List, Cell, int, int);

static bool anyGenerics(Type, int);

static bool resolveVar(int, bool);
static Class classConstraining(int, Cell, int);

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

/* --------------------------------------------------------------------------
 * Equality predicates:
 *
 * A predicate t1 ~ t2 holds when its two sides are the same type.  It has
 * no evidence: the dictionary-passing translation never sees one, so it
 * never enters preds and never becomes a parameter or an argument.  Taken
 * as given, in the context of a signature or an instance, it is applied by
 * unifying its sides before the body is checked (assumeGivens).  Wanted,
 * at a use of a name whose type has one or of an instance whose context
 * has one, it is solved by unifying its sides as soon as it arises
 * (assumeEvid, inEntail): nothing can make an equality hold later that
 * does not hold now, so there is nothing to defer.
 * ------------------------------------------------------------------------*/

/* Solve the equality pi, read at offset o, by unifying its sides.  One
 * that does not unify is an error, at the line of the signature or
 * instance it was written in, or 0 for the type of a name being used. */
static void solveEquality(int line, Cell pi, int o) {
    if (!unify(arg(fun(pi)), o, arg(pi), o)) {
        clearMarks();
        errHead(line);
        errPrintf("Equality constraint cannot hold");
        errPrintf("\n*** Constraint     : ");
        errPred(copyPred(pi, o));
        if (unifyFails)
            errPrintf("\n*** Because        : %s", unifyFails);
        errPrintf("\n");
        errFail();
    }
}

/* Add predicate pi (offset o) to preds with new dict var nd, which is
 * returned to be put where the dictionary argument goes.  An equality has
 * no dictionary: it is solved on the spot and NIL is returned. */
Cell assumeEvid(Cell pi, int o) {
    assertPred(pi);
    if (isEquality(pi)) {
        solveEquality(0, pi, o);
        return NIL;
    }
    Cell nd = inventDictVar();
    preds = cons(triple(pi, mkInt(o), nd), preds);
    return nd;
}

/* Apply the builder of instance in to the dictionaries for its context,
 * read at offset beta where findInstFor matched it, each a new wanted; an
 * equality in the context is solved and takes no argument.  Each wanted is
 * first improved by the functional dependencies, as entail does: a context
 * variable not in the head, such as r in
 *     instance Generic t r => Datatype (MetaData t),
 * is only fixed that way, and without it the wanted would match no
 * instance. */
static Cell applyToEvid(Inst in, int beta) {
    Cell e = instRec(in)->builder;
    List qs = instRec(in)->specifics;
    for (; nonNull(qs); qs = tl(qs))
        if (isEquality(hd(qs))) {
            solveEquality(instRec(in)->line, hd(qs), beta);
        } else {
            improve1(instRec(in)->line, NIL, hd(qs), beta);
            e = ap(e, assumeEvid(hd(qs), beta));
        }
    return e;
}

/* --------------------------------------------------------------------------
 * Typeable:
 *
 * As in GHC, a type built from type constructors is an instance of
 * Data.Typeable's Typeable without any instance declaration.  When no
 * instance matches Typeable t, its evidence is the class's dictionary
 * constructor applied to (constRep r), r being an expression for the
 * TypeRep of t, built from t's structure:
 *
 *   rep (T t1 ... tn) = mkTyConApp (mkTyConQ "M" "T") [rep t1, ..., rep tn]
 *   rep u             = typeOf ev []   for an argument u of kind *, ev
 *                                      being the evidence for Typeable u
 *
 * An argument of kind * is solved as a Typeable predicate of its own, by an
 * instance if one matches it, and is the only place a type variable may be:
 * one of another kind has no TypeRep, and leaves the predicate unsolved.
 * The argument given to typeOf is never looked at.
 * ------------------------------------------------------------------------*/

/* How typeableRep solves the Typeable predicates of arguments of kind * */
typedef enum {
    TYPEABLE_CHECK,  /* not at all: only find whether the rep can be built */
    TYPEABLE_ENTAIL, /* by entailment from the given predicates            */
    TYPEABLE_REDUCE  /* by making each one a new wanted                    */
} TypeableMode;

typedef struct {
    Class c; /* Typeable                                    */
    TypeableMode mode;
    List ps; /* the given predicates, for TYPEABLE_ENTAIL   */
    int d;   /* the depth of entailment                     */
} TypeableSolve;

/* Is c the Typeable class of Data.Typeable? */
static bool isTypeableClass(Class c) {
    return cclassRec(c)->text == findText("Typeable") &&
           moduleRec(cclassRec(c)->mod)->text == findText("Data.Typeable");
}

/* The name s defined in the module of class c */
static Name typeableName(Class c, char *s) {
    Text t = findText(s);
    List ns = moduleRec(cclassRec(c)->mod)->names;
    for (; nonNull(ns); ns = tl(ns))
        if (nameRec(hd(ns))->text == t)
            return hd(ns);
    errHead(0);
    errPrintf("Cannot solve Typeable: no \"%s\" in module \"%s\"", s,
              textToStr(moduleRec(cclassRec(c)->mod)->text));
    errFail();
}

/* An expression for the TyCon of the type constructor h, named as
 * Data.Typeable names those of the Prelude it has instances for */
static Cell typeableTyCon(Class c, Cell h) {
    char *m = "";
    char commas[NUM_TUPLES + 1];
    char *n;
    if (isTuple(h)) {
        int i = 0;
        for (; i < tupleOf(h) - 1 && i < NUM_TUPLES; i++)
            commas[i] = ',';
        commas[i] = '\0';
        n = commas;
    } else if (h == typeArrow) {
        n = "->";
    } else {
        n = textToStr(tyconRec(h)->text);
        if (h != typeList && h != typeUnit)
            m = textToStr(moduleRec(tyconRec(h)->mod)->text);
    }
    return ap2(typeableName(c, "mkTyConQ"), mkStr(findText(m)),
               mkStr(findText(n)));
}

/* An expression for the TypeRep of t (offset o), or NIL if there is none;
 * top is set for the type of the predicate, which has kind *, and is
 * built from its structure whatever its kind.  In TYPEABLE_CHECK mode the
 * result only tells whether there would be one. */
static Cell typeableRep(TypeableSolve *s, Type t, int o, bool top) {
    Type h;
    int a;
    List args = NIL;
    Cell rep;
    for (;;) {
        h = getDerefHead(t, o);
        a = argCount;
        if (isSynonym(h) && a >= tyconRec(h)->arity)
            expandSyn(h, a, &t, &o);
        else
            break;
    }
    if (!top && getKind(t, o) == STAR) { /* a Typeable predicate itself */
        Cell pi = ap(s->c, t);
        Cell ev;
        switch (s->mode) {
        case TYPEABLE_CHECK:
            return nameNil;
        case TYPEABLE_REDUCE:
            ev = assumeEvid(pi, o);
            break;
        default:
            ev = entail(s->ps, pi, o, s->d);
            break;
        }
        if (isNull(ev))
            return NIL;
        return ap2(hd(cclassRec(s->c)->members), ev, nameNil);
    }
    if (!isTycon(h) && !isTuple(h)) /* a type variable, or one applied   */
        return NIL;
    for (; 0 < a--; t = fun(t)) {
        Cell r;
        deRef(&t, &o);
        r = typeableRep(s, arg(t), o, false);
        if (isNull(r))
            return NIL;
        args = cons(r, args);
    }
    if (s->mode == TYPEABLE_CHECK)
        return nameNil;
    rep = nameNil;
    for (args = rev(args); nonNull(args); args = tl(args))
        rep = ap2(nameCons, hd(args), rep);
    return ap2(typeableName(s->c, "mkTyConApp"), typeableTyCon(s->c, h), rep);
}

/* The evidence for (pi,o), a Typeable predicate that no instance matched,
 * solving the predicates of its arguments of kind * as mode says, or NIL
 * if it cannot be built from the type's structure.  Nothing is solved,
 * and no new wanted made, unless the whole can be. */
static Cell typeableEvid(TypeableMode mode, List ps, Cell pi, int o, int d) {
    TypeableSolve s;
    Cell rep;
    s.c = getHead(pi);
    s.mode = TYPEABLE_CHECK;
    s.ps = ps;
    s.d = d;
    if (!isClass(s.c) || !isTypeableClass(s.c) ||
        isNull(typeableRep(&s, arg(pi), o, true)))
        return NIL;
    s.mode = mode;
    rep = typeableRep(&s, arg(pi), o, true);
    if (isNull(rep))
        return NIL;
    return ap(cclassRec(s.c)->dcon, ap(typeableName(s.c, "constRep"), rep));
}

/* Make list of predicate assumps. from qs (offset o), w/ new dict vars for
 * each predicate */
List makePredAss(List qs, int o) {
    assertPredList(qs);
    List result = NIL;
    for (; nonNull(qs); qs = tl(qs))
        result = cons(triple(hd(qs), mkInt(o), inventDictVar()), result);
    return rev(result);
}

/* The predicate assignment for the context qs, read at offset o, taken as
 * given for the body about to be checked, at line: each equality in it is
 * applied by unifying its sides, and each class predicate gets a new dict
 * var, as makePredAss gives it */
List assumeGivens(int line, List qs, int o) {
    assertPredList(qs);
    List result = NIL;
    for (; nonNull(qs); qs = tl(qs))
        if (isEquality(hd(qs)))
            solveEquality(line, hd(qs), o);
        else
            result = cons(triple(hd(qs), mkInt(o), inventDictVar()), result);
    return rev(result);
}

/* Copy list of predicates */
List copyPreds(List qs) {
    List result = NIL;
    for (; nonNull(qs); qs = tl(qs)) {
        Cell pi = hd(qs);
        result = cons(copyPred(fst3(pi), intOf(snd3(pi))), result);
    }
    return rev(result);
}

/* Add extra dictionary args to qualify alt by predicates in qs :: ([Pat],Rhs) */
void qualify(List qs, Cell alt) {
    List ds;
    for (ds = NIL; nonNull(qs); qs = tl(qs))
        ds = cons(thd3(hd(qs)), ds);
    fst(alt) = revOnto(ds, fst(alt));
}

/* Add extra dict args to each alternative in function binding */
void qualifyBinding(List qs, Cell b) {
    if (!isVar(fst(b))) /* check for function binding      */
        internal("qualifyBinding");
    map1Proc(qualify, qs, snd(snd(b)));
}

/* Add dictionary params to expr */
Cell qualifyExpr(int l, List ps, Cell e) {
    if (nonNull(ps)) {           /* Qualify input expression with   */
        if (whatIs(e) != LAMBDA) /* additional dictionary params           */
            e = ap(LAMBDA, pair(NIL, pair(mkInt(l), e)));
        qualify(ps, snd(e));
    }
    return e;
}

/* Overwrite dict var dv with evidence ev */
static void overEvid(Cell dv, Cell ev) {
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

int cutoff = 40; /* Used to limit depth of recursion*/

/* Display error msg when cutoff */
static void cutoffExceeded(Cell pi, int o, List ps) {
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

/* Use superclass entailment to find evidence for (pi,o) using the evidence e
 * for (pi1,o1). */
static Cell scFind(Cell e, Cell pi1, int o1, Cell pi, int o, int d) {
    Class h1 = getHead(pi1);
    Class h = getHead(pi);
    Cell ev = NIL;

    if (samePred(pi1, o1, pi, o))
        return e;

    if (isClass(h1) &&
        (!isClass(h) || cclassRec(h)->level < cclassRec(h1)->level)) {
        int beta = newKindedVars(cclassRec(h1)->kinds);
        List scs = cclassRec(h1)->supers;
        List dsels = cclassRec(h1)->dsels;
        List ps = NIL;
        if (!matchPred(pi1, o1, cclassRec(h1)->head, beta))
            internal("scFind");

        for (; nonNull(scs); scs = tl(scs), dsels = tl(dsels))
            ps = cons(triple(hd(scs), mkInt(beta), ap(hd(dsels), e)), ps);
        ps = rev(ps);

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
        improve1(0, ps, pi, o);
        ev = scEntail(ps, pi, o, d);
        if (showInstRes && nonNull(ev)) {
            int i;
            for (i = 0; i < d; i++)
                fputc(' ', stdout);
            fputs("scSat.\n", stdout);
        }
        return ev;
    }
    return NIL;
}

/* Calc evidence for (pi,o) from ps Using superclasses and equality. */
Cell scEntail(List ps, Cell pi, int o, int d) {
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

/* Calc evidence for (pi,o) from ps Uses superclasses, equality, tautology, and
 * construction */
static Cell entail(List ps, Cell pi, int o, int d) {
    assertPred(pi);
    Cell ev = NIL;

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

    ev = scEntail(ps, pi, o, d);
    if (nonNull(ev)) {
        if (showInstRes) {
            int i;
            for (i = 0; i < d; i++)
                fputc(' ', stdout);
            fputs("scSat.\n", stdout);
        }
    } else {
        ev = inEntail(ps, pi, o, d);
        if (nonNull(ev) && showInstRes) {
            int i;
            for (i = 0; i < d; i++)
                fputc(' ', stdout);
            fputs("inSat.\n", stdout);
        }
    }
    return ev;
}

/* Calc evidence for (pi,o) from ps using a top-level instance entailment */
Cell inEntail(List ps, Cell pi, int o, int d) {
    int i;
    Inst in;
    int beta;

    if (d++ >= cutoff)
        cutoffExceeded(pi, o, ps);

    in = findInstFor(pi, o, &beta); /* Class predicates                   */
    if (nonNull(in)) {
        Cell e = instRec(in)->builder;
        List es = instRec(in)->specifics;
        List fs = NIL;
        for (; nonNull(es); es = tl(es))
            fs = cons(triple(hd(es), mkInt(beta), NIL), fs);
        fs = rev(fs);
        improve(0, ps, fs);
        if (showInstRes) {
            for (i = 0; i < d; i++)
                fputc(' ', stdout);
            fputs("try ", stdout);
            printContext(stdout, copyPreds(fs));
            fputs(" => ", stdout);
            printPred(stdout, copyPred(instRec(in)->head, beta));
            fputc('\n', stdout);
        }
        for (es = instRec(in)->specifics; nonNull(es); es = tl(es)) {
            Cell ev;
            if (isEquality(hd(es))) { /* solved, and no argument      */
                solveEquality(instRec(in)->line, hd(es), beta);
                continue;
            }
            improve1(0, ps, hd(es), beta);
            ev = entail(ps, hd(es), beta, d);
            if (nonNull(ev))
                e = ap(e, ev);
            else
                return NIL;
        }
        return e;
    } else {
        Cell ev = typeableEvid(TYPEABLE_ENTAIL, ps, pi, o, d);
        if (nonNull(ev))
            return ev;
        if (showInstRes) {
            for (i = 0; i < d; i++)
                fputc(' ', stdout);
            fputs("No instance found for ", stdout);
            printPred(stdout, copyPred(pi, o));
            fputc('\n', stdout);
        }
    }
    return NIL;
}

/* Find evidence for predicate pi assuming ps. If ps is null, then we get to
 * decide whether is tautological, and we can use */
Cell provePred(Kinds ks, List ps, Cell pi) {
    assertKinds(ks);
    assertPred(pi);
    int beta; /* the evidence as a dictionary.   */
    Cell ev;
    emptySubstitution();
    beta = newKindedVars(ks);   /* (ks provides kinds for any      */
    ps = makePredAss(ps, beta); /*  vars that appear in pi.)       */
    ev = entail(ps, pi, beta, 0);
    emptySubstitution();
    return ev;
}

/* Find evidence for predicate pi. Unlike `provePred', also uses desparate
 * measures like context */
Cell resolvePred(Kinds ks, Cell pi) {
    assertKinds(ks);
    assertPred(pi);
    int beta; /* reduction and defaulting.       */
    List qs;

    emptySubstitution();
    beta = newKindedVars(ks);
    qs = makePredAss(cons(pi, NIL), beta);
    preds = qs;
    elimTauts();
    if (resolveDefs(NIL, true))
        elimTauts();
    emptySubstitution();
    return (nonNull(preds) ? NIL : thd3(hd(qs)));
}

/* --------------------------------------------------------------------------
 * Predicate set Simplification:
 *
 * Calculate a minimal equivalent subset of a given set of predicates.
 * ------------------------------------------------------------------------*/

/* Simplify predicates in qs, returning equiv minimal subset */
List scSimplify(List qs) {
    int n = length(qs);

    while (0 < n--) {
        Cell pi = hd(qs);
        Cell ev = NIL;
        if (showInstRes) {
            fputs("scSimplify: ", stdout);
            printContext(stdout, copyPreds(tl(qs)));
            fputs(" ||- ", stdout);
            printPred(stdout, copyPred(fst3(pi), intOf(snd3(pi))));
            fputc('\n', stdout);
        }
        ev = scEntail(tl(qs), fst3(pi), intOf(snd3(pi)), 0);
        if (nonNull(ev)) {
            if (showInstRes)
                fputs("Simplified!\n", stdout);
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

/* Simplify context of skeletons skeletons, offset o, using superclass
 * hierarchy */
List simpleContext(List ps, int o) {
    return copyPreds(scSimplify(makePredAss(ps, o)));
}

/* --------------------------------------------------------------------------
 * Context splitting --- tautological and locally tautological predicates:
 * ------------------------------------------------------------------------*/

/* Remove tautological constraints */
void elimTauts(void) {
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

static int numFixedVars = 0; /* Number of fixed vars found         */

/* Test for generic vars, and count fixed variables */
static bool anyGenerics(Type t, int o) {
    Type h = getDerefHead(t, o); /* This code is careful to expand  */
    int a = argCount;            /* synonyms; mark* & copy* do not. */
    if (isSynonym(h) && a >= tyconRec(h)->arity) {
        expandSyn(h, a, &t, &o);
        return anyGenerics(t, o);
    } else {
        Tyvar *tyv;
        for (; 0 < a--; t = fun(t)) { /* cycle through any arguments       */
            tyv = deRef(&t, &o);
            if (anyGenerics(arg(t), o))
                return true;
        }
        tyv = deRef(&t, &o);
        if (tyv)
            if (tyv->offs == FIXED_TYVAR) {
                numFixedVars++;
                return false;
            } else
                return true;
        else
            return false;
    }
}

/* Simplify and defer any remaining preds that contain no generics. */
List elimOuterPreds(List sps) {
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

/* Try to discharge or defer preds, splitting if necessary to match context ps.
 * sps = savePreds. */
List elimPredsUsing(List ps, List sps) {
    List rems = NIL;
    while (nonNull(preds)) { /* Pick a predicate from preds        */
        Cell p = preds;
        Cell pi = fst3(hd(p));
        int o = intOf(snd3(hd(p)));
        Cell ev = entail(ps, pi, o, 0);
        preds = tl(preds);

        if (nonNull(ev)) { /* Discharge if ps ||- (pi,o)           */
            overEvid(thd3(hd(p)), ev);
        } else if (!isAp(pi) || !anyGenerics(pi, o)) {
            tl(p) = sps; /* Defer if no generics                   */
            sps = p;
        } else { /* Try to split generics and fixed */
            Inst in;
            int beta;
            if (numFixedVars > 0 && nonNull(in = findInstFor(pi, o, &beta))) {
                ev = applyToEvid(in, beta);
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

/* Context reduce predicates: uggh! */
void reducePreds(void) {
    List rems = NIL;         /* (A last resort for defaulting)  */
    while (nonNull(preds)) { /* Pick a predicate from preds        */
        Cell p = preds;
        Cell pi = fst3(hd(p));
        int o = intOf(snd3(hd(p)));
        int beta;
        Inst in = findInstFor(pi, o, &beta);
        preds = tl(preds);
        if (nonNull(in)) {
            Cell ev = applyToEvid(in, beta);
            overEvid(thd3(hd(p)), ev);
        } else {
            Cell ev = typeableEvid(TYPEABLE_REDUCE, NIL, pi, o, 0);
            if (nonNull(ev)) {
                overEvid(thd3(hd(p)), ev);
            } else { /* No worthwhile progress possible */
                tl(p) = rems;
                rems = p;
            }
        }
    }
    preds = scSimplify(rems); /* Return any remaining predicates */
}

/* Normalize each element of preds in some appropriate manner */
void normPreds(int line) {
}

/* --------------------------------------------------------------------------
 * Mechanisms for dealing with defaults:
 * ------------------------------------------------------------------------*/

/* Attempt to resolve defaults for variables vs subject to constraints in preds */
bool resolveDefs(List vs, bool interactive) {
    List pvs = NIL;
    List qs = preds;
    bool defaulted = false;

    resetGenerics(); /* find type variables in ps       */
    for (; nonNull(qs); qs = tl(qs)) {
        Cell pi = fst3(hd(qs));
        int o = intOf(snd3(hd(qs)));
        for (; isAp(pi); pi = fun(pi))
            pvs = genvarType(arg(pi), o, pvs);
    }

    for (; nonNull(pvs); pvs = tl(pvs)) { /* now try defaults           */
        int vn = intOf(hd(pvs));

        if (!intIsMember(vn, vs))
            defaulted |= resolveVar(vn, interactive);
    }

    return defaulted;
}

/* Determine whether an ambig. variable vn can be resolved by default in the
 * context of the predicates in ps */
static bool resolveVar(int vn, bool interactive) {
    List ps = preds;
    List cs = NIL;
    bool aNumClass = false;

    if (tyvar(vn)->bound == SKOLEM)
        return false;

    /* According to the Haskell definition, we can only default an ambiguous
     * variable if the set of classes that constrain it:
     *   (a) includes at least one numeric class.
     *       (However if interactive is true, we also allow Show, Eq or Ord)
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
                aNumClass = true;
            else if (c != classEq && c != classOrd && c != classShow &&
                     c != classRead && c != classIx && c != classEnum &&
                     c != classBounded)
                return false;

            {
                Type t = arg(fst3(pi)); /* Check for single var as arg     */
                int o = intOf(snd3(pi));
                Tyvar *tyv;
                tyv = deRef(&t, &o);
                if (!tyv || tyvNum(tyv) != vn)
                    return false;
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
                return true;
            }
        }
    }

    return false;
}

/* Return class constraining var vn in predicate pi, or NIL if vn is not
 * involved */
static Class classConstraining(int vn, Cell pi, int o) {
    for (; isAp(pi); pi = fun(pi))
        if (!doesntOccurIn(tyvar(vn), arg(pi), o))
            return getHead(pi);
    return NIL;
}

/*-------------------------------------------------------------------------*/
