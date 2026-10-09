/* --------------------------------------------------------------------------
 * Kind inference for type constructors, classes and instances.
 *
 * Every type constructor and class gets a kind here, a group of mutually
 * recursive ones at a time in the order the dependency analysis hands them
 * over, and every type expression that the static analysis has checked is
 * checked to be well-kinded.  Kind unification itself is in subst.c.
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
#include "tagcheck.h"
#include "kind.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static void kindError(int, Constr, Constr, char *, Typing, Kind, int);
static Typing kindConstr(int, int, int, Constr);
static Kind kindAtom(int, Constr);
static void initTCKind(Cell);
static void kindTC(Cell);
static void genTC(Cell);
static void declareKind(int, Text, int, Kind);
static void useDeclaredKinds(Text, Kind);

/* --------------------------------------------------------------------------
 * Kind checking preliminaries:
 * ------------------------------------------------------------------------*/

bool kindExpert = false; /* true => display kind errors in  */
                         /*        full detail             */

/* Line number near constuctor exp constructor context (if any) place in which
 * error occurs inferred kind got expected kind (k,o) */
static void kindError(int l, Constr c, Constr in, char *wh, Typing got, Kind k,
                      int o) {
    clearMarks();

    if (!kindExpert) { /* for those with a fear of kinds  */
        errHead(l);
        errPrintf("Illegal type");
        if (nonNull(in)) {
            errPrintf(" \"");
            errType(in);
            errPrintf("\"");
        }
        errPrintf(" in %s\n", wh);
        errFail();
    }

    errHead(l);
    errPrintf("Kind error in %s", wh);
    if (nonNull(in)) {
        errPrintf("\n*** expression     : ");
        errType(in);
    }
    errPrintf("\n*** constructor    : ");
    errType(c);
    errPrintf("\n*** kind           : ");
    errKind(copyType(got.type, got.offs));
    errPrintf("\n*** does not match : ");
    errKind(copyType(k, o));
    if (unifyFails) {
        errPrintf("\n*** because        : %s", unifyFails);
    }
    errPrintf("\n");
    errFail();
}

/* Unify the inferred kind got with (k,o) */
static void shouldKind(int l, Cell c, Cell in, char *wh, Typing got, Kind k,
                       int o) {
    if (!kunify(got.type, got.offs, k, o))
        kindError(l, c, in, wh, got, k, o);
}

/* Infer the kind of c, then as above */
static void checkKind(int l, int a, int m, Cell c, Cell in, char *wh, Kind k,
                      int o) {
    shouldKind(l, c, in, wh, kindConstr(l, a, m, c), k, o);
}

List unkindTypes; /* types in need of kind annotation*/

/* --------------------------------------------------------------------------
 * Kind inference for simple types:
 * ------------------------------------------------------------------------*/

/* Determine kind of constructor */
static Typing kindConstr(int line, int alpha, int m, Cell c) {
    assertType(c);
    Cell h = getHead(c);
    int n = argCount;

    switch (whatIs(h)) {
    case POLYTYPE: {
        if (n != 0) {
            internal("kindConstr1");
        }
        static char *pt = "polymorphic type";
        Type t = dropRank1(c, alpha, m);
        Kinds ks = polySigOf(t);
        int m1 = 0;
        int beta;
        Typing got;
        for (; isAp(ks); ks = tl(ks)) {
            m1++;
        }
        beta = newKindvars(m1);
        unkindTypes = cons(pair(mkInt(beta), t), unkindTypes);
        got = kindConstr(line, beta, m1, monotypeOf(t));
        shouldKind(line, monotypeOf(t), NIL, pt, got, STAR, 0);
        return got;
    }

    case CDICTS:
    case QUAL:
        if (n != 0) {
            internal("kindConstr2");
        }
        map3Proc(kindPred, line, alpha, m, fst(snd(c)));
        return kindConstr(line, alpha, m, snd(snd(c)));

    case EXIST:
    case RANK2:
        return kindConstr(line, alpha, m, snd(snd(c)));

    case TYCON:
        if (isSynonym(h) && n < tyconRec(h)->arity) {
            errHead(line);
            errPrintf("Not enough arguments for type synonym \"%s\"",
                      textToStr(tyconRec(h)->text));
            errFail();
        }
        break;
    default:
        break;
    }

    if (n == 0) /* trivial case, no arguments    */
        return typing(kindAtom(alpha, c), 0);

    /* non-trivial application    */
    static char *app = "constructor application";
    Cell a = c;
    int i;
    Typing k = varKind(n);
    int beta = k.offs;

    /* h  :: v1 -> ... -> vn -> w         */
    shouldKind(line, h, c, app, typing(kindAtom(alpha, h), 0), k.type, beta);

    for (i = n; i > 0; --i) { /* ci :: vi for each 1 <- 1..n           */
        checkKind(line, alpha, m, arg(a), c, app, aVar, beta + i - 1);
        a = fun(a);
    }
    return tyvarType(beta + n); /* inferred kind is w                 */
}

/* Find kind of atomic constructor */
static Kind kindAtom(int alpha, Cell c) {
    switch (whatIs(c)) {
    case TUPLE:
        return simpleKind(tupleOf(c)); /*(,)::* -> * -> * */
    case OFFSET:
        return mkInt(alpha + offsetOf(c));
    case TYCON:
        return tyconRec(c)->kind;
    case INTCELL:
        return c;
    case VARIDCELL:
    case VAROPCELL: {
        Cell vt = findBtyvs(textOf(c));
        if (nonNull(vt)) {
            return snd(vt);
        }
    }
    default:
        break;
    }
    internal("kindAtom");
    return STAR; /* not reached */
}

/* Check kinds of arguments in pred: a class's arguments have the kinds the
 * class declares, the two sides of an equality have one kind between them */
void kindPred(int l, int alpha, int m, Cell pi) {
    assertPred(pi);
    if (isEquality(pi)) {
        static char *equality = "equality constraint";
        Typing k = kindConstr(l, alpha, m, arg(fun(pi)));
        checkKind(l, alpha, m, arg(pi), NIL, equality, k.type, k.offs);
    } else {
        static char *predicate = "class constraint";
        Class c = getHead(pi);
        List as = getArgs(pi);
        Kinds ks = cclassRec(c)->kinds;

        while (nonNull(ks)) {
            checkKind(l, alpha, m, hd(as), NIL, predicate, hd(ks), 0);
            ks = tl(ks);
            as = tl(as);
        }
    }
}

/* Check that (poss qualified) type is well-kinded */
void kindType(int line, char *wh, Type type) {
    assertType(type);
    checkKind(line, 0, 0, type, NIL, wh, STAR, 0);
}

/* Add kind annotations to types */
void fixKinds(void) {
    for (; nonNull(unkindTypes); unkindTypes = tl(unkindTypes)) {
        Pair pr = hd(unkindTypes);
        int beta = intOf(fst(pr));
        Cell qts = polySigOf(snd(pr));
        for (;;) {
            if (isNull(hd(qts))) {
                hd(qts) = copyKindvar(beta++);
            } else {
                internal("fixKinds");
            }
            if (nonNull(tl(qts))) {
                qts = tl(qts);
            } else {
                tl(qts) = STAR;
                break;
            }
        }
    }
}

/* --------------------------------------------------------------------------
 * Kind checking of groups of type constructors and classes:
 * ------------------------------------------------------------------------*/

/* Find kinds for mutually rec. gp of tycons and classes */
void kindTCGroup(List tcs) {
    emptySubstitution();
    unkindTypes = NIL;
    mapProc(initTCKind, tcs);
    mapProc(kindTC, tcs);
    mapProc(genTC, tcs);
    fixKinds();
    emptySubstitution();
}

/* Build initial kind/arity for c */
static void initTCKind(Cell c) {
    /* The initial kind of a tycon of arity n is v1 -> ... -> vn -> vn+1,
     * and for data definitions vn+1 == * */
    if (isTycon(c)) {
        int beta = newKindvars(1);
        Typing k = varKind(tyconRec(c)->arity);
        bindTv(beta, k.type, k.offs);
        switch (whatIs(tyconRec(c)->what)) {
        case NEWTYPE:
        case DATATYPE:
            bindTv(k.offs + tyconRec(c)->arity, STAR, 0);
        default:
            break;
        }
        tyconRec(c)->kind = mkInt(beta);
        useDeclaredKinds(tyconRec(c)->text, mkInt(beta));
    } else {
        int n = cclassRec(c)->arity;
        int beta = newKindvars(n);
        Kind k = CONSTRAINT;       /* v1 -> .. -> vn -> Constraint, to meet */
        cclassRec(c)->kinds = NIL; /* the declared kinds           */
        while (n > 0) {
            n--;
            cclassRec(c)->kinds = pair(mkInt(beta + n), cclassRec(c)->kinds);
            k = ap(mkInt(beta + n), k);
        }
        useDeclaredKinds(cclassRec(c)->text, k);
    }
}

/* Check each part of a tycon/class is well-kinded */
static void kindTC(Cell c) {
    if (isTycon(c)) {
        static char *cfun = "data constructor";
        static char *tsyn = "synonym declaration";
        int line = tyconRec(c)->line;
        int beta = tyvar(intOf(tyconRec(c)->kind))->offs;
        int m = tyconRec(c)->arity;
        switch (whatIs(tyconRec(c)->what)) {
        case NEWTYPE:
        case DATATYPE: {
            List cs = tyconRec(c)->defn;
            if (isQualType(cs)) {
                map3Proc(kindPred, line, beta, m, fst(snd(cs)));
                tyconRec(c)->defn = cs = snd(snd(cs));
            }
            for (; hasCfun(cs); cs = tl(cs)) {
                kindType(line, cfun, nameRec(hd(cs))->type);
            }
            break;
        }

        default:
            checkKind(line, beta, m, tyconRec(c)->defn, NIL, tsyn, aVar,
                      beta + m);
        }
    } else { /* scan type exprs in class defn to*/
        List ms = fst(cclassRec(c)->members);
        int m = cclassRec(c)->arity; /* determine the class signature   */
        int beta = newKindvars(length(cclassRec(c)->tyvars));
        kindPred(cclassRec(c)->line, beta, m, cclassRec(c)->head);
        map3Proc(kindPred, cclassRec(c)->line, beta, m, cclassRec(c)->supers);
        for (; nonNull(ms); ms = tl(ms)) {
            int line = intOf(fst3(hd(ms)));
            Type type = thd3(hd(ms));
            kindType(line, "member function type signature", type);
        }
    }
}

/* Generalise kind inferred for given tycon/class */
static void genTC(Cell c) {
    if (isTycon(c)) {
        tyconRec(c)->kind = copyKindvar(intOf(tyconRec(c)->kind));
    } else {
        Kinds ks = cclassRec(c)->kinds;
        for (; nonNull(ks); ks = tl(ks)) {
            hd(ks) = copyKindvar(intOf(hd(ks)));
        }
    }
}

/* --------------------------------------------------------------------------
 * Kind checking of instance declaration headers:
 * ------------------------------------------------------------------------*/

/* Check predicates in instance */
void kindInst(Inst in, int freedom) {
    int beta;

    emptySubstitution();
    beta = newKindvars(freedom);
    kindPred(instRec(in)->line, beta, freedom, instRec(in)->head);
    if (whatIs(instRec(in)->specifics) != DERIVE) {
        map3Proc(kindPred, instRec(in)->line, beta, freedom,
                 instRec(in)->specifics);
    }
    for (instRec(in)->kinds = NIL; 0 < freedom--;) {
        instRec(in)->kinds =
            cons(copyKindvar(beta + freedom), instRec(in)->kinds);
    }
    emptySubstitution();
}

/* --------------------------------------------------------------------------
 * Declared kinds:
 *
 * A declaration may say what kind its type constructor or class has, as a
 * standalone signature, `type T :: * -> *`, or on the variables it binds,
 * `data T (f :: * -> *) a = ...`.  Both are kept in kindSigDefns, with the
 * script's other declarations, against the name they are for, as a kind
 * skeleton in which offsets stand for the parts left unsaid, and each is
 * unified with the kind the inference starts from when the group that
 * declares the name is kinded.  Kind inference then goes on as it would
 * have; a use that disagrees with the signature is a kind error at the use.
 * A declared kind that no declaration in the script claims is an error too.
 *
 * kindSigDefns :: [(Name, (line, (unknowns, skeleton)))]
 * ------------------------------------------------------------------------*/

/* Keep the kind skeleton k, with its offsets numbered below unknowns, as a
 * kind declared for t */
static void declareKind(int line, Text t, int unknowns, Kind k) {
    Cell d;
    push(k); /* the parts a GC must not lose while the rest is made */
    push(mkInt(line));
    push(ap(CONIDCELL, t));
    d = pair(mkInt(unknowns), pushed(2));
    d = pair(pushed(1), d);
    d = pair(top(), d);
    kindSigDefns = cons(d, kindSigDefns);
    drop();
    drop();
    drop();
}

/* A standalone kind signature, type T :: k, as the parser read it */
void kindSigDefn(int line, Text t, Kind k) {
    declareKind(line, t, 0, k);
}

/* Take the kind annotations off the binders of a declaration head,
 * T a (b :: k) ..., leaving plain variables, and declare the kind they give
 * for t.  The unannotated binders and the result are left unsaid. */
void stripBinderKinds(int line, Text t, Cell lhs) {
    int unknowns = 1; /* offset 0 is the result */
    Kind k = mkOffset(0);
    bool any = false;
    Cell h;

    for (h = lhs; isAp(h); h = fun(h)) { /* last binder first */
        if (whatIs(arg(h)) == KINDED) {
            k = ap(snd(snd(arg(h))), k);
            any = true;
        } else {
            k = ap(mkOffset(unknowns++), k);
        }
    }
    if (any) {
        declareKind(line, t, unknowns, k);
        for (h = lhs; isAp(h); h = fun(h)) {
            if (whatIs(arg(h)) == KINDED) {
                arg(h) = fst(snd(arg(h)));
            }
        }
    }
}

/* Unify every kind declared for the name t with k, the kind its declaration
 * starts from, and forget them */
static void useDeclaredKinds(Text t, Kind k) {
    List *ds = &kindSigDefns;

    while (nonNull(*ds)) {
        Cell d = hd(*ds);
        if (textOf(fst(d)) != t) {
            ds = &tl(*ds);
        } else {
            int line = intOf(fst(snd(d)));
            int unknowns = intOf(fst(snd(snd(d))));
            Kind s = snd(snd(snd(d)));
            int base = newKindvars(unknowns);
            if (!kunify(s, base, k, 0)) {
                errHead(line);
                errPrintf("Kind signature for \"%s\" does not fit its "
                          "declaration",
                          textToStr(t));
                errPrintf("\n*** signature     : ");
                errKind(s); /* the parts left unsaid print as variables */
                errPrintf("\n*** declaration   : ");
                errKind(copyKind(k, 0));
                errPrintf("\n");
                errFail();
            }
            *ds = tl(*ds);
        }
    }
}

/* Every declared kind must have met its declaration by the time the
 * script's type constructors and classes are kinded */
void noUnclaimedKinds(void) {
    if (nonNull(kindSigDefns)) {
        Cell d = hd(kindSigDefns);
        errHead(intOf(fst(snd(d))));
        errPrintf("Kind signature for \"%s\", which this module does not "
                  "declare",
                  textToStr(textOf(fst(d))));
        errFail();
    }
}

/* --------------------------------------------------------------------------
 * Control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const kindRoots[] = {
    &unkindTypes,
};

/* Clear the list of types still waiting for a kind annotation. */
void resetKind(void) {
    unkindTypes = NIL;
}

/* Register the roots and reset; done once, before the Prelude is read. */
void installKind(void) {
    registerRoots(kindRoots);
    resetKind();
}

/*-------------------------------------------------------------------------*/
