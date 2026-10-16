/* --------------------------------------------------------------------------
 * Derived instances: working out their contexts and generating their code.
 *
 * A "deriving" clause asks for an instance the programmer did not write.
 * This file decides what context such an instance needs, by iterating to a
 * fixed point over the requests, and then writes the bindings for it: Eq,
 * Ord, Enum, Ix, Show, Read, Bounded and Generic.  tycon.c collects the requests
 * and hands them over, and class.c hands over each standalone deriving
 * declaration once it has checked it as an instance.
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
#include "typeexp.h"
#include "deriving.h"

static void deriveContexts(List);
static void addDerivImp(Inst);

static Inst addDerInst(int, Class, List, List, Type, int);
static void initDerInst(Inst);
static void calcInstPreds(Inst);
static void maybeAddPred(Cell, int, int, List);
static bool smallPred(Cell, int);
static bool smallType(Type, int, List *);
static Cell copyAdj(Cell, int, int);
static void tidyDerInst(Inst);
static List freshVars(int);
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
static bool isGenericClass(Class);
static bool isTypeableClass(Class);
static bool isDataClass(Class);
static List deriveData(Inst, Tycon);
static Name dataName(Class, char *);
static Cell dataTypeExpr(Class, Tycon);
static List deriveGeneric(Inst, Type);
static Cell genTycon(char *);
static Name genName(char *);
static List conFieldTypes(Name);
static Type genRep(Tycon);
static Type genConRep(Type, Name, int);
static Cell genIndex(int);
static Cell genBalanced(Cell, List, int);
static Cell genInject(Cell, int, int);
static Cell genValue(Name, List, int, int);
static Cell genDataInfo(Tycon);
static Cell genConInfo(Name);
static List stockBindings(Inst, Type);
static void viaHead(Inst);
static void checkVia(Inst);
static Type runtimeType(Type);
static Type unwrapHead(Type, bool);
static Type unwrapOnce(Type, bool);
static Type etaReduce(Tycon, int, List);
static bool hasOffset(Type, int);
static Type substOffsets(Type, List);
static bool sameRep(Type, Type);
static bool holdsEvidence(Type, List *);
static bool viaReusable(Class);

/* --------------------------------------------------------------------------
 * Process derived instance requests:
 * ------------------------------------------------------------------------*/

static List derivedInsts; /* list of derived instances     */

/* Verify derived instance request for tycon t, with explicit context p,
 * component types ts and named class ct */
void checkDerive(Tycon t, List p, List ts, Cell ct) {
    int line = tyconRec(t)->line;
    Type via = NIL;
    bool isVia = whatIs(ct) == DERIVE; /* DERIVE (class, via): see parser.y */
    Class c;
    if (isVia) {
        via = snd(snd(ct));
        ct = fst(snd(ct));
    }
    c = findQualClass(ct);
    if (isNull(c)) {
        errHead(line);
        errPrintf("Unknown class \"%s\" in derived instance", identToStr(ct));
        errFail();
    }
    if (isTypeableClass(c)) { /* every type is, under its own name: see  */
        return;               /* preds.c; a via type would lend its name */
    }
    if (isVia) {
        if (isNull(via)) { /* deriving newtype: via what it wraps */
            if (tyconRec(t)->what != NEWTYPE) {
                errHead(line);
                errPrintf("Cannot derive \"%s\" for \"%s\" with the newtype "
                          "strategy: it is not a newtype",
                          textToStr(cclassRec(c)->text),
                          textToStr(tyconRec(t)->text));
                errFail();
            }
            instRec(addDerInst(line, c, p, singleton(hd(ts)), t,
                               tyconRec(t)->arity))
                ->via = NEWTYPE; /* viaHead finds the type, as only it knows */
            return;              /* how many parameters the class leaves off */
        }
        instRec(addDerInst(line, c, p, singleton(via), t, tyconRec(t)->arity))
            ->via = via;
        return;
    }
    if (isGenericClass(c)) { /* a representation needs no context */
        ts = NIL;
    }
    addDerInst(line, c, p, dupList(ts), t, tyconRec(t)->arity);
}

/* Add a derived instance */
static Inst addDerInst(int line, Class c, List p, List cts, Type t, int a) {
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
    return in;
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
 * stage.  A user who wants another context can write it in a standalone
 * deriving declaration.  We eliminate tautological predicates, and, as GHC
 * does, only allow a predicate to appear in the final result if it is a
 * class applied to types made only of distinct type variables: C (f a)
 * may, but C (f [a]), C (f Int) and C (f a a) may not.  See smallPred.
 *
 * In general, we have to deal with mutually recursive instance declarations.
 * We find a solution in the obvious way by iterating to find a fixed point.
 * Each iteration but the last adds a predicate to some context, and there
 * are only finitely many such predicates over an instance's variables, so
 * this terminates; reduction through the instances used is bounded by the
 * cutoff.  Without the restriction it need not: the context of
 *
 *    data T f a = T (f a) (T f [a]) deriving Show
 *
 * would gain Show (f [a]), Show (f [[a]]), ... one per iteration.
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
        hd(spcs) = triple(ap(instRec(in)->c, hd(spcs)), mkInt(beta), mkInt(0));
    }
    instRec(in)->numSpecifics = beta;
}

/* Calculate next approximation of the context for a derived instance.  The
 * work list ps holds class predicates, as triples (pi,o,d) where d is the
 * depth of reduction pi was reached at, and delayed substitutions.  A
 * reduction that does not terminate goes ever deeper, so it is the depth
 * that is bounded by the cutoff, not the number of predicates: reducing
 * through a generic representation is wide but shallow. */
static void calcInstPreds(Inst in) {
    List retain = NIL;
    List ps = snd(snd(instRec(in)->specifics));
    List spcs = fst(snd(instRec(in)->specifics));
    int beta = instRec(in)->numSpecifics;

    while (nonNull(ps)) {
        Cell p = hd(ps);
        ps = tl(ps);

        if (isInt(fst(p))) { /* Delayed substitution?   */
            List qs = snd(p);
            for (; nonNull(hd(qs)); qs = tl(qs)) {
                ps = cons(triple(hd(qs), fst(p), mkInt(0)), ps);
            }
            retain = cons(pair(fst(p), qs), retain);
        } else { /* Class predicate        */
            Cell pi = fst3(p);
            int o = intOf(snd3(p));
            int d = intOf(thd3(p));
            int at;
            Inst in1;
            if (d >= cutoff) {
                Cell bpi = instRec(in)->head;
                errHead(instRec(in)->line);
                errPrintf("\n*** Cannot derive ");
                errPred(bpi);
                errPrintf(": its context is still reducing at depth %d.", d);
                errPrintf(
                    "\n*** This may indicate that the problem is undecidable.  However,\n");
                errPrintf(
                    "*** you may still try to increase the cutoff limit using the -c\n");
                errPrintf(
                    "*** option and then try again.  (The current setting is -c%d)\n",
                    cutoff);
                errFail();
            }
            if (isEquality(pi)) { /* from the context of an instance used */
                Cell bpi = instRec(in)->head;
                pi = copyPred(pi, o);
                errHead(instRec(in)->line);
                errPrintf("Cannot derive ");
                errPred(bpi);
                errPrintf(": an instance it needs has the equality ");
                errPred(pi);
                errPrintf(" in its context\n");
                errFail();
            }
            improve1(instRec(in)->line, NIL, pi, o); /* as applyToEvid */
            in1 = findInstFor(pi, o, &at);
            if (nonNull(in1)) {
                List qs = instRec(in1)->specifics;
                int off = mkInt(at);
                if (whatIs(qs) == DERIVE) { /* Still being derived         */
                    for (qs = fst(snd(qs)); nonNull(hd(qs)); qs = tl(qs)) {
                        ps = cons(triple(hd(qs), off, mkInt(d + 1)), ps);
                    }
                    retain = cons(pair(off, qs), retain);
                } else {           /* Previously def'd inst   */
                    List qs1 = qs; /* improve the whole context first: */
                    for (; nonNull(qs1); qs1 = tl(qs1)) { /* one of its  */
                        if (!isEquality(hd(qs1))) {       /* preds may fix a */
                            improve1(instRec(in)->line, NIL, hd(qs1), at);
                        } /* variable of another, as Generic a r fixes */
                    } /* the r of GEq r                              */
                    for (; nonNull(qs); qs = tl(qs)) {
                        ps = cons(triple(hd(qs), off, mkInt(d + 1)), ps);
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
                } else if (!smallPred(pi, o)) {
                    Cell bpi = instRec(in)->head;
                    pi = copyPred(pi, o);
                    errHead(instRec(in)->line);
                    errPrintf("Cannot derive ");
                    errPred(bpi);
                    errPrintf(": its context would need ");
                    errPred(pi);
                    errPrintf(", which is not a class applied to distinct type "
                              "variables\n");
                    errPrintf("*** Give the context yourself in a standalone "
                              "declaration: deriving instance ... => ");
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

/* Is the predicate (pi,o) a class applied to types made only of distinct
 * type variables, as C (f a) and C (f (g a)) b are and C (f [a]), C (f Int)
 * and C (f a a) are not?  Only such a predicate may be in a derived context,
 * as in GHC (validDerivPred): no instance can reduce it, and a context with
 * others in it may grow without end, as the one of
 * data T f a = T (f a) (T f [a]) would. */
static bool smallPred(Cell pi, int o) {
    List seen = NIL;
    for (; isAp(pi); pi = fun(pi)) {
        if (!smallType(arg(pi), o, &seen)) {
            return false;
        }
    }
    return true;
}

/* Is the type (t,o) made only of type variables by application, none of
 * them in seen, to which those it has are added? */
static bool smallType(Type t, int o, List *seen) {
    Tyvar *tyv = deRef(&t, &o);
    if (tyv) {
        int vn = tyvNum(tyv);
        if (nonNull(intIsMember(vn, *seen))) {
            return false;
        }
        *seen = cons(mkInt(vn), *seen);
        return true;
    }
    return isAp(t) && smallType(fun(t), o, seen) && smallType(arg(t), o, seen);
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
    if (isTycon(t) && !hasCfun(tyconRec(t)->defn) && !isGenericClass(c)) {
        errHead(instRec(in)->line);
        errPrintf("Cannot derive \"%s\" for type \"%s\", which has no "
                  "constructors",
                  textToStr(cclassRec(c)->text), textToStr(tyconRec(t)->text));
        errFail();
    }
    if (nonNull(instRec(in)->via)) { /* typeInstDefn builds it */
        viaHead(in);
        checkVia(in);
    } else {
        imp = stockBindings(in, t);
    }

    kindInst(in, intOf(instRec(in)->kinds));
    insertInst(in);
    instRec(in)->builder = newInstImp(in);
    instRec(in)->implements =
        classBindings("derived instance", instRec(in)->c, imp);
}

/* The bindings of the instance in of a class with a standard deriver, for
 * the type t */
static List stockBindings(Inst in, Type t) {
    Class c = instRec(in)->c;
    if (c == classEq) {
        return deriveEq(t);
    } else if (c == classOrd) {
        return deriveOrd(t);
    } else if (c == classEnum) {
        return deriveEnum(t);
    } else if (c == classIx) {
        return deriveIx(t);
    } else if (c == classShow) {
        return deriveShow(t);
    } else if (c == classRead) {
        return deriveRead(t);
    } else if (c == classBounded) {
        return deriveBounded(t);
    } else if (isGenericClass(c)) {
        return deriveGeneric(in, t);
    } else if (isDataClass(c)) {
        return deriveData(in, t);
    }
    errHead(instRec(in)->line);
    errPrintf("Cannot derive instances of class \"%s\"",
              textToStr(cclassRec(c)->text));
    errFail();
}

/* --------------------------------------------------------------------------
 * Standalone deriving.  A declaration
 *
 *   deriving instance Show a => Show (T a)
 *   deriving newtype instance Num Age
 *   deriving via (LiftingAccum MaybeT m) instance
 *       MonadAccum w m => MonadAccum w (MaybeT m)
 *   deriving anyclass instance C T
 *
 * is an instance declaration whose context is written out, so nothing
 * needs inferring: the parser makes it an instance with no bindings, and
 * marks a stock one by DERIVE in place of its bindings and a newtype one
 * by NEWTYPE as its via type.  checkInstDefn checks it as any instance,
 * then hands it here for the bindings or the via type.  A via type stands
 * for the class's last parameter, so a class of several parameters can be
 * derived this way too.  An anyclass one is just an instance with no
 * bindings, and one of Typeable is dropped, as every type is Typeable.
 * ------------------------------------------------------------------------*/

/* The derived bindings of the standalone deriving declaration in, whose
 * head and context are checked and whose type variables are tyvars; for
 * one derived via a type, check that type and return no bindings */
List standaloneDeriving(Inst in, List tyvars) {
    int line = instRec(in)->line;
    Class c = instRec(in)->c;
    Type self;
    Type t;

    if (!isAp(instRec(in)->head)) {
        errHead(line);
        errPrintf("Cannot derive \"%s\": it has no parameters",
                  textToStr(cclassRec(c)->text));
        errFail();
    }
    self = arg(instRec(in)->head);
    if (instRec(in)->via == NEWTYPE) { /* via the type the newtype wraps */
        Type u = unwrapHead(self, false);
        Cell h = getHead(u);
        if (!isNewtype(h) || argCount != tyconRec(h)->arity) {
            errHead(line);
            errPrintf("Cannot derive ");
            errPred(instRec(in)->head);
            errPrintf(" with the newtype strategy: ");
            errType(self);
            errPrintf(" is not a newtype applied to all its parameters");
            errFail();
        }
        instRec(in)->via = unwrapOnce(u, true);
    } else if (nonNull(instRec(in)->via)) {
        instRec(in)->via = depTypeExp(line, tyvars, instRec(in)->via);
    }
    if (nonNull(instRec(in)->via)) {
        checkVia(in);
        return NIL;
    }

    t = getHead(self);
    if (isSynonym(t)) {
        errHead(line);
        errPrintf("Cannot derive ");
        errPred(instRec(in)->head);
        errPrintf(": \"%s\" is a type synonym", textToStr(tyconRec(t)->text));
        errFail();
    }
    if (!isTycon(t) || !hasCfun(tyconRec(t)->defn)) {
        errHead(line);
        errPrintf("Cannot derive ");
        errPred(instRec(in)->head);
        errPrintf(": the type has no constructors to derive it from");
        errFail();
    }
    if (isGenericClass(c)) {
        errHead(line);
        errPrintf("Cannot derive \"Generic\" in a standalone declaration: "
                  "derive it in the declaration of \"%s\"",
                  textToStr(tyconRec(t)->text));
        errFail();
    }
    return stockBindings(in, t);
}

/* The instances is with the standalone deriving declarations of Typeable
 * left out, whatever their strategy: one via another type would give the
 * type that one's name */
List dropTypeableDerivings(List is) {
    List rs = NIL;
    for (; nonNull(is); is = tl(is)) {
        Inst in = hd(is);
        Class c = NIL;
        if (instRec(in)->implements == DERIVE || nonNull(instRec(in)->via)) {
            Cell h = getHead(instRec(in)->head);
            if (isQCon(h)) {
                c = findQualClass(h);
            }
        }
        if (isNull(c) || !isTypeableClass(c)) {
            rs = cons(in, rs);
        }
    }
    return rev(rs);
}

/* A list of n fresh vars for derived instance generation */
static List freshVars(int n) {
    List vs = NIL;
    for (; n > 0; n--) {
        vs = cons(inventVar(), vs);
    }
    return vs;
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
    List us = freshVars(2 * n);
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
    List vs = freshVars(3 * n);
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
    List vs = freshVars(a + 1);
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

/* --------------------------------------------------------------------------
 * Deriving Generic, the class of GHC.Generics.  Given
 *
 *   data T a = A Int a | B
 *
 * the instance is
 *
 *   instance Generic (T a)
 *       (M1 D (MetaData (T a))
 *          (M1 C (MetaCons (T a) Zero)
 *               (M1 S (MetaSel (T a) Zero Zero) (K1 R Int) :*:
 *                M1 S (MetaSel (T a) Zero (Succ Zero)) (K1 R a))
 *           :+: M1 C (MetaCons (T a) (Succ Zero)) U1)) where
 *     from (A x y) = M1 (L1 (M1 (M1 (K1 x) :*: M1 (K1 y))))
 *     from B       = M1 (R1 (M1 U1))
 *     to (M1 (L1 (M1 (M1 (K1 x) :*: M1 (K1 y))))) = A x y
 *     to (M1 (R1 (M1 U1)))                          = B
 *     datatypeInfo _ = DataInfo "T" "M" False
 *                        [ConInfo "A" Prefix False ["", ""],
 *                         ConInfo "B" Prefix False []]
 *
 * The class has the representation as its second parameter, determined by
 * the first, since there are no type families, so the instance head is
 * filled in here once the constructors' types are known; and it has no
 * context, since a representation needs nothing of the types it holds.
 * Sums and products are balanced, n splitting as n/2 on the left, as in
 * GHC.  The metadata are type indices into the value of datatypeInfo, the
 * constructors and fields counted from Zero: Cupid has no type-level
 * strings.  The types and constructors used are those of the module that
 * defines the class, looked up by name.
 * ------------------------------------------------------------------------*/

static Module genModule; /* the module defining Generic, while deriving */

/* Is c the Generic class of GHC.Generics? */
/* Is c the Typeable class of Data.Typeable?  The type checker solves it
 * for every type built from type constructors, so deriving it does
 * nothing, as in GHC. */
static bool isTypeableClass(Class c) {
    return cclassRec(c)->text == findText("Typeable") &&
           moduleRec(cclassRec(c)->mod)->text == findText("Data.Typeable");
}

/* --------------------------------------------------------------------------
 * Deriving Data, the class of Data.Data, for
 *
 *   data T a = C1 a Int | C2 | ...
 *
 * gives
 *
 *   gfoldl k z (C1 x1 x2) = k (k (z C1) x1) x2
 *   gfoldl k z C2         = z C2
 *   gunfold k z c         = gunfoldIndex c [k (k (z C1)), z C2, ...]
 *   toConstr x            = indexConstr (dataTypeOf x) (enFrEn x + 1)
 *   dataTypeOf _          = mkDerivedDataType "M.T"
 *                               [("C1", [], False), ("C2", [], False), ...]
 *
 * where gunfoldIndex picks the alternative for the constructor's index,
 * and mkDerivedDataType builds the DataType and its constructors, each
 * with its field labels and whether it is an operator.  A type with one
 * constructor, or a newtype, has 1 for its index, without looking at x.
 * ------------------------------------------------------------------------*/

/* Is c the Data class of Data.Data? */
static bool isDataClass(Class c) {
    return cclassRec(c)->text == findText("Data") &&
           moduleRec(cclassRec(c)->mod)->text == findText("Data.Data");
}

/* The name s defined in the module of class c, which is Data.Data */
static Name dataName(Class c, char *s) {
    Text t = findText(s);
    List ns = moduleRec(cclassRec(c)->mod)->names;
    for (; nonNull(ns); ns = tl(ns)) {
        if (nameRec(hd(ns))->text == t) {
            return hd(ns);
        }
    }
    errHead(0);
    errPrintf("Cannot derive Data: no \"%s\" in module \"%s\"", s,
              textToStr(moduleRec(cclassRec(c)->mod)->text));
    errFail();
}

/* The DataType of t, for its derived dataTypeOf */
static Cell dataTypeExpr(Class c, Tycon t) {
    char name[FILENAME_MAX + 1];
    List cs = tyconRec(t)->defn;
    List rcs = NIL;
    Cell infos = nameNil;
    snprintf(name, sizeof(name), "%s.%s",
             textToStr(moduleRec(tyconRec(t)->mod)->text),
             textToStr(tyconRec(t)->text));
    for (; hasCfun(cs); cs = tl(cs)) {
        rcs = cons(hd(cs), rcs);
    }
    for (; nonNull(rcs); rcs = tl(rcs)) {
        Name con = hd(rcs);
        List cfs = cfunSfuns;
        Cell labels = nameNil;
        for (; nonNull(cfs) && con != fst(hd(cfs)); cfs = tl(cfs)) {
        }
        if (nonNull(cfs)) {
            List ls = rev(dupList(snd(hd(cfs))));
            for (; nonNull(ls); ls = tl(ls)) {
                labels = ap2(nameCons, mkStr(textOf(hd(ls))), labels);
            }
        }
        infos = ap2(
            nameCons,
            ap(ap(ap(mkTuple(3), mkStr(nameRec(con)->text)), labels),
               textToStr(nameRec(con)->text)[0] == ':' ? nameTrue : nameFalse),
            infos);
    }
    return ap2(dataName(c, "mkDerivedDataType"), mkStr(findText(name)), infos);
}

/* Bindings for the derived Data instance in, for the type t */
static List deriveData(Inst in, Tycon t) {
    Class c = instRec(in)->c;
    int line = tyconRec(t)->line;
    List cs = tyconRec(t)->defn;
    List folds = NIL;
    Cell unfolds = nameNil;
    List rcs = NIL;
    List vs;
    Cell k, z, x, index;
    Name dataTypeOf = NIL;
    List ms = cclassRec(c)->members;

    if (!isTycon(t)) {
        errHead(line);
        errPrintf("Cannot derive Data for a tuple type");
        errFail();
    }
    for (; nonNull(ms); ms = tl(ms)) {
        if (nameRec(hd(ms))->text == findText("dataTypeOf")) {
            dataTypeOf = hd(ms);
        }
    }

    for (; hasCfun(cs); cs = tl(cs)) {
        Name con = hd(cs);
        int a = userArity(con);
        Cell pat = con;
        Cell body;
        int i;
        vs = freshVars(a + 2);
        k = hd(vs);
        z = hd(tl(vs));
        body = ap(z, con);
        for (vs = tl(tl(vs)), i = 0; i < a; i++, vs = tl(vs)) {
            pat = ap(pat, hd(vs));
            body = ap2(k, body, hd(vs));
        }
        folds = cons(
            pair(cons(k, cons(z, singleton(pat))), pair(mkInt(line), body)),
            folds);
        rcs = cons(con, rcs);
    }

    vs = freshVars(3);
    k = hd(vs);
    z = hd(tl(vs));
    x = hd(tl(tl(vs)));
    for (; nonNull(rcs); rcs = tl(rcs)) {
        Cell alt = ap(z, hd(rcs));
        int i;
        for (i = userArity(hd(rcs)); i > 0; i--) {
            alt = ap(k, alt);
        }
        unfolds = ap2(nameCons, alt, unfolds);
    }

    if (tyconRec(t)->what == NEWTYPE || length(tyconRec(t)->defn) == 1) {
        index = mkInt(1);
    } else {
        index = ap2(namePlus, ap(nameEnFrEn, x), mkInt(1));
    }

    return cons(
        mkBind("gfoldl", rev(folds)),
        cons(mkBind("gunfold",
                    singleton(
                        pair(cons(k, cons(z, singleton(x))),
                             pair(mkInt(line), ap2(dataName(c, "gunfoldIndex"),
                                                   x, unfolds))))),
             cons(mkBind("toConstr",
                         singleton(pair(singleton(x),
                                        pair(mkInt(line),
                                             ap2(dataName(c, "indexConstr"),
                                                 ap(dataTypeOf, x), index))))),
                  singleton(
                      mkBind("dataTypeOf",
                             singleton(pair(
                                 singleton(WILDCARD),
                                 pair(mkInt(line), dataTypeExpr(c, t)))))))));
}

static bool isGenericClass(Class c) {
    return cclassRec(c)->text == findText("Generic") &&
           moduleRec(cclassRec(c)->mod)->text == findText("GHC.Generics");
}

/* Bindings for the derived Generic instance in, for the type t; completes
 * the instance head with the representation */
static List deriveGeneric(Inst in, Type t) {
    int line = instRec(in)->line;
    List cs = tyconRec(t)->defn;
    int n = 0;
    int i = 0;
    List froms = NIL;
    List tos = NIL;
    List info;

    if (!isTycon(t)) {
        internal("deriveGeneric");
    }
    genModule = cclassRec(instRec(in)->c)->mod;
    instRec(in)->head = ap(instRec(in)->head, genRep(t));

    for (; hasCfun(cs); cs = tl(cs)) {
        n++;
    }
    if (n == 0) { /* from x = M1 (voidFrom x); to (M1 x) = voidTo x */
        Cell v = hd(freshVars(1));
        Name m1 = genName("M1");
        froms = singleton(
            pair(singleton(v),
                 pair(mkInt(line), ap(m1, ap(genName("voidFrom"), v)))));
        tos = singleton(pair(singleton(ap(m1, v)),
                             pair(mkInt(line), ap(genName("voidTo"), v))));
    }
    for (cs = tyconRec(t)->defn; hasCfun(cs); cs = tl(cs), i++) {
        Name c = hd(cs);
        int k = userArity(c);
        List us = freshVars(k);
        List vs = us;
        Cell con = c;
        Cell pat = c;
        int j = 0;
        for (; j < k; j++, us = tl(us)) {
            con = ap(con, hd(us));
            pat = ap(pat, hd(us));
        }
        froms =
            cons(pair(singleton(pat), pair(mkInt(line), genValue(c, vs, i, n))),
                 froms);
        tos =
            cons(pair(singleton(genValue(c, vs, i, n)), pair(mkInt(line), con)),
                 tos);
    }

    info = mkVarAlts(line, genDataInfo(t));
    fst(hd(info)) = singleton(WILDCARD);
    return cons(
        mkBind("from", rev(froms)),
        cons(mkBind("to", rev(tos)), cons(mkBind("datatypeInfo", info), NIL)));
}

/* The type constructor named s in the module defining Generic */
static Cell genTycon(char *s) {
    Text t = findText(s);
    List ts = moduleRec(genModule)->tycons;
    for (; nonNull(ts); ts = tl(ts)) {
        if (isTycon(hd(ts)) && tyconRec(hd(ts))->text == t) {
            return hd(ts);
        }
    }
    errHead(0);
    errPrintf("Cannot derive Generic: no type \"%s\" in module \"%s\"", s,
              textToStr(moduleRec(genModule)->text));
    errFail();
}

/* The value or constructor named s in the module defining Generic */
static Name genName(char *s) {
    Text t = findText(s);
    List ns = moduleRec(genModule)->names;
    for (; nonNull(ns); ns = tl(ns)) {
        if (nameRec(hd(ns))->text == t) {
            return hd(ns);
        }
    }
    errHead(0);
    errPrintf("Cannot derive Generic: no \"%s\" in module \"%s\"", s,
              textToStr(moduleRec(genModule)->text));
    errFail();
}

/* The types of the fields of constructor c, in order */
static List conFieldTypes(Name c) {
    Type t = nameRec(c)->type;
    List ts = NIL;
    int k = userArity(c);
    if (isPolyType(t)) {
        t = monotypeOf(t);
    }
    if (whatIs(t) == QUAL) {
        t = snd(snd(t));
    }
    for (; k > 0; k--) {
        ts = cons(arg(fun(t)), ts);
        t = arg(t);
    }
    return rev(ts);
}

/* The representation of the type constructor t */
static Type genRep(Tycon t) {
    Type self = t;
    List reps = NIL;
    List cs = tyconRec(t)->defn;
    int n = 0;
    int i = 0;
    for (; i < tyconRec(t)->arity; i++) {
        self = ap(self, mkOffset(i));
    }
    for (; hasCfun(cs); cs = tl(cs), n++) {
        reps = cons(genConRep(self, hd(cs), n), reps);
    }
    return ap(
        ap(ap(genTycon("M1"), genTycon("D")), ap(genTycon("MetaData"), self)),
        n == 0 ? genTycon("V1") : genBalanced(genTycon(":+:"), rev(reps), n));
}

/* The representation of constructor c, the ith of the type self */
static Type genConRep(Type self, Name c, int i) {
    List fs = conFieldTypes(c);
    List sels = NIL;
    int k = 0;
    for (; nonNull(fs); fs = tl(fs), k++) {
        Type meta =
            ap(ap(ap(genTycon("MetaSel"), self), genIndex(i)), genIndex(k));
        sels = cons(ap(ap(ap(genTycon("M1"), genTycon("S")), meta),
                       ap(ap(genTycon("K1"), genTycon("R")), hd(fs))),
                    sels);
    }
    return ap(ap(ap(genTycon("M1"), genTycon("C")),
                 ap(ap(genTycon("MetaCons"), self), genIndex(i))),
              k == 0 ? genTycon("U1")
                     : genBalanced(genTycon(":*:"), rev(sels), k));
}

/* The type-level number n, Succ (... (Succ Zero)) */
static Cell genIndex(int n) {
    Cell t = genTycon("Zero");
    for (; n > 0; n--) {
        t = ap(genTycon("Succ"), t);
    }
    return t;
}

/* The first n of xs, n > 0, joined by the binary op into a balanced tree;
 * used for types and values alike */
static Cell genBalanced(Cell op, List xs, int n) {
    int l = n / 2;
    List rs = xs;
    int i = 0;
    if (n == 1) {
        return hd(xs);
    }
    for (; i < l; i++) {
        rs = tl(rs);
    }
    return ap(ap(op, genBalanced(op, xs, l)), genBalanced(op, rs, n - l));
}

/* Put e in the L1 and R1 that choose the ith of n constructors */
static Cell genInject(Cell e, int i, int n) {
    int l = n / 2;
    if (n == 1) {
        return e;
    } else if (i < l) {
        return ap(genName("L1"), genInject(e, i, l));
    } else {
        return ap(genName("R1"), genInject(e, i - l, n - l));
    }
}

/* The representation of constructor c, the ith of n, applied to the
 * variables vs; built afresh for each use, as pattern or expression */
static Cell genValue(Name c, List vs, int i, int n) {
    Name m1 = genName("M1");
    int k = userArity(c);
    List fs = NIL;
    int j = 0;
    for (; j < k; j++, vs = tl(vs)) {
        fs = cons(ap(m1, ap(genName("K1"), hd(vs))), fs);
    }
    return ap(
        m1, genInject(ap(m1, k == 0 ? genName("U1")
                                    : genBalanced(genName(":*:"), rev(fs), k)),
                      i, n));
}

/* The value of datatypeInfo for the type t */
static Cell genDataInfo(Tycon t) {
    List cs = tyconRec(t)->defn;
    Cell infos = nameNil;
    List rcs = NIL;
    for (; hasCfun(cs); cs = tl(cs)) {
        rcs = cons(hd(cs), rcs);
    }
    for (; nonNull(rcs); rcs = tl(rcs)) {
        infos = ap2(nameCons, genConInfo(hd(rcs)), infos);
    }
    return ap(ap(ap(ap(genName("DataInfo"), mkStr(tyconRec(t)->text)),
                    mkStr(moduleRec(tyconRec(t)->mod)->text)),
                 tyconRec(t)->what == NEWTYPE ? nameTrue : nameFalse),
              infos);
}

/* The ConInfo for constructor c: its fixity is infix when derived Show
 * would show it infix */
static Cell genConInfo(Name c) {
    int k = userArity(c);
    Syntax s = syntaxOf(c);
    List cfs = cfunSfuns;
    Cell labels = nameNil;
    Cell fixity = genName("Prefix");
    bool record = false;

    for (; nonNull(cfs) && c != fst(hd(cfs)); cfs = tl(cfs)) {
    }
    if (nonNull(cfs)) {
        List ls = rev(dupList(snd(hd(cfs))));
        record = true;
        for (; nonNull(ls); ls = tl(ls)) {
            labels = ap2(nameCons, mkStr(textOf(hd(ls))), labels);
        }
    } else {
        for (; k > 0; k--) {
            labels = ap2(nameCons, mkStr(findText("")), labels);
        }
    }
    if (userArity(c) == 2 && assocOf(s) != APPLIC) {
        char *a = assocOf(s) == LEFT_ASS    ? "LeftAssociative"
                  : assocOf(s) == RIGHT_ASS ? "RightAssociative"
                                            : "NotAssociative";
        fixity = ap(ap(genName("Infix"), genName(a)), mkInt(precOf(s)));
    }
    return ap(ap(ap(ap(genName("ConInfo"), mkStr(nameRec(c)->text)), fixity),
                 record ? nameTrue : nameFalse),
              labels);
}

/* --------------------------------------------------------------------------
 * Deriving via.  In
 *
 *   newtype Score = Score Int deriving (Semigroup, Monoid) via Sum Int
 *
 * the instances for Score reuse the methods of those for Sum Int.  A newtype
 * constructor is the identity at run time, so a Score and a Sum Int are the
 * same value, and the dictionary of Monoid (Sum Int) serves for Score as it
 * is: the instance has no bindings, and typeInstDefn makes its dictionary
 * from the methods of the via type's (its superclasses are Score's own).
 * Its context is the one the via type's instance needs, found as for any
 * derived instance.  `deriving newtype C' is deriving C via the type the
 * newtype wraps.
 *
 * Two checks keep this safe.  The two types must be the same at run time:
 * with newtypes and synonyms unwrapped at the top, the same type.  And the
 * class must not let evidence for the via type pass for evidence for the
 * derived type, which is what roles are for in GHC; without type families
 * that can only happen through a functional dependency, which could then
 * fix a type wrongly, so a class whose methods mention a class with
 * dependencies, or a type whose constructors hold class evidence, is
 * refused.
 * ------------------------------------------------------------------------*/

/* Fit the head of in, an instance a deriving clause asks for via a type, to
 * the class: for a class whose parameter has the kind k1 -> ... -> kn ->
 * Type, as Monad's has, the type is applied to all but its last n
 * parameters, so newtype L t m a = L (t m a) deriving Monad via (t m)
 * gives Monad (L t m).  With the newtype strategy, the via type is then
 * what that wraps, newtype App m a = App (StateT Int m a) deriving newtype
 * Monad being derived via StateT Int m, and the context to infer is the
 * via type's instance's. */
static void viaHead(Inst in) {
    Class c = instRec(in)->c;
    Kind k = nth(cclassRec(c)->arity - 1, cclassRec(c)->kinds);
    int a = intOf(instRec(in)->kinds);
    int n = 0;
    Type self = arg(instRec(in)->head);
    for (; isAp(k); k = snd(k)) {
        n++;
    }
    if (n > a) {
        errHead(instRec(in)->line);
        errPrintf("Cannot derive \"%s\" for \"%s\": the type has too few "
                  "parameters for the class",
                  textToStr(cclassRec(c)->text),
                  textToStr(tyconRec(getHead(self))->text));
        errFail();
    }
    for (a -= n; n > 0; n--) {
        self = fun(self);
    }
    arg(instRec(in)->head) = self;
    instRec(in)->kinds = mkInt(a);
    if (instRec(in)->via == NEWTYPE) {
        Type via = unwrapOnce(self, true);
        if (isNull(via)) {
            errHead(instRec(in)->line);
            errPrintf("Cannot derive ");
            errPred(instRec(in)->head);
            errPrintf(" with the newtype strategy: what ");
            errType(self);
            errPrintf(" wraps does not end in the parameters the class "
                      "leaves off");
            errFail();
        }
        instRec(in)->via = via;
        hd(snd(snd(instRec(in)->specifics))) = via;
    }
}

/* Check the derived instance in, which is derived via a type */
static void checkVia(Inst in) {
    Cell self = arg(instRec(in)->head);
    Class c = instRec(in)->c;
    Type via = instRec(in)->via;
    Type selfRep = runtimeType(self);
    Type viaRep = runtimeType(via);

    if (isNull(selfRep) || isNull(viaRep)) {
        errHead(instRec(in)->line);
        errPrintf("Cannot derive ");
        errPred(instRec(in)->head);
        errPrintf(" via ");
        errType(via);
        errPrintf(": unwrapping the newtypes in ");
        errType(isNull(selfRep) ? self : via);
        errPrintf(" reached the cutoff limit, %d, without reaching its type "
                  "at run time",
                  cutoff);
        errFail();
    }
    if (!sameRep(selfRep, viaRep)) {
        errHead(instRec(in)->line);
        errPrintf("Cannot derive ");
        errPred(instRec(in)->head);
        errPrintf(" via ");
        errType(via);
        errPrintf(": the two types are not the same at run time");
        errFail();
    }
    if (!viaReusable(c)) {
        errHead(instRec(in)->line);
        errPrintf("Cannot derive \"%s\" via a type: its methods involve a "
                  "functional dependency or a type holding class evidence",
                  textToStr(cclassRec(c)->text));
        errFail();
    }
}

/* The type t as it is at run time, with the newtypes and synonyms at its
 * head unwrapped, or NIL if that takes more than cutoff steps: a recursive
 * newtype such as
 *
 *   newtype Loop = Loop Loop
 *
 * never reaches a type that is not a newtype */
static Type runtimeType(Type t) {
    Type u;
    int n = 0;
    while (nonNull(u = unwrapOnce(t, true))) {
        if (n++ >= cutoff) {
            return NIL;
        }
        t = u;
    }
    return t;
}

/* The type t with the synonyms at its head expanded, and its newtypes
 * unwrapped too if newtypes; which must be false if t may have a recursive
 * newtype at its head, as runtimeType deals with */
static Type unwrapHead(Type t, bool newtypes) {
    Type u;
    while (nonNull(u = unwrapOnce(t, newtypes))) {
        t = u;
    }
    return t;
}

/* The type t with the synonym at its head expanded, or the newtype there
 * unwrapped if newtypes, or NIL if there is neither.  A newtype applied to
 * fewer than all its parameters unwraps only if what it wraps ends in the
 * missing ones, as LiftingAccum t m does in
 *
 *   newtype LiftingAccum t m a = LiftingAccum (t m a)
 *
 * which is then t m. */
static Type unwrapOnce(Type t, bool newtypes) {
    Cell h = getHead(t);
    int n = argCount;
    List args = NIL;
    Type u = t;
    int i = 0;
    if (!isTycon(h) || !(isSynonym(h) || (newtypes && isNewtype(h))) ||
        (isNewtype(h) && !hasCfun(tyconRec(h)->defn))) {
        return NIL;
    }
    for (; i < n; i++, u = fun(u)) {
        args = cons(arg(u), args);
    }
    if (n < tyconRec(h)->arity) {
        return isNewtype(h) ? etaReduce(h, n, args) : NIL;
    }
    if (isSynonym(h)) {
        t = substOffsets(tyconRec(h)->defn, args);
    } else {
        t = substOffsets(hd(conFieldTypes(hd(tyconRec(h)->defn))), args);
    }
    for (i = tyconRec(h)->arity; i < n; i++) {
        t = ap(t, nth(i, args));
    }
    return t;
}

/* Does the type t have the offset n in it? */
static bool hasOffset(Type t, int n) {
    if (isAp(t)) {
        return hasOffset(fun(t), n) || hasOffset(arg(t), n);
    }
    return isOffset(t) && offsetOf(t) == n;
}

/* What the newtype h wraps, applied to the first n of its parameters, args:
 * its field's type with the parameters after the nth dropped from its end,
 * if it ends in them and has them nowhere else; NIL if not */
static Type etaReduce(Tycon h, int n, List args) {
    Type f = hd(conFieldTypes(hd(tyconRec(h)->defn)));
    int i = tyconRec(h)->arity;
    while (i-- > n) {
        if (!isAp(f) || !isOffset(arg(f)) || offsetOf(arg(f)) != i) {
            return NIL;
        }
        f = fun(f);
    }
    for (i = n; i < tyconRec(h)->arity; i++) {
        if (hasOffset(f, i)) {
            return NIL;
        }
    }
    return substOffsets(f, args);
}

/* The type t with each offset n replaced by the nth of args */
static Type substOffsets(Type t, List args) {
    switch (whatIs(t)) {
    case AP:
        return ap(substOffsets(fun(t), args), substOffsets(arg(t), args));
    case OFFSET:
        return nth(offsetOf(t), args);
    default:
        return t;
    }
}

/* Are the types t and u the same, synonyms aside? */
static bool sameRep(Type t, Type u) {
    Cell h = getHead(t);
    if (isSynonym(h) && argCount >= tyconRec(h)->arity) {
        return sameRep(unwrapHead(t, false), u);
    }
    h = getHead(u);
    if (isSynonym(h) && argCount >= tyconRec(h)->arity) {
        return sameRep(t, unwrapHead(u, false));
    }
    if (isAp(t) && isAp(u)) {
        return sameRep(fun(t), fun(u)) && sameRep(arg(t), arg(u));
    }
    return t == u || (isOffset(t) && isOffset(u) && offsetOf(t) == offsetOf(u));
}

/* Can the methods of class c be reused at another type with the same run
 * time representation?  Not if a method's context has a class with
 * functional dependencies, or its type involves a data type whose
 * constructors hold class evidence. */
static bool viaReusable(Class c) {
    List ms = cclassRec(c)->members;
    List seen = NIL;
    for (; nonNull(ms); ms = tl(ms)) {
        Type t = nameRec(hd(ms))->type;
        if (isPolyType(t)) {
            t = monotypeOf(t);
        }
        if (whatIs(t) == QUAL) {       /* past C's own predicate, the reused */
            List ps = tl(fst(snd(t))); /* dictionary                    */
            for (; nonNull(ps); ps = tl(ps)) {
                Cell h = getHead(hd(ps));
                if (isClass(h) && nonNull(cclassRec(h)->fds)) {
                    return false;
                }
            }
            t = snd(snd(t));
        }
        if (holdsEvidence(t, &seen)) {
            return false;
        }
    }
    return true;
}

/* Does the type t, or a type it involves, hold evidence that a functional
 * dependency could misuse?  seen lists the type constructors looked at. */
static bool holdsEvidence(Type t, List *seen) {
    switch (whatIs(t)) {
    case AP:
        return holdsEvidence(fun(t), seen) || holdsEvidence(arg(t), seen);
    case POLYTYPE:
    case RANK2:
    case EXIST:
        return holdsEvidence(snd(snd(t)), seen);
    case QUAL: {
        List ps = fst(snd(t));
        for (; nonNull(ps); ps = tl(ps)) {
            Cell h = getHead(hd(ps));
            if (isClass(h) && nonNull(cclassRec(h)->fds)) {
                return true;
            }
        }
        return holdsEvidence(snd(snd(t)), seen);
    }
    case CDICTS:
        return true;
    case TYCON: {
        List cs;
        if (cellIsMember(t, *seen) || isSynonym(t)) {
            return false;
        }
        *seen = cons(t, *seen);
        for (cs = tyconRec(t)->defn; hasCfun(cs); cs = tl(cs)) {
            if (holdsEvidence(nameRec(hd(cs))->type, seen)) {
                return true;
            }
        }
        return false;
    }
    default:
        return false;
    }
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
};

/* Clear the list of derived instances still to implement. */
void resetDeriving(void) {
    derivedInsts = NIL;
}

/* Register the roots and reset; done once, before the Prelude is read. */
void installDeriving(void) {
    registerRoots(derivingRoots);
    resetDeriving();
}

/*-------------------------------------------------------------------------*/
