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

static void kindError(int, Constr, Constr, char *, Kind, int);
static void kindConstr(int, int, int, Constr);
static Kind kindAtom(int, Constr);
static void initTCKind(Cell);
static void kindTC(Cell);
static void genTC(Cell);

/* --------------------------------------------------------------------------
 * Kind checking preliminaries:
 * ------------------------------------------------------------------------*/

bool kindExpert = false; /* true => display kind errors in  */
                         /*        full detail             */

/* Line number near constuctor exp constructor context (if any) place in which
 * error occurs expected kind (k,o) inferred kind (typeIs,typeOff) */
static void kindError(int l, Constr c, Constr in, char *wh, Kind k, int o) {
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
    errKind(copyType(typeIs, typeOff));
    errPrintf("\n*** does not match : ");
    errKind(copyType(k, o));
    if (unifyFails) {
        errPrintf("\n*** because        : %s", unifyFails);
    }
    errPrintf("\n");
    errFail();
}

/* Unify the inferred kind with (k,o) */
static void shouldKind(int l, Cell c, Cell in, char *wh, Kind k, int o) {
    if (!kunify(typeIs, typeOff, k, o))
        kindError(l, c, in, wh, k, o);
}

/* Infer the kind of c, then as above */
static void checkKind(int l, int a, int m, Cell c, Cell in, char *wh, Kind k,
                      int o) {
    kindConstr(l, a, m, c);
    shouldKind(l, c, in, wh, k, o);
}

List unkindTypes; /* types in need of kind annotation*/

/* --------------------------------------------------------------------------
 * Kind inference for simple types:
 * ------------------------------------------------------------------------*/

/* Determine kind of constructor */
static void kindConstr(int line, int alpha, int m, Cell c) {
    assertType(c);
    Cell h = getHead(c);
    int n = argCount;

    switch (whatIs(h)) {
    case POLYTYPE:
        if (n != 0) {
            internal("kindConstr1");
        } else {
            static char *pt = "polymorphic type";
            Type t = dropRank1(c, alpha, m);
            Kinds ks = polySigOf(t);
            int m1 = 0;
            int beta;
            for (; isAp(ks); ks = tl(ks)) {
                m1++;
            }
            beta = newKindvars(m1);
            unkindTypes = cons(pair(mkInt(beta), t), unkindTypes);
            checkKind(line, beta, m1, monotypeOf(t), NIL, pt, STAR, 0);
        }
        return;

    case CDICTS:
    case QUAL:
        if (n != 0) {
            internal("kindConstr2");
        }
        map3Proc(kindPred, line, alpha, m, fst(snd(c)));
        kindConstr(line, alpha, m, snd(snd(c)));
        return;

    case EXIST:
    case RANK2:
        kindConstr(line, alpha, m, snd(snd(c)));
        return;

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

    if (n == 0) { /* trivial case, no arguments    */
        typeIs = kindAtom(alpha, c);
    } else { /* non-trivial application    */
        static char *app = "constructor application";
        Cell a = c;
        int i;
        Kind k;
        int beta;

        varKind(n);
        beta = typeOff;
        k = typeIs;

        typeIs = kindAtom(alpha, h); /* h  :: v1 -> ... -> vn -> w         */
        shouldKind(line, h, c, app, k, beta);

        for (i = n; i > 0; --i) { /* ci :: vi for each 1 <- 1..n           */
            checkKind(line, alpha, m, arg(a), c, app, aVar, beta + i - 1);
            a = fun(a);
        }
        tyvarType(beta + n); /* inferred kind is w                 */
    }
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

/* Check kinds of arguments in pred */
void kindPred(int l, int alpha, int m, Cell pi) {
    assertPred(pi);
    {
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
    if (isTycon(c)) {                  /* Initial kind of tycon is:        */
        int beta = newKindvars(1);     /*    v1 -> ... -> vn -> vn+1       */
        varKind(tyconRec(c)->arity);   /* where n is the arity of c.       */
        bindTv(beta, typeIs, typeOff); /* For data definitions, vn+1 == * */
        switch (whatIs(tyconRec(c)->what)) {
        case NEWTYPE:
        case DATATYPE:
            bindTv(typeOff + tyconRec(c)->arity, STAR, 0);
        default:
            break;
        }
        tyconRec(c)->kind = mkInt(beta);
    } else {
        int n = cclassRec(c)->arity;
        int beta = newKindvars(n);
        cclassRec(c)->kinds = NIL;
        while (n > 0) {
            n--;
            cclassRec(c)->kinds = pair(mkInt(beta + n), cclassRec(c)->kinds);
        }
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

void kindControl(int what) {
    switch (what) {
    case RESET:
        resetKind();
        break;
    case INSTALL:
        installKind();
        break;
    default:
        break;
    }
}

/*-------------------------------------------------------------------------*/
