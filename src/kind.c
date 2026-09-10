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
#include "output.h"
#include "subst.h"
#include "module.h"
#include "opts.h"
#include "tagcheck.h"
#include "kind.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static Void local kindError(Int, Constr, Constr, String, Kind, Int);
static Void local kindConstr(Int, Int, Int, Constr);
static Kind local kindAtom(Int, Constr);
static Void local initTCKind(Cell);
static Void local kindTC(Cell);
static Void local genTC(Cell);

/* --------------------------------------------------------------------------
 * Kind checking preliminaries:
 * ------------------------------------------------------------------------*/

Bool kindExpert = FALSE; /* TRUE => display kind errors in  */
                         /*        full detail             */

static Void local kindError(Int l, Constr c, Constr in, String wh, Kind k,
                            Int o)
/* line number near constuctor exp */
/* constructor                     */
/* context (if any)                */
/* place in which error occurs     */
/* expected kind (k,o)             */
{ /* inferred kind (typeIs,typeOff)  */
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

static Void local shouldKind(Int l, Cell c, Cell in, String wh, Kind k,
                             Int o) { /* unify the inferred kind with (k,o) */
    if (!kunify(typeIs, typeOff, k, o))
        kindError(l, c, in, wh, k, o);
}

static Void local checkKind(Int l, Int a, Int m, Cell c, Cell in, String wh,
                            Kind k,
                            Int o) { /* infer the kind of c, then as above */
    kindConstr(l, a, m, c);
    shouldKind(l, c, in, wh, k, o);
}

List unkindTypes; /* types in need of kind annotation*/

/* --------------------------------------------------------------------------
 * Kind inference for simple types:
 * ------------------------------------------------------------------------*/

static Void local kindConstr(Int line, Int alpha, Int m, Cell c)
/* Determine kind of constructor   */
{
    assertType(c);
    Cell h = getHead(c);
    Int n = argCount;

    switch (whatIs(h)) {
    case POLYTYPE:
        if (n != 0) {
            internal("kindConstr1");
        } else {
            static String pt = "polymorphic type";
            Type t = dropRank1(c, alpha, m);
            Kinds ks = polySigOf(t);
            Int m1 = 0;
            Int beta;
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
        static String app = "constructor application";
        Cell a = c;
        Int i;
        Kind k;
        Int beta;

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

static Kind local kindAtom(Int alpha,
                           Cell c) /* Find kind of atomic constructor */
{
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

Void kindPred(Int l, Int alpha, Int m,
              Cell pi) /* Check kinds of arguments in pred*/
{
    assertPred(pi);
    {
        static String predicate = "class constraint";
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

Void kindType(Int line, String wh,
              Type type) /* check that (poss qualified) type*/
                         /* is well-kinded            */
{
    assertType(type);
    checkKind(line, 0, 0, type, NIL, wh, STAR, 0);
}

Void fixKinds() { /* add kind annotations to types   */
    for (; nonNull(unkindTypes); unkindTypes = tl(unkindTypes)) {
        Pair pr = hd(unkindTypes);
        Int beta = intOf(fst(pr));
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

Void kindTCGroup(List tcs) /* find kinds for mutually rec. gp */
{                          /* of tycons and classes           */
    emptySubstitution();
    unkindTypes = NIL;
    mapProc(initTCKind, tcs);
    mapProc(kindTC, tcs);
    mapProc(genTC, tcs);
    fixKinds();
    emptySubstitution();
}

static Void local initTCKind(Cell c) /* build initial kind/arity for    c  */
{
    if (isTycon(c)) {                  /* Initial kind of tycon is:        */
        Int beta = newKindvars(1);     /*    v1 -> ... -> vn -> vn+1       */
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
        Int n = cclassRec(c)->arity;
        Int beta = newKindvars(n);
        cclassRec(c)->kinds = NIL;
        while (n > 0) {
            n--;
            cclassRec(c)->kinds = pair(mkInt(beta + n), cclassRec(c)->kinds);
        }
    }
}

static Void local kindTC(Cell c) /* check each part of a tycon/class*/
{                                /* is well-kinded                 */
    if (isTycon(c)) {
        static String cfun = "data constructor";
        static String tsyn = "synonym declaration";
        Int line = tyconRec(c)->line;
        Int beta = tyvar(intOf(tyconRec(c)->kind))->offs;
        Int m = tyconRec(c)->arity;
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
        Int m = cclassRec(c)->arity; /* determine the class signature   */
        Int beta = newKindvars(length(cclassRec(c)->tyvars));
        kindPred(cclassRec(c)->line, beta, m, cclassRec(c)->head);
        map3Proc(kindPred, cclassRec(c)->line, beta, m, cclassRec(c)->supers);
        for (; nonNull(ms); ms = tl(ms)) {
            Int line = intOf(fst3(hd(ms)));
            Type type = thd3(hd(ms));
            kindType(line, "member function type signature", type);
        }
    }
}

static Void local genTC(Cell c) /* generalise kind inferred for    */
{                               /* given tycon/class               */
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

Void kindInst(Inst in, Int freedom) /* check predicates in instance    */
{
    Int beta;

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

Void kindControl(Int what) {
    switch (what) {
    case RESET:
        unkindTypes = NIL;
        break;

    case INSTALL:
        registerRoots(kindRoots);
        kindControl(RESET);
        break;
    default:
        break;
    }
}

/*-------------------------------------------------------------------------*/
