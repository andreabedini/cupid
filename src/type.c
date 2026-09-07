/* --------------------------------------------------------------------------
 * This is the Cuddles type checker
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: type.c,v $
 * $Revision: 1.81 $
 * $Date: 2005/12/10 11:25:13 $
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "scc.h"
#include "preds.h"
#include "tagcheck.h"
#include "errors.h"
#include "output.h"
#include "subst.h"
#include "goal.h"
#include "opts.h" /* needed for EXPLAIN_INSTANCE_RESOLUTION */

Bool catchAmbigs = FALSE; /* TRUE => functions with ambig.   */
                          /*       types produce error     */

Type typeArrow, typeList; /* Important primitive types     */
Type typeUnit;

Module modulePrelude;
Module moduleUserPrelude;

Type typeInt;
Type typeInt8;
Type typeInt16;
Type typeInt32;
Type typeInt64;
Type typeWord;
Type typeWord8;
Type typeWord16;
Type typeWord32;
Type typeWord64;
Type typeFunPtr;
Type typePtr;
Type typeAddr;
Type typeFloat;
Type typeDouble;
Type typeChar;
Type typeForeignP;
Type typeForeign;
Type typeStable;
Type typeBool;
Type typeString;

static Type typeInteger;
static Type typeMaybe;
static Type typeOrdering;

Class classEq, classOrd; /* `standard' classes             */
Class classIx, classEnum;
Class classShow, classRead;
Class classBounded;

Class classReal, classIntegral; /* `numeric' classes               */
Class classRealFrac, classRealFloat;
Class classFractional, classFloating;
Class classNum;

List stdDefaults; /* standard default values       */

Name nameFromInt, nameFromDouble; /* coercion of numerics                  */
Name nameFromInteger;
Name nameEq, nameCompare; /* derivable names               */
Name nameLe;
Name nameShowsPrec;
Name nameReadsPrec;
Name nameMinBnd, nameMaxBnd;
Name nameIndex, nameInRange;
Name nameRange;
Name nameMult, namePlus;
Name nameTrue, nameFalse;   /* primitive boolean constructors  */
Name nameNil, nameCons;     /* primitive list constructors         */
Name nameJust, nameNothing; /* primitive Maybe constructors        */
Name nameLeft, nameRight;   /* primitive Either constructors   */
Name nameUnit;              /* primitive Unit type constructor */
Name nameLT, nameEQ;        /* Ordering constructors       */
Name nameGT;
Class classMonad;                    /* Monads                     */
Name nameReturn, nameBind, nameThen; /* for translating do/monad comps  */
Name nameMFail;
Name nameGt; /* for readsPrec              */

Type typeIO;     /* For the IO monad, IO           */
Type typeProgIO; /* For the IO monad, IO a         */

Name nameIOError, nameUserErr; /* loosely coupled IOError cfuns   */
Name namePermDenied;
Name nameAlreadyExists, nameAlreadyInUse, nameDoesNotExist, nameIsFull;
Name nameIllegal;
Name nameEOFErr;
Name nameProtocolError;
Name nameArithException;
Name nameArrayException;
Name nameErrorCall;
Name nameIOException;
Name nameNoMethodError;
Name nameNonTermination;
Name namePatternMatchFail;
Name nameRecConError;
Name nameRecSelError;
Name nameRecUpdError;

Name nameOverflow;
Name nameDivideByZero;
Name nameIndexOutOfBounds;
Name nameUndefinedElement;

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static Void local emptyAssumption(Void);
static Void local enterBindings(Void);
static Void local leaveBindings(Void);
static Int local defType(Cell);
static Type local useType(Cell);
static Void local markAssumList(List);
static Cell local findAssum(Text);
static Pair local findInAssumList(Text, List);
static List local intsIntersect(List, List);
static List local genvarAllAss(List);
static List local genvarAnyAss(List);
static Int local newVarsBind(Cell);
static Void local newDefnBind(Cell, Type);

static Void local enterPendingBtyvs(Void);
static Void local leavePendingBtyvs(Void);
static Cell local patBtyvs(Cell);
static Void local doneBtyvs(Int);
static Void local enterScope(Void);
static Void local leaveScope(Int, Type, Int, Int);

static Void local typeError(Int, Cell, Cell, String, Type, Int);
static Void local reportTypeError(Int, Cell, Cell, String, Type, Type);
static Void local cantEstablish(Int, String, Cell, Type, List);
static Void local tooGeneral(Int, Cell, Type, Type);

static Cell local typeExpr(Int, Cell);

static Cell local typeAp(Int, Cell);
static Type local typeExpected(Int, String, Cell, Type, Int, Int, Bool);
static Void local typeAlt(String, Cell, Cell, Type, Int, Int);
static Int local funcType(Int);
static Void local typeCase(Int, Int, Cell);
static Void local typeComp(Int, Type, Cell, List);
static Cell local typeMonadComp(Int, Cell);
static Void local typeDo(Int, Cell);
static Void local typeConFlds(Int, Cell);
static Void local typeUpdFlds(Int, Cell);
static Cell local typeFreshPat(Int, Cell);

static Void local typeBindings(List);
static Void local removeTypeSigs(Cell);

static Void local monorestrict(List);
static Void local restrictedBindAss(Cell);
static Void local restrictedAss(Int, Cell, Type);

static Void local unrestricted(List);
static List local itbscc(List);
static Void local addEvidParams(List, Cell);

static Void local typeClassDefn(Class);
static Void local typeInstDefn(Inst);
static Void local typeMember(String, Name, Cell, List, Cell, Int);

static Void local typeBind(Cell);
static Void local typeDefAlt(Int, Cell, Pair);
static Cell local typeRhs(Cell);
static Void local guardedType(Int, Cell);

static Void local genBind(List, Cell);
static Void local genAss(Int, List, Cell, Type);
static Type local genTest(Int, Cell, List, Type, Type, Int);
static Type local generalize(List, Type);
static Bool local equalTypes(Type, Type);

static Void local typeDefnGroup(List);
static Void local typeForeignExport(Name);
static Pair local typeSel(Name);

static Name local linkName(String);
static Tycon local linkTycon(String);
static Class local linkClass(String);

/* --------------------------------------------------------------------------
 * Frequently used type skeletons:
 * ------------------------------------------------------------------------*/

static Type arrow;        /* mkOffset(0) -> mkOffset(1)      */
static Type boundPair;    /* (mkOffset(0),mkOffset(0))     */
static Type listof;       /* [ mkOffset(0) ]               */
static Type typeVarToVar; /* mkOffset(0) -> mkOffset(0)            */

static Cell predNum;        /* Num (mkOffset(0))                   */
static Cell predFractional; /* Fractional (mkOffset(0))    */
static Cell predIntegral;   /* Integral (mkOffset(0))      */
static Kind starToStar;     /* Type -> Type                        */
static Cell predMonad;      /* Monad (mkOffset(0))                 */

/* --------------------------------------------------------------------------
 * Assumptions:
 *
 * A basic typing statement is a pair (Var,Type) and an assumption contains
 * an ordered list of basic typing statements in which the type for a given
 * variable is given by the most recently added assumption about that var.
 *
 * In practice, the assumption set is split between a pair of lists, one
 * holding assumptions for vars defined in bindings, the other for vars
 * defined in patterns/binding parameters etc.  The reason for this
 * separation is that vars defined in bindings may be overloaded (with the
 * overloading being unknown until the whole binding is typed), whereas the
 * vars defined in patterns have no overloading.  A form of dependency
 * analysis (at least as far as calculating dependents within the same group
 * of value bindings) is required to implement this.  Where it is known that
 * no overloaded values are defined in a binding (i.e., when the `dreaded
 * monomorphism restriction' strikes), the list used to record dependents
 * is flagged with a NODEPENDS tag to avoid gathering dependents at that
 * level.
 *
 * To interleave between vars for bindings and vars for patterns, we use
 * a list of lists of typing statements for each.  These lists are always
 * the same length.  The implementation here is very similar to that of the
 * dependency analysis used in the static analysis component of this system.
 *
 * To deal with polymorphic recursion, variables defined in bindings can be
 * assigned types of the form (POLYREC,(def,use)), where def is a type
 * variable for the type of the defining occurence, and use is a type
 * scheme for (recursive) calls/uses of the variable.
 * ------------------------------------------------------------------------*/

static List defnBounds; /*::[[(Var,Type)]] possibly ovrlded*/
static List varsBounds; /*::[[(Var,Type)]] not overloaded  */
static List depends;    /*::[?[Var]] dependents/NODEPENDS  */
static Cell dummyVar;   /* Used to put extra tvars into ass*/

typedef struct {
    List skolems;    /* [(tyvar,pat)] skolems bound here           */
    List givens;     /* [(Pred,offset,ev)] local evidence          */
    List savedPreds; /* the enclosing scope's predicates           */
} TcScope;

static TcScope *scopes = 0; /* the open scopes, innermost last */
static Int numScopes = 0;
static Int maxScopes = 0;

#define scope() (scopes + numScopes - 1) /* the innermost open scope       */

#define addVarAssump(v, t) hd(varsBounds) = cons(pair(v, t), hd(varsBounds))
#define findTopBinding(v) findInAssumList(textOf(v), hd(defnBounds))

static Void local emptyAssumption() { /* set empty type assumption         */
    defnBounds = NIL;
    varsBounds = NIL;
    depends = NIL;
    numScopes = 0;
}

static Void local enterBindings() { /* Add new level to assumption sets */
    defnBounds = cons(NIL, defnBounds);
    varsBounds = cons(NIL, varsBounds);
    depends = cons(NIL, depends);
}

static Void local leaveBindings() { /* Drop one level of assumptions    */
    defnBounds = tl(defnBounds);
    varsBounds = tl(varsBounds);
    depends = tl(depends);
}

static Int local defType(Cell a) /* Return type for defining occ.   */
{                                /* of a var from assumption pair  */
    return (isPair(a) && fst(a) == POLYREC) ? fst(snd(a)) : a;
}

static Type local useType(Cell a) /* Return type for use of a var          */
{                                 /* defined in an assumption      */
    return (isPair(a) && fst(a) == POLYREC) ? snd(snd(a)) : a;
}

static Void local markAssumList(List as) /* Mark all types in assumption set*/
{                                        /* :: [(Var, Type)]               */
    for (; nonNull(as); as = tl(as)) {   /* No need to mark generic types;  */
        Type t = defType(snd(hd(as)));   /* the only free variables in those*/
        if (!isPolyType(t))              /* must have been free earlier too */
            markType(t, 0);
    }
}

static Cell local findAssum(Text t) /* Find most recent assumption about*/
{                                   /* variable named t, if any    */
    List defnBounds1 = defnBounds;  /* return translated variable, with */
    List varsBounds1 = varsBounds;  /* type in typeIs              */
    List depends1 = depends;

    while (nonNull(defnBounds1)) {
        Pair ass = findInAssumList(t, hd(varsBounds1)); /* search varsBounds */
        if (nonNull(ass)) {
            typeIs = snd(ass);
            return fst(ass);
        }

        ass = findInAssumList(t, hd(defnBounds1)); /* search defnBounds */
        if (nonNull(ass)) {
            Cell v = fst(ass);
            typeIs = snd(ass);

            if (hd(depends1) != NODEPENDS && /* save dependent?   */
                isNull(v = varIsMember(t, hd(depends1))))
                /* N.B. make new copy of variable and store this on list of*/
                /* dependents, and in the assumption so that all uses of   */
                /* the variable will be at the same node, if we need to    */
                /* overwrite the call of a function with a translation...  */
                hd(depends1) = cons(v = mkVar(t), hd(depends1));

            return v;
        }

        defnBounds1 = tl(defnBounds1); /* look in next level*/
        varsBounds1 = tl(varsBounds1); /* of assumption set */
        depends1 = tl(depends1);
    }
    return NIL;
}

static Pair local
findInAssumList(Text t, List as) /* Search for assumption for var    */
                                 /* named t in list of assumptions as*/
{
    for (; nonNull(as); as = tl(as))
        if (textOf(fst(hd(as))) == t)
            return hd(as);
    return NIL;
}

static List local intsIntersect(List as,
                                List bs) /* calculate intersection of lists */
{                                        /* of integers (as sets)          */
    List ts = NIL;                       /* destructively modifies as      */
    while (nonNull(as))
        if (intIsMember(intOf(hd(as)), bs)) {
            List temp = tl(as);
            tl(as) = ts;
            ts = as;
            as = temp;
        } else
            as = tl(as);
    return ts;
}

static List local genvarAllAss(List as) /* calculate generic vars that are */
{                                       /* in every type in assumptions as */
    List vs = genvarTyvar(intOf(defType(snd(hd(as)))), NIL);
    for (as = tl(as); nonNull(as) && nonNull(vs); as = tl(as))
        vs = intsIntersect(vs, genvarTyvar(intOf(defType(snd(hd(as)))), NIL));
    return vs;
}

static List local genvarAnyAss(List as) /* calculate generic vars that are */
{                                       /* in any type in assumptions as   */
    List vs = genvarTyvar(intOf(defType(snd(hd(as)))), NIL);
    for (as = tl(as); nonNull(as); as = tl(as))
        vs = genvarTyvar(intOf(defType(snd(hd(as)))), vs);
    return vs;
}

static Int local newVarsBind(Cell v) /* make new assump for pattern var  */
{
    Int beta = newTyvars(1);
    addVarAssump(v, mkInt(beta));
    return beta;
}

static Void local newDefnBind(Cell v,
                              Type type) /* make new assump for defn var     */
                                         /* and set type if given (nonNull)  */
{
    Int beta = newTyvars(1);
    Cell ta = mkInt(beta);
    instantiate(type);
    if (nonNull(type) && isPolyType(type))
        ta = pair(POLYREC, pair(ta, type));
    hd(defnBounds) = cons(pair(v, ta), hd(defnBounds));
    bindTv(beta, typeIs, typeOff); /* Bind beta to new type skeleton   */
}

/* --------------------------------------------------------------------------
 * Bound and skolemized type variables:
 * ------------------------------------------------------------------------*/

static List pendingBtyvs = NIL;

static Void local enterPendingBtyvs() {
    enterBtyvs();
    pendingBtyvs = cons(NIL, pendingBtyvs);
}

static Void local leavePendingBtyvs() {
    List pts = hd(pendingBtyvs);
    pendingBtyvs = tl(pendingBtyvs);
    for (; nonNull(pts); pts = tl(pts)) {
        Int line = intOf(fst(hd(pts)));
        List vs = snd(hd(pts));
        Int i = 0;
        clearMarks();
        for (; nonNull(vs); vs = tl(vs)) {
            Cell v = fst(hd(vs));
            Cell t = copyTyvar(intOf(snd(hd(vs))));
            if (!isOffset(t)) {
                errHead(line);
                errPrintf("Type annotation uses variable ");
                errExpr(v);
                errPrintf(" where a more specific type ");
                errType(t);
                errPrintf(" was inferred");
                errFail();
            } else if (offsetOf(t) != i) {
                List us = snd(hd(pts));
                Int j = offsetOf(t);
                if (j >= i)
                    internal("leavePendingBtyvs");
                for (; j > 0; j--)
                    us = tl(us);
                errHead(line);
                errPrintf("Type annotation uses distinct variables ");
                errExpr(v);
                errPrintf(" and ");
                errExpr(fst(hd(us)));
                errPrintf(" where a single variable was inferred");
                errFail();
            } else
                i++;
        }
    }
    leaveBtyvs();
}

static Cell local patBtyvs(Cell p) /* Strip bound type vars from pat  */
{
    if (whatIs(p) == BIGLAM) {
        List bts = hd(btyvars) = fst(snd(p));
        for (p = snd(snd(p)); nonNull(bts); bts = tl(bts)) {
            Int beta = newTyvars(1);
            tyvar(beta)->kind = snd(hd(bts));
            snd(hd(bts)) = mkInt(beta);
        }
    }
    return p;
}

static Void local doneBtyvs(Int l) {
    if (nonNull(hd(btyvars))) { /* Save bound tyvars               */
        hd(pendingBtyvs) = cons(pair(mkInt(l), hd(btyvars)), hd(pendingBtyvs));
        hd(btyvars) = NIL;
    }
}

/* --------------------------------------------------------------------------
 * Scopes:
 *
 * A scope is opened for each construct that can bind skolem constants and
 * bring local evidence into being: a lambda, a case or function
 * alternative, a generator, a let, and each top-level binding group.
 * While it is open, the predicates the type checker collects belong to it.
 * On leaving, those that the local evidence discharges are discharged, the
 * rest are handed back to the enclosing scope, and the skolems must not
 * have escaped into the result type or the assumptions.
 *
 * Bound type variables (btyvars, in subst.c) and the assumption levels
 * above have nestings of their own that do not always coincide with this
 * one, so they are kept apart.
 * ------------------------------------------------------------------------*/

static Void local enterScope() {
    TcScope *sc;
    if (numScopes == maxScopes) {
        Int more = maxScopes ? 2 * maxScopes : 64;
        TcScope *grown = realloc(scopes, more * sizeof(TcScope));
        if (!grown) {
            errHead(0);
            errPrintf("Cannot allocate type checker scopes");
            errFail();
        }
        scopes = grown;
        maxScopes = more;
    }
    sc = scopes + numScopes++;
    sc->skolems = NIL;
    sc->givens = NIL;
    sc->savedPreds = preds;
    preds = NIL;
}

static Void local leaveScope(Int l, Type t, Int o, Int m) {
    TcScope *sc = scope();

    if (nonNull(sc->givens)) { /* Check for local predicates       */
        List sks = sc->skolems;
        List sps = NIL;
        if (isNull(sks)) {
            internal("leaveScope");
        }
        markAllVars(); /* Mark all variables in current   */
        do {           /* substitution, then unmark sks.  */
            tyvar(intOf(fst(hd(sks))))->offs = UNUSED_GENERIC;
            sks = tl(sks);
        } while (nonNull(sks));
        normPreds(l);
        sps = elimPredsUsing(sc->givens, sps);
        preds = revOnto(preds, sps);
    }

    if (nonNull(sc->skolems)) { /* Check that Skolem vars do not   */
        List vs;                /* escape their scope              */
        Int i = 0;

        clearMarks();      /* Look for occurences in the           */
        for (; i < m; i++) /* inferred type                */
            markTyvar(o + i);
        markType(t, o);

        for (vs = sc->skolems; nonNull(vs); vs = tl(vs)) {
            Int vn = intOf(fst(hd(vs)));
            if (tyvar(vn)->offs == FIXED_TYVAR) {
                Cell tv = copyTyvar(vn);
                Type ty = liftRank2(t, o, m);
                errHead(l);
                errPrintf("Existentially quantified variable in inferred type");

                errPrintf("\n*** Variable     : ");
                errType(tv);
                errPrintf("\n*** From pattern : ");
                errExpr(snd(hd(vs)));
                errPrintf("\n*** Result type  : ");
                errType(ty);
                errPrintf("\n");
                errFail();
            }
        }

        markBtyvs(); /* Now check assumptions      */
        mapProc(markAssumList, defnBounds);
        mapProc(markAssumList, varsBounds);
        mapProc(markPred, preds);

        for (vs = sc->skolems; nonNull(vs); vs = tl(vs)) {
            Int vn = intOf(fst(hd(vs)));
            if (tyvar(vn)->offs == FIXED_TYVAR) {
                errHead(l);
                errPrintf(
                    "Existentially quantified variable escapes from pattern ");
                errExpr(snd(hd(vs)));
                errPrintf("\n");
                errFail();
            }
        }
    }
    preds = revOnto(preds, sc->savedPreds);
    numScopes--;
}

/* --------------------------------------------------------------------------
 * Type errors:
 * ------------------------------------------------------------------------*/

static Void local typeError(Int l, Cell e, Cell in, String wh, Type t, Int o)
/* line number near type error     */
/* source of error                 */
/* context if any (NIL if not)     */
/* place in which error occurs     */
/* should be of type (t,o)         */
{ /* type inferred is (typeIs,typeOff) */

    clearMarks(); /* types printed here are monotypes  */
                  /* use marking to give sensible names*/

    reportTypeError(l, e, in, wh, copyType(typeIs, typeOff), copyType(t, o));
}

static Void local reportTypeError(Int l, Cell e, Cell in, String wh, Type inft,
                                  Type expt)
/* Error printing part of typeError*/
{
    errHead(l);
    errPrintf("Type error in %s", wh);
    if (nonNull(in)) {
        errPrintf("\n*** Expression     : ");
        errExpr(in);
    }
    errPrintf("\n*** Term           : ");
    errExpr(e);
    errPrintf("\n*** Type           : ");
    errType(inft);
    errPrintf("\n*** Does not match : ");
    errType(expt);
    if (unifyFails) {
        errPrintf("\n*** Because        : %s", unifyFails);
    }
    errPrintf("\n");
    errFail();
}

static Void local shouldBe(Int l, Cell e, Cell in, String where, Type t,
                           Int o) { /* unify the inferred type with (t,o) */
    if (!unify(typeIs, typeOff, t, o))
        typeError(l, e, in, where, t, o);
}

static Cell local check(Int l, Cell e, Cell in, String where, Type t,
                        Int o) { /* type check e against (t,o); returns */
    e = typeExpr(l, e);          /* the translated e               */
    shouldBe(l, e, in, where, t, o);
    return e;
}

static Void local cantEstablish(Int line, String wh, Cell e, Type t, List ps)
/* Complain when declared preds    */
/* are not sufficient to discharge */
/* or defer the inferred context.  */
{
    errHead(line);
    errPrintf("Cannot justify constraints in %s", wh);
    errPrintf("\n*** Expression    : ");
    errExpr(e);
    errPrintf("\n*** Type          : ");
    errType(t);
    errPrintf("\n*** Given context : ");
    errContext(ps);
    errPrintf("\n*** Constraints   : ");
    errContext(copyPreds(preds));
    errPrintf("\n");
    errFail();
}

static Void local tooGeneral(Int l, Cell e, Type dt,
                             Type it) /* explicit type sig. too general  */
{
    errHead(l);
    errPrintf("Inferred type is not general enough");
    errPrintf("\n*** Expression    : ");
    errExpr(e);
    errPrintf("\n*** Expected type : ");
    errType(dt);
    errPrintf("\n*** Inferred type : ");
    errType(it);
    errPrintf("\n");
    errFail();
}

/* --------------------------------------------------------------------------
 * Typing of expressions:
 * ------------------------------------------------------------------------*/

#define EXPRESSION 0  /* type checking expression          */
#define NEW_PATTERN 1 /* pattern, introducing new vars   */
#define OLD_PATTERN 2 /* pattern, involving bound vars   */
static int tcMode = EXPRESSION;

static Cell local typeExpr(Int l, Cell e) /* Determine type of expr/pattern  */
{
    stackCheck();
    static String cond = "conditional";
    static String list = "list";
    static String discr = "case discriminant";
    static String aspat = "as (@) pattern";
    static String typeSig = "type annotation";
    static String lambda = "lambda expression";

    switch (whatIs(e)) {

        /* The following cases can occur in either pattern or expr. mode   */

    case AP:
    case NAME:
    case VAROPCELL:
    case VARIDCELL:
        return typeAp(l, e);

    case TUPLE:
        typeTuple(e);
        break;

    case POSNUM:
    case ZERONUM:
    case NEGNUM: {
        Int alpha = newTyvars(1);
        inferType(aVar, alpha);
        return ap(ap(nameFromInteger, assumeEvid(predNum, alpha)), e);
    }
    case INTCELL: {
        Int alpha = newTyvars(1);
        inferType(aVar, alpha);
        return ap(ap(nameFromInt, assumeEvid(predNum, alpha)), e);
    }

    case DOUBLECELL: {
        Int alpha = newTyvars(1);
        inferType(aVar, alpha);
        return ap(ap(nameFromDouble, assumeEvid(predFractional, alpha)), e);
    }

    case STRCELL:
        inferType(typeString, 0);
        break;

    case CHARCELL:
        inferType(typeChar, 0);
        break;

    case CONFLDS:
        typeConFlds(l, e);
        break;

    case ESIGN:
        snd(snd(e)) = localizeBtyvs(snd(snd(e)));
        return typeExpected(l, typeSig, fst(snd(e)), snd(snd(e)), 0, 0, FALSE);

        /* The following cases can only occur in expr mode                 */

    case UPDFLDS:
        typeUpdFlds(l, e);
        break;

    case COND: {
        Int beta = newTyvars(1);
        fst3(snd(e)) = check(l, fst3(snd(e)), e, cond, typeBool, 0);
        snd3(snd(e)) = check(l, snd3(snd(e)), e, cond, aVar, beta);
        thd3(snd(e)) = check(l, thd3(snd(e)), e, cond, aVar, beta);
        tyvarType(beta);
    } break;

    case LETREC:
        enterBindings();
        enterScope();
        mapProc(typeBindings, fst(snd(e)));
        snd(snd(e)) = typeExpr(l, snd(snd(e)));
        leaveBindings();
        leaveScope(l, typeIs, typeOff, 0);
        break;

    case FINLIST: {
        Int beta = newTyvars(1);
        List xs;
        for (xs = snd(e); nonNull(xs); xs = tl(xs)) {
            hd(xs) = check(l, hd(xs), e, list, aVar, beta);
        }
        inferType(listof, beta);
    } break;

    case DOCOMP:
        typeDo(l, e);
        break;

    case COMP:
        return typeMonadComp(l, e);

    case CASE: {
        Int beta = newTyvars(2); /* discr result */
        fst(snd(e)) = check(l, fst(snd(e)), NIL, discr, aVar, beta);
        map2Proc(typeCase, l, beta, snd(snd(e)));
        tyvarType(beta + 1);
    } break;

    case LAMBDA: {
        Int beta = newTyvars(1);
        enterPendingBtyvs();
        typeAlt(lambda, e, snd(e), aVar, beta, 1);
        leavePendingBtyvs();
        tyvarType(beta);
    } break;

        /* The remaining cases can only occur in pattern mode: */

    case WILDCARD:
        inferType(aVar, newTyvars(1));
        break;

    case ASPAT: {
        Int beta = newTyvars(1);
        snd(snd(e)) = typeExpr(l, snd(snd(e)));
        bindTv(beta, typeIs, typeOff);
        fst(snd(e)) = check(l, fst(snd(e)), e, aspat, aVar, beta);
        tyvarType(beta);
    } break;

    case LAZYPAT:
        snd(e) = typeExpr(l, snd(e));
        break;

    default:
        internal("typeExpr");
    }

    return e;
}

/* --------------------------------------------------------------------------
 * Typing rules for particular special forms:
 * ------------------------------------------------------------------------*/

static Cell local typeAp(Int l, Cell e) /* Type check application, which   */
                                        /* may be headed with a variable   */
{                                       /* requires polymorphism, qualified*/
    static String app = "application";  /* types, and possible rank2 args. */
    Cell h = getHead(e);
    Int n = argCount;
    Cell p = NIL;
    Cell a = e;
    Int i;

    switch (whatIs(h)) {
    case NAME:
        typeIs = nameRec(h)->type;
        break;

    case VAROPCELL:
    case VARIDCELL:
        if (tcMode == NEW_PATTERN) {
            inferType(aVar, newVarsBind(e));
        } else {
            Cell v = findAssum(textOf(h));
            if (nonNull(v)) {
                h = v;
                typeIs =
                    (tcMode == OLD_PATTERN) ? defType(typeIs) : useType(typeIs);
            } else {
                h = findName(textOf(h));
                if (isNull(h))
                    internal("typeAp0");
                typeIs = nameRec(h)->type;
            }
        }
        break;

    default:
        h = typeExpr(l, h);
        break;
    }

    if (isNull(typeIs))
        internal("typeAp1");

    instantiate(typeIs);     /* Deal with polymorphism ...         */
    if (nonNull(predsAre)) { /* ... and with qualified types.   */
        List evs = NIL;
        for (; nonNull(predsAre); predsAre = tl(predsAre)) {
            evs = cons(assumeEvid(hd(predsAre), typeOff), evs);
        }
        if (!isName(h) || !isCfun(h)) {
            h = applyToArgs(h, rev(evs));
        }
    }

    if (whatIs(typeIs) == CDICTS) { /* Deal with local dictionaries        */
        List evs = makePredAss(fst(snd(typeIs)), typeOff);
        List ps = evs;
        typeIs = snd(snd(typeIs));
        for (; nonNull(ps); ps = tl(ps)) {
            h = ap(h, thd3(hd(ps)));
        }
        if (tcMode == EXPRESSION) {
            preds = revOnto(evs, preds);
        } else {
            scope()->givens = revOnto(evs, scope()->givens);
        }
    }

    if (whatIs(typeIs) == EXIST) { /* Deal with existential arguments */
        Int n = intOf(fst(snd(typeIs)));
        typeIs = snd(snd(typeIs));
        if (!isCfun(getHead(h)) || n > typeFree) {
            internal("typeAp2");
        } else if (tcMode != EXPRESSION) {
            Int alpha = typeOff + typeFree;
            for (; n > 0; n--) {
                bindTv(alpha - n, SKOLEM, 0);
                scope()->skolems =
                    cons(pair(mkInt(alpha - n), e), scope()->skolems);
            }
        }
    }

    if (whatIs(typeIs) == RANK2) { /* Deal with rank 2 arguments           */
        Int alpha = typeOff;
        Int m = typeFree;
        Int nr2 = intOf(fst(snd(typeIs)));
        Type body = snd(snd(typeIs));
        List as = e;
        Bool added = FALSE;

        if (n < nr2) { /* Must have enough arguments       */
            errHead(l);
            errPrintf("Use of ");
            errExpr(h);
            if (n > 1) {
                errPrintf(" in ");
                errExpr(e);
            }
            errPrintf(" requires at least %d argument%s\n", nr2,
                      (nr2 == 1 ? "" : "s"));
            errFail();
        }

        for (i = nr2; i < n; ++i) /* Find rank two arguments       */
            as = fun(as);

        for (as = getArgs(as); nonNull(as); as = tl(as), body = arg(body)) {
            Type expect = dropRank1(arg(fun(body)), alpha, m);
            if (isPolyOrQualType(expect)) {
                if (tcMode == EXPRESSION) /* poly/qual type in expr  */
                    hd(as) =
                        typeExpected(l, app, hd(as), expect, alpha, m, TRUE);
                else if (hd(as) != WILDCARD) { /* Pattern binding/match   */
                    if (!isVar(hd(as))) {
                        errHead(l);
                        errPrintf("Argument ");
                        errExpr(arg(as));
                        errPrintf(" in pattern ");
                        errExpr(e);
                        errPrintf(" where a variable is required\n");
                        errFail();
                    }
                    if (tcMode == NEW_PATTERN) { /* Pattern match          */
                        if (m > 0 && !added) {
                            for (i = 0; i < m; i++)
                                addVarAssump(dummyVar, mkInt(alpha + i));
                            added = TRUE;
                        }
                        addVarAssump(hd(as), expect);
                    } else { /* Pattern binding    */
                        Text t = textOf(hd(as));
                        Cell a = findInAssumList(t, hd(defnBounds));
                        if (isNull(a))
                            internal("typeAp3");
                        instantiate(expect);
                        if (nonNull(predsAre)) {
                            errHead(l);
                            errPrintf("Cannot use pattern binding for ");
                            errExpr(hd(as));
                            errPrintf(
                                " as a component with a qualified type\n");
                            errFail();
                        }
                        shouldBe(l, hd(as), e, app, aVar,
                                 intOf(defType(snd(a))));
                    }
                }
            } else { /* Not a poly/qual type       */
                hd(as) = check(l, hd(as), e, app, expect, alpha);
            }
            h = ap(h, hd(as)); /* Save checked argument   */
        }
        inferType(body, alpha);
        n -= nr2;
    }

    if (n > 0) {                /* Deal with remaining args        */
        Int beta = funcType(n); /* check h::t1->t2->...->tn->rn+1  */
        shouldBe(l, h, e, app, aVar, beta);
        for (i = n; i > 0; --i) { /* check e_i::t_i for each i     */
            arg(a) = check(l, arg(a), e, app, aVar, beta + 2 * i - 1);
            p = a;
            a = fun(a);
        }
        tyvarType(beta + 2 * n); /* Inferred type is r_n+1          */
    }

    if (isNull(p)) /* Replace head with translation   */
        e = h;
    else
        fun(p) = h;

    return e;
}

static Cell local typeExpected(Int l, String wh, Cell e, Type reqd, Int alpha,
                               Int n, Bool addEvid)
/* Type check expression e in wh   */
/* at line l, expecting type reqd, */
/* and treating vars alpha through */
/* (alpha+n-1) as fixed.           */
{ /* TRUE => add \ev -> ...        */
    List savePreds = preds;
    Type t;
    Int o;
    Int m;
    List ps;
    Int i;

    instantiate(reqd);
    t = typeIs;
    o = typeOff;
    m = typeFree;
    ps = makePredAss(predsAre, o);

    preds = NIL;
    e = check(l, e, NIL, wh, t, o);
    improve(l, ps, preds);

    clearMarks();
    mapProc(markAssumList, defnBounds);
    mapProc(markAssumList, varsBounds);
    mapProc(markPred, savePreds);
    markBtyvs();

    if (n > 0) {                 /* mark alpha thru alpha+n-1, plus any   */
                                 /* type vars that are functionally        */
        List us = NIL, vs = NIL; /* dependent on them                      */
        List fds = calcFunDepsPreds(preds);
        for (i = 0; i < n; i++) {
            Type t1 = zonkTyvar(alpha + i);
            us = zonkTyvarsIn(t1, us);
        }
        vs = oclose(fds, us);
        for (; nonNull(vs); vs = tl(vs))
            markTyvar(intOf(hd(vs)));
    }

    normPreds(l);
    savePreds = elimPredsUsing(ps, savePreds);
    if (nonNull(preds) && resolveDefs(genvarType(t, o, NIL), FALSE))
        savePreds = elimPredsUsing(ps, savePreds);
    elimTauts();
    if (nonNull(preds)) {
        Type ty = copyType(t, o);
        List qs = copyPreds(ps);
        cantEstablish(l, wh, e, ty, qs);
    }

    resetGenerics();
    for (i = 0; i < m; i++)
        if (copyTyvar(o + i) != mkOffset(i)) {
            List qs = copyPreds(ps);
            Type it = copyType(t, o);
            tooGeneral(l, e, reqd, generalize(qs, it));
        }

    if (addEvid) {
        e = qualifyExpr(l, ps, e);
        preds = savePreds;
    } else
        preds = revOnto(ps, savePreds);

    inferType(t, o);
    return e;
}

static Void local typeAlt(String wh, Cell e, Cell a, Type t, Int o,
                          Int m) /* Type check abstraction (Alt)           */
                                 /* a = ( [p1, ..., pn], rhs )     */
{
    Type origt = t;
    List ps = fst(a) = patBtyvs(fst(a));
    Int n = length(ps);
    Int l = rhsLine(snd(a));
    Int nr2 = 0;
    List as = NIL;
    Bool added = FALSE;

    List savedVars = hd(varsBounds);
    enterScope();
    if (whatIs(t) == RANK2) {
        if (n < (nr2 = intOf(fst(snd(t))))) {
            errHead(l);
            errPrintf("Definition requires at least %d parameters on lhs",
                      intOf(fst(snd(t))));
            errFail();
        }
        t = snd(snd(t));
    }

    while (getHead(t) == typeArrow && argCount == 2 && nonNull(ps)) {
        Type ta = arg(fun(t));
        if (isPolyOrQualType(ta)) {
            if (hd(ps) != WILDCARD) {
                if (!isVar(hd(ps))) {
                    errHead(l);
                    errPrintf("Argument ");
                    errExpr(hd(ps));
                    errPrintf(
                        " used where a variable or wildcard is required\n");
                    errFail();
                }
                if (m > 0 && !added) {
                    Int i = 0;
                    for (; i < m; i++)
                        addVarAssump(dummyVar, mkInt(o + i));
                    added = TRUE;
                }
                addVarAssump(hd(ps), ta);
            }
        } else {
            hd(ps) = typeFreshPat(l, hd(ps));
            shouldBe(l, hd(ps), NIL, wh, ta, o);
        }
        t = arg(t);
        ps = tl(ps);
        as = fn(ta, as);
        n--;
    }

    if (n == 0)
        snd(a) = typeRhs(snd(a));
    else {
        Int beta = funcType(n);
        Int i = 0;
        for (; i < n; ++i) {
            hd(ps) = typeFreshPat(l, hd(ps));
            bindTv(beta + 2 * i + 1, typeIs, typeOff);
            ps = tl(ps);
        }
        snd(a) = typeRhs(snd(a));
        bindTv(beta + 2 * n, typeIs, typeOff);
        tyvarType(beta);
    }

    if (!unify(typeIs, typeOff, t, o)) {
        Type req, got;
        clearMarks();
        req = liftRank2(origt, o, m);
        liftRank2Args(as, o, m);
        got =
            ap(RANK2, pair(mkInt(nr2), revOnto(as, copyType(typeIs, typeOff))));
        reportTypeError(l, e, NIL, wh, got, req);
    }

    hd(varsBounds) = savedVars;
    doneBtyvs(l);
    leaveScope(l, origt, o, m);
}

static Int local funcType(Int n)     /*return skeleton for function type*/
{                                    /*with n arguments, taking the form*/
    Int beta = newTyvars(2 * n + 1); /*    r1 t1 r2 t2 ... rn tn rn+1   */
    Int i;                           /* with r_i := t_i -> r_i+1           */
    for (i = 0; i < n; ++i)
        bindTv(beta + 2 * i, arrow, beta + 2 * i + 1);
    return beta;
}

static Void local typeCase(Int l, Int beta,
                           Cell c) /* type check case: pat -> rhs          */
                                   /* (case given by c == (pat,rhs))   */
                                   /* need:  pat :: (var,beta)     */
{                                  /*    rhs :: (var,beta+1)       */
    static String casePat = "case pattern";
    static String caseExpr = "case expression";

    List savedVars = hd(varsBounds);
    enterScope();
    fst(c) = typeFreshPat(l, patBtyvs(fst(c)));
    shouldBe(l, fst(c), NIL, casePat, aVar, beta);
    snd(c) = typeRhs(snd(c));
    shouldBe(l, rhsExpr(snd(c)), NIL, caseExpr, aVar, beta + 1);

    hd(varsBounds) = savedVars;
    doneBtyvs(l);
    leaveScope(l, typeIs, typeOff, 0);
}

static Void local typeComp(Int l, Type m, Cell e,
                           List qs) /* type check comprehension    */
                                    /* monad (mkOffset(0))                 */
{
    static String boolQual = "boolean qualifier";
    static String genQual = "generator";

    stackCheck();
    if (isNull(qs)) { /* no qualifiers left                */
        fst(e) = typeExpr(l, fst(e));
    } else {
        Cell q = hd(qs);
        List qs1 = tl(qs);
        switch (whatIs(q)) {
        case BOOLQUAL:
            snd(q) = check(l, snd(q), NIL, boolQual, typeBool, 0);
            typeComp(l, m, e, qs1);
            break;

        case QWHERE:
            enterBindings();
            enterScope();
            mapProc(typeBindings, snd(q));
            typeComp(l, m, e, qs1);
            leaveBindings();
            leaveScope(l, typeIs, typeOff, 0);
            break;

        case FROMQUAL: {
            Int beta = newTyvars(1);
            List savedVars = hd(varsBounds);
            enterPendingBtyvs();
            snd(snd(q)) = check(l, snd(snd(q)), NIL, genQual, m, beta);
            enterScope();
            fst(snd(q)) = typeFreshPat(l, patBtyvs(fst(snd(q))));
            shouldBe(l, fst(snd(q)), NIL, genQual, aVar, beta);
            typeComp(l, m, e, qs1);
            hd(varsBounds) = savedVars;
            leavePendingBtyvs();
            leaveScope(l, typeIs, typeOff, 0);
        } break;

        case DOQUAL:
            snd(q) = check(l, snd(q), NIL, genQual, m, newTyvars(1));
            typeComp(l, m, e, qs1);
            break;
        default:
            break;
        }
    }
}

static Cell local typeMonadComp(Int l,
                                Cell e) /* type check monad comprehension  */
{
    Int alpha = newTyvars(1);
    Int beta = newTyvars(1);
    Cell mon = ap(mkInt(beta), aVar);
    Cell m = assumeEvid(predMonad, beta);
    tyvar(beta)->kind = starToStar;
    bindTv(beta, typeList, 0);
    m = nameListMonad;

    typeComp(l, mon, snd(e), snd(snd(e)));
    bindTv(alpha, typeIs, typeOff);
    inferType(mon, alpha);
    return ap(MONADCOMP, pair(m, snd(e)));
}

static Void local typeDo(Int l, Cell e) /* type check do-notation          */
{
    static String finGen = "final generator";
    Int alpha = newTyvars(1);
    Int beta = newTyvars(1);
    Cell mon = ap(mkInt(beta), aVar);
    Cell m = assumeEvid(predMonad, beta);
    tyvar(beta)->kind = starToStar;

    typeComp(l, mon, snd(e), snd(snd(e)));
    shouldBe(l, fst(snd(e)), NIL, finGen, mon, alpha);
    snd(e) = pair(m, snd(e));
}

static Void local typeConFlds(Int l,
                              Cell e) /* Type check a construction         */
{
    static String conExpr = "value construction";
    Name c = fst(snd(e));
    List fs = snd(snd(e));
    Type tc;
    Int to;
    Int tf;
    Int i;

    instantiate(nameRec(c)->type);
    for (; nonNull(predsAre); predsAre = tl(predsAre))
        assumeEvid(hd(predsAre), typeOff);
    if (whatIs(typeIs) == RANK2)
        typeIs = snd(snd(typeIs));
    tc = typeIs;
    to = typeOff;
    tf = typeFree;

    for (; nonNull(fs); fs = tl(fs)) {
        Type t = tc;
        for (i = sfunPos(fst(hd(fs)), c); --i > 0; t = arg(t))
            ;
        t = dropRank1(arg(fun(t)), to, tf);
        if (isPolyOrQualType(t))
            snd(hd(fs)) =
                typeExpected(l, conExpr, snd(hd(fs)), t, to, tf, TRUE);
        else {
            snd(hd(fs)) = check(l, snd(hd(fs)), e, conExpr, t, to);
        }
    }
    for (i = nameRec(c)->arity; i > 0; i--)
        tc = arg(tc);
    inferType(tc, to);
}

static Void local typeUpdFlds(Int line,
                              Cell e) /* Type check an update              */
                                      /* (Written in what might seem a   */
{                                     /* bizarre manner for the benefit  */
    static String update = "update";  /* of as yet unreleased extensions)*/
    List cs = snd3(snd(e));           /* List of constructors              */
    List fs = thd3(snd(e));           /* List of field specifications      */
    List ts = NIL;                    /* List of types for fields          */
    Int n = length(fs);
    Int alpha = newTyvars(2 + n);
    Int i;
    List fs1;

    /* Calculate type and translation for each expr in the field list      */
    for (fs1 = fs, i = alpha + 2; nonNull(fs1); fs1 = tl(fs1), i++) {
        snd(hd(fs1)) = typeExpr(line, snd(hd(fs1)));
        bindTv(i, typeIs, typeOff);
    }

    clearMarks();
    mapProc(markAssumList, defnBounds);
    mapProc(markAssumList, varsBounds);
    mapProc(markPred, preds);
    markBtyvs();

    for (fs1 = fs, i = alpha + 2; nonNull(fs1); fs1 = tl(fs1), i++) {
        resetGenerics();
        ts = cons(generalize(NIL, copyTyvar(i)), ts);
    }
    ts = rev(ts);

    /* Type check expression to be updated                                 */
    fst3(snd(e)) = typeExpr(line, fst3(snd(e)));
    bindTv(alpha, typeIs, typeOff);

    for (; nonNull(cs); cs = tl(cs)) { /* Loop through constrs             */
        Name c = hd(cs);
        List ta = replicate(nameRec(c)->arity, NIL);
        Type td, tr;
        Int od, or ;

        tcMode = NEW_PATTERN; /* Domain type                       */
        instantiate(nameRec(c)->type);
        tcMode = EXPRESSION;
        td = typeIs;
        od = typeOff;
        for (; nonNull(predsAre); predsAre = tl(predsAre))
            assumeEvid(hd(predsAre), typeOff);

        if (whatIs(td) == RANK2) /* Skip rank2 annotation, if any   */
            td = snd(snd(td));

        instantiate(
            nameRec(c)->type); /* Range type                           */
        tr = typeIs;
        or = typeOff;
        for (; nonNull(predsAre); predsAre = tl(predsAre))
            assumeEvid(hd(predsAre), typeOff);

        if (whatIs(tr) == RANK2) /* Skip rank2 annotation, if any   */
            tr = snd(snd(tr));

        for (fs1 = fs, i = 1; nonNull(fs1); fs1 = tl(fs1), i++) {
            Int n = sfunPos(fst(hd(fs1)), c);
            Cell ta1 = ta;
            for (; n > 1; n--)
                ta1 = tl(ta1);
            hd(ta1) = mkInt(i);
        }

        for (; nonNull(ta); ta = tl(ta)) { /* For each cfun arg    */
            if (nonNull(hd(ta))) {         /* Field to updated?    */
                Int n = intOf(hd(ta));
                Cell f = fs;
                Cell t = ts;
                for (; n-- > 1; f = tl(f), t = tl(t))
                    ;

                if (isPolyOrQualType(arg(fun(td)))) {
                    errHead(line);
                    errPrintf(
                        "Sorry, record update syntax cannot currently be used for polymorphic components");
                    errFail();
                }

                f = hd(f);
                t = hd(t);
                instantiate(t);
                shouldBe(line, snd(f), e, update, arg(fun(tr)), or);
            } /* Unmentioned component   */
            else if (!unify(arg(fun(td)), od, arg(fun(tr)), or))
                internal("typeUpdFlds");

            tr = arg(tr);
            td = arg(td);
        }

        inferType(td, od); /* Check domain type    */
        shouldBe(line, fst3(snd(e)), e, update, aVar, alpha);
        inferType(tr, or); /* Check range type     */
        shouldBe(line, e, NIL, update, aVar, alpha + 1);
    }
    /* (typeIs,typeOff) still carry the result type when we exit the loop  */
}

static Cell local typeFreshPat(Int l,
                               Cell p) /* find type of pattern, assigning  */
                                       /* fresh type variables to each var */
{                                      /* bound in the pattern             */
    tcMode = NEW_PATTERN;
    p = typeExpr(l, p);
    tcMode = EXPRESSION;
    return p;
}

/* --------------------------------------------------------------------------
 * Type check group of bindings:
 * ------------------------------------------------------------------------*/

static Void local typeBindings(List bs) /* type check a binding group      */
{
    Bool usesPatBindings = FALSE; /* TRUE => pattern binding in bs   */
    Bool usesUntypedVar = FALSE;  /* TRUE => var bind w/o type decl  */
    List bs1;

    /* The following loop is used to determine whether the monomorphism    */
    /* restriction should be applied.  It could be written marginally more */
    /* efficiently by using breaks, but clarity is more important here ... */

    for (bs1 = bs; nonNull(bs1); bs1 = tl(bs1)) { /* Analyse binding group    */
        Cell b = hd(bs1);
        if (!isVar(fst(b)))
            usesPatBindings = TRUE;
        else if (isNull(fst(hd(snd(snd(b)))))       /* no arguments        */
                 && whatIs(fst(snd(b))) == IMPDEPS) /* implicitly typed*/
            usesUntypedVar = TRUE;
    }

    if (usesPatBindings || usesUntypedVar)
        monorestrict(bs);
    else
        unrestricted(bs);

    elimTauts(); /* clean up any additional */
                 /* tauts that arose due to */
                 /* late-stage `improvement' */

    mapProc(removeTypeSigs, bs);              /* Remove binding type info */
    hd(varsBounds) = revOnto(hd(defnBounds),  /* transfer completed assmps*/
                             hd(varsBounds)); /* out of defnBounds        */
    hd(defnBounds) = NIL;
    hd(depends) = NIL;
}

static Void local removeTypeSigs(Cell b) /* Remove type info from a binding  */
{
    snd(b) = snd(snd(b));
}

/* --------------------------------------------------------------------------
 * Type check a restricted binding group:
 * ------------------------------------------------------------------------*/

static Void local monorestrict(List bs) /* Type restricted binding group   */
{
    List savePreds = preds;
    Int line = isVar(fst(hd(bs))) ? rhsLine(snd(hd(snd(snd(hd(bs))))))
                                  : rhsLine(snd(snd(snd(hd(bs)))));
    hd(defnBounds) = NIL;
    hd(depends) = NODEPENDS; /* No need for dependents here        */

    preds = NIL; /* Type check the bindings        */
    mapProc(restrictedBindAss, bs);
    mapProc(typeBind, bs);
    improve(line, NIL, preds);
    normPreds(line);
    elimTauts();
    preds = revOnto(preds, savePreds);

    clearMarks(); /* Mark fixed variables                  */
    mapProc(markAssumList, tl(defnBounds));
    mapProc(markAssumList, tl(varsBounds));
    mapProc(markPred, preds);
    markBtyvs();

    if (isNull(tl(defnBounds))) { /* Top-level may need defaulting   */
        normPreds(line);
        if (nonNull(preds) && resolveDefs(genvarAnyAss(hd(defnBounds)), FALSE))
            elimTauts();

        clearMarks();
        reducePreds();
        improve(line, NIL, preds);
        if (nonNull(preds))
            resolveDefs(NIL, FALSE); /* Nearly Haskell 1.4?                */
        elimTauts();

        if (nonNull(preds)) { /* Look for unresolved overloading */
            Cell v = isVar(fst(hd(bs))) ? fst(hd(bs)) : hd(fst(hd(bs)));
            Cell ass = findInAssumList(textOf(v), hd(varsBounds));
            preds = scSimplify(preds);

            errHead(line);
            errPrintf("Unresolved top-level overloading");
            errPrintf("\n*** Binding             : %s", textToStr(textOf(v)));

            if (nonNull(ass)) {
                errPrintf("\n*** Inferred type       : ");
                errType(snd(ass));
            }
            errPrintf("\n*** Outstanding context : ");
            errContext(copyPreds(preds));
            errPrintf("\n");
            errFail();
        }
    }

    map1Proc(genBind, NIL, bs); /* Generalize types of def'd vars  */
}

static Void local
restrictedBindAss(Cell b) /* Make assums for vars in binding */
{                         /* gp with restricted overloading  */

    if (isVar(fst(b))) { /* function-binding?              */
        Cell t = fst(snd(b));
        if (whatIs(t) == IMPDEPS) { /* Discard implicitly typed deps   */
            fst(snd(b)) = t = NIL;  /* in a restricted binding group.  */
        }
        fst(snd(b)) = localizeBtyvs(t);
        restrictedAss(rhsLine(snd(hd(snd(snd(b))))), fst(b), t);
    } else { /* pattern-binding?                   */
        List vs = fst(b);
        List ts = fst(snd(b));
        Int line = rhsLine(snd(snd(snd(b))));

        for (; nonNull(vs); vs = tl(vs)) {
            if (nonNull(ts)) {
                restrictedAss(line, hd(vs), hd(ts) = localizeBtyvs(hd(ts)));
                ts = tl(ts);
            } else {
                restrictedAss(line, hd(vs), NIL);
            }
        }
    }
}

static Void local restrictedAss(Int l, Cell v,
                                Type t) /* Assume that type of binding var v*/
                                        /* is t (if nonNull) in restricted  */
                                        /* binding group                   */
{
    newDefnBind(v, t);
    if (nonNull(predsAre)) {
        errHead(l);
        errPrintf("Explicit overloaded type for \"%s\"", textToStr(textOf(v)));

        errPrintf(" not permitted in restricted binding");
        errFail();
    }
}

/* --------------------------------------------------------------------------
 * Unrestricted binding group:
 * ------------------------------------------------------------------------*/

static Void local unrestricted(List bs) /* Type unrestricted binding group */
{
    List savePreds = preds;
    List imps = NIL; /* Implicitly typed bindings          */
    List exps = NIL; /* Explicitly typed bindings          */
    List bs1;

    /* ----------------------------------------------------------------------
     * STEP 1: Separate implicitly typed bindings from explicitly typed 
     * bindings and do a dependency analyis, where f depends on g iff f
     * is implicitly typed and involves a call to g.
     * --------------------------------------------------------------------*/

    for (; nonNull(bs); bs = tl(bs)) {
        Cell b = hd(bs);
        if (whatIs(fst(snd(b))) == IMPDEPS)
            imps = cons(b, imps); /* N.B. New lists are built to           */
        else                      /* avoid breaking the original           */
            exps = cons(b, exps); /* list structure for bs.        */
    }

    for (bs = imps; nonNull(bs); bs = tl(bs)) {
        Cell b = hd(bs);            /* Restrict implicitly typed dep   */
        List ds = snd(fst(snd(b))); /* lists to bindings in imps           */
        List cs = NIL;
        while (nonNull(ds)) {
            bs1 = tl(ds);
            if (cellIsMember(hd(ds), imps)) {
                tl(ds) = cs;
                cs = ds;
            }
            ds = bs1;
        }
        fst(snd(b)) = cs;
    }
    imps = itbscc(imps); /* Dependency analysis on imps    */
    for (bs = imps; nonNull(bs); bs = tl(bs))
        for (bs1 = hd(bs); nonNull(bs1); bs1 = tl(bs1))
            fst(snd(hd(bs1))) = NIL; /* reset imps type fields     */

    /* ----------------------------------------------------------------------
     * STEP 2: Add type assumptions about any explicitly typed variable.
     * --------------------------------------------------------------------*/

    for (bs = exps; nonNull(bs); bs = tl(bs)) {
        fst(snd(hd(bs))) = localizeBtyvs(fst(snd(hd(bs))));
        hd(varsBounds) =
            cons(pair(fst(hd(bs)), fst(snd(hd(bs)))), hd(varsBounds));
    }

    /* ----------------------------------------------------------------------
     * STEP 3: Calculate types for each group of implicitly typed bindings.
     * --------------------------------------------------------------------*/

    for (; nonNull(imps); imps = tl(imps)) {
        Cell b = hd(hd(imps));
        Int line = isVar(fst(b)) ? rhsLine(snd(hd(snd(snd(b)))))
                                 : rhsLine(snd(snd(snd(b))));
        hd(defnBounds) = NIL;
        hd(depends) = NIL;
        for (bs1 = hd(imps); nonNull(bs1); bs1 = tl(bs1))
            newDefnBind(fst(hd(bs1)), NIL);

        preds = NIL;
        mapProc(typeBind, hd(imps));
        improve(line, NIL, preds);

        clearMarks();
        mapProc(markAssumList, tl(defnBounds));
        mapProc(markAssumList, tl(varsBounds));
        mapProc(markPred, savePreds);
        markBtyvs();

        normPreds(line);
        savePreds = elimOuterPreds(savePreds);
        if (nonNull(preds) &&
            resolveDefs(genvarAllAss(hd(defnBounds)), FALSE)) {
            savePreds = elimOuterPreds(savePreds);
        }

        map1Proc(genBind, preds, hd(imps));
        if (nonNull(preds)) {
            map1Proc(addEvidParams, preds, hd(depends));
            map1Proc(qualifyBinding, preds, hd(imps));
        }

        hd(varsBounds) = revOnto(hd(defnBounds), hd(varsBounds));
    }

    /* ----------------------------------------------------------------------
     * STEP 4: Now infer a type for each explicitly typed variable and
     * check for compatibility with the declared type.
     * --------------------------------------------------------------------*/

    for (; nonNull(exps); exps = tl(exps)) {
        static String extbind = "explicitly typed binding";
        Cell b = hd(exps);
        List alts = snd(snd(b));
        Int line = rhsLine(snd(hd(alts)));
        Type t;
        Int o;
        Int m;
        List ps;

        hd(defnBounds) = NIL;
        hd(depends) = NODEPENDS;
        preds = NIL;

        instantiate(fst(snd(b)));
        o = typeOff;
        m = typeFree;
        t = dropRank2(typeIs, o, m);
        ps = makePredAss(predsAre, o);

        enterPendingBtyvs();
        for (; nonNull(alts); alts = tl(alts))
            typeAlt(extbind, fst(b), hd(alts), t, o, m);
        improve(line, ps, preds);
        leavePendingBtyvs();

        if (nonNull(ps)) /* Add dict params, if necessary   */
            qualifyBinding(ps, b);

        clearMarks();
        mapProc(markAssumList, tl(defnBounds));
        mapProc(markAssumList, tl(varsBounds));
        mapProc(markPred, savePreds);
        markBtyvs();

        normPreds(line);
        savePreds = elimPredsUsing(ps, savePreds);
        if (nonNull(preds)) {
            List vs = NIL;
            Int i = 0;
            for (; i < m; ++i)
                vs = cons(mkInt(o + i), vs);
            if (resolveDefs(vs, FALSE)) {
                savePreds = elimPredsUsing(ps, savePreds);
            }
            if (nonNull(preds)) {
                clearMarks();
                reducePreds();
                if (nonNull(preds) && resolveDefs(vs, FALSE))
                    savePreds = elimPredsUsing(ps, savePreds);
            }
        }

        resetGenerics(); /* Make sure we're general enough  */
        ps = copyPreds(ps);
        t = generalize(ps, liftRank2(t, o, m));

        if (!sameSchemes(t, fst(snd(b))))
            tooGeneral(line, fst(b), fst(snd(b)), t);

        if (nonNull(preds)) /* Check context was strong enough */
            cantEstablish(line, extbind, fst(b), t, ps);
    }

    preds = savePreds; /* Restore predicates       */
    hd(defnBounds) = NIL;
}

static Cell local itbDepends(Cell t) /* dependency list of an implicitly*/
{                                    /* typed binding              */
    return fst(snd(t));
}

static Void local itbSetDepends(Cell t, Cell v) {
    fst(snd(t)) = v;
}

static List local itbscc(List bs) /* scc for implicitly typed binds  */
{
    return sccSort(bs, NIL, itbDepends, itbSetDepends);
}

static Void local addEvidParams(List qs,
                                Cell v) /* overwrite VARID/OPCELL v with    */
                                        /* application of variable to evid. */
{                                       /* parameters given by qs           */
    if (nonNull(qs)) {
        Cell nv;

        if (!isVar(v))
            internal("addEvidParams");

        for (nv = mkVar(textOf(v)); nonNull(tl(qs)); qs = tl(qs))
            nv = ap(nv, thd3(hd(qs)));
        fst(v) = nv;
        snd(v) = thd3(hd(qs));
    }
}

/* --------------------------------------------------------------------------
 * Type check bodies of class and instance declarations:
 * ------------------------------------------------------------------------*/

static Void local typeClassDefn(Class c) /* Type check implementations of   */
{ /* defaults for class c                   */

    /* ----------------------------------------------------------------------
     * Generate code for default dictionary builder functions:
     * --------------------------------------------------------------------*/

    Int beta = newKindedVars(cclassRec(c)->kinds);
    Cell d = inventDictVar();
    List dparam = singleton(triple(cclassRec(c)->head, mkInt(beta), d));
    List mems = cclassRec(c)->members;
    List defs = cclassRec(c)->defaults;
    List dsels = cclassRec(c)->dsels;
    Cell pat = cclassRec(c)->dcon;
    Int width = cclassRec(c)->numSupers + cclassRec(c)->numMembers;
    Int i = 0;

    if (isNull(defs) && nonNull(mems)) {
        defs = cclassRec(c)->defaults = cons(NIL, NIL);
    }

    for (; nonNull(mems); mems = tl(mems)) {
        Name n;
        Text member = nameRec(hd(mems))->text;
        n = newName(inventText(), c);

        if (isNull(hd(defs))) { /* No default definition           */
            nameRec(n)->line = cclassRec(c)->line;
            nameRec(n)->arity = 1;
            nameRec(n)->defn = singleton(pair(
                singleton(d),
                ap(mkInt(cclassRec(c)->line),
                   ap(namePrimThrow, ap(nameNoMethodError, mkStr(member))))));
        } else { /* User supplied default defn     */
            List alts = snd(hd(defs));
            Int line = rhsLine(snd(hd(alts)));

            typeMember("default member binding", hd(mems), alts, dparam,
                       cclassRec(c)->head, beta);

            nameRec(n)->line = line;
            nameRec(n)->arity = 1 + length(fst(hd(alts)));
            nameRec(n)->defn = alts;

            for (; nonNull(alts); alts = tl(alts)) {
                fst(hd(alts)) = cons(d, fst(hd(alts)));
            }
        }

        hd(defs) = n;
        genDefns = cons(n, genDefns);
        if (isNull(tl(defs)) && nonNull(tl(mems))) {
            tl(defs) = cons(NIL, NIL);
        }
        defs = tl(defs);
    }

    /* ----------------------------------------------------------------------
     * Generate code for superclass and member function selectors:
     * --------------------------------------------------------------------*/

    for (i = 0; i < width; i++) {
        pat = ap(pat, inventVar());
    }
    pat = singleton(pat);
    for (i = 0; nonNull(dsels); dsels = tl(dsels)) {
        nameRec(hd(dsels))->defn = singleton(
            pair(pat, ap(mkInt(cclassRec(c)->line), nthArg(i++, hd(pat)))));
        genDefns = cons(hd(dsels), genDefns);
    }
    for (mems = cclassRec(c)->members; nonNull(mems); mems = tl(mems)) {
        nameRec(hd(mems))->defn = singleton(pair(
            pat, ap(mkInt(nameRec(hd(mems))->line), nthArg(i++, hd(pat)))));
        genDefns = cons(hd(mems), genDefns);
    }
}

static Void local typeInstDefn(Inst in) /* Type check implementations of   */
{                                       /* member functions for instance in*/

    /* ----------------------------------------------------------------------
     * Generate code for instance specific dictionary builder function:
     *
     *   inst.maker d1 ... dn = let sc1    = ...
     *                                     .
     *                                     .
     *                                     .
     *                              scm    = ...
     *                              vj ... = ...
     *                              d      = Make.C sc1 ... scm v1 ... vk
     *                          in d
     *
     * where sci are superclass dictionaries, d is a new name, vj
     * is a newly generated name corresponding to the implementation of a
     * member function.  (Additional line number values must be added at
     * appropriate places but, for clarity, these are not shown above.)
     * If no implementation of a particular vj is available, then we use
     * the default implementation, partially applied to d.
     * --------------------------------------------------------------------*/

    Int alpha = newKindedVars(cclassRec(instRec(in)->c)->kinds);
    List supers = makePredAss(cclassRec(instRec(in)->c)->supers, alpha);
    Int beta = newKindedVars(instRec(in)->kinds);
    List params = makePredAss(instRec(in)->specifics, beta);
    Cell d = inventDictVar();
    /* A previous comment indicated that evids may be wrong
       in some cases when you have overlapping instances.
       (the fix was to add self to the list of evidence)
       I can't reconstruct what I meant by that comment,
       and adding self can cause definitions that are unintentionally
       self-recursive. */
    List evids = dupList(params);

    List imps = instRec(in)->implements;
    Cell l = mkInt(instRec(in)->line);
    Cell dictDef = cclassRec(instRec(in)->c)->dcon;
    List mems = cclassRec(instRec(in)->c)->members;
    List defs = cclassRec(instRec(in)->c)->defaults;
    List args = NIL;
    List locs = NIL;
    List ps;

    if (!unifyPred(cclassRec(instRec(in)->c)->head, alpha, instRec(in)->head,
                   beta))
        internal("typeInstDefn");

    for (ps = params; nonNull(ps); ps = tl(ps)) /* Build arglist           */
        args = cons(thd3(hd(ps)), args);
    args = rev(args);

    for (ps = supers; nonNull(ps); ps = tl(ps)) { /* Superclass dictionaries */
        Cell pi = hd(ps);
        Cell ev = NIL;
#if EXPLAIN_INSTANCE_RESOLUTION
        if (showInstRes) {
            fputs("scEntail: ", stdout);
            printContext(stdout, copyPreds(params));
            fputs(" ||- ", stdout);
            printPred(stdout, copyPred(fst3(pi), intOf(snd3(pi))));
            fputc('\n', stdout);
        }
#endif
        ev = scEntail(params, fst3(pi), intOf(snd3(pi)), 0);
        if (isNull(ev)) {
#if EXPLAIN_INSTANCE_RESOLUTION
            if (showInstRes) {
                fputs("inEntail: ", stdout);
                printContext(stdout, copyPreds(evids));
                fputs(" ||- ", stdout);
                printPred(stdout, copyPred(fst3(pi), intOf(snd3(pi))));
                fputc('\n', stdout);
            }
#endif
            ev = inEntail(evids, fst3(pi), intOf(snd3(pi)), 0);
        }
        if (isNull(ev)) {
            clearMarks();
            errHead(instRec(in)->line);
            errPrintf("Cannot build superclass instance");
            errPrintf("\n*** Instance            : ");
            errPred(copyPred(instRec(in)->head, beta));
            errPrintf("\n*** Context supplied    : ");
            errContext(copyPreds(params));
            errPrintf("\n*** Required superclass : ");
            errPred(copyPred(fst3(pi), intOf(snd3(pi))));
            errPrintf("\n");
            errFail();
        }
        locs = cons(pair(thd3(pi), singleton(pair(NIL, ap(l, ev)))), locs);
        dictDef = ap(dictDef, thd3(pi));
    }

    for (; nonNull(defs); defs = tl(defs)) {
        Cell imp = NIL;
        if (nonNull(imps)) {
            imp = hd(imps);
            imps = tl(imps);
        }
        if (isNull(imp)) {
            dictDef = ap(dictDef, ap(hd(defs), d));
        } else {
            Cell v = inventVar();
            dictDef = ap(dictDef, v);
            typeMember("instance member binding", hd(mems), snd(imp), evids,
                       instRec(in)->head, beta);
            locs = cons(pair(v, snd(imp)), locs);
        }
        mems = tl(mems);
    }
    locs = cons(pair(d, singleton(pair(NIL, ap(l, dictDef)))), locs);

    nameRec(instRec(in)->builder)->defn /* Register builder imp    */
        = singleton(pair(args, ap(LETREC, pair(singleton(locs), ap(l, d)))));
    genDefns = cons(instRec(in)->builder, genDefns);
}

static Void local typeMember(String wh, Name mem, Cell alts, List evids,
                             Cell head, Int beta)
/* Type check alternatives alts of */
/* member mem for inst type head   */
/* at offset beta using predicate  */
/* assignment evids                */
{
    Int line = rhsLine(snd(hd(alts)));
    Type t;
    Int o;
    Int m;
    List ps;
    List qs;
    Type rt;

    instantiate(nameRec(mem)->type); /* Find required type             */
    o = typeOff;
    m = typeFree;
    t = dropRank2(typeIs, o, m);
    ps = makePredAss(predsAre, o);
    if (!unifyPred(hd(predsAre), typeOff, head, beta))
        internal("typeMember1");
    clearMarks();
    qs = copyPreds(ps);
    rt = generalize(qs, liftRank2(t, o, m));

    hd(defnBounds) = NIL; /* Type check each alternative           */
    hd(depends) = NODEPENDS;
    enterPendingBtyvs();
    for (preds = NIL; nonNull(alts); alts = tl(alts)) {
        typeAlt(wh, mem, hd(alts), t, o, m);
        qualify(tl(ps), hd(alts)); /* Add any extra dict params    */
    }
    improve(line, evids, preds);
    leavePendingBtyvs();

    evids = appendOnto(dupList(tl(ps)), /* Build full complement of dicts  */
                       evids);
    clearMarks();
    normPreds(line);
    qs = elimPredsUsing(evids, NIL);
    if (nonNull(preds) && resolveDefs(genvarType(t, o, NIL), FALSE))
        qs = elimPredsUsing(evids, qs);
    if (nonNull(qs)) {
        errHead(line);
        errPrintf("Implementation of %s requires extra context",
                  textToStr(nameRec(mem)->text));
        errPrintf("\n*** Expected type   : ");
        errType(rt);
        errPrintf("\n*** Missing context : ");
        errContext(copyPreds(qs));
        errPrintf("\n");
        errFail();
    }

    resetGenerics(); /* Make sure we're general enough  */
    ps = copyPreds(ps);
    t = generalize(ps, liftRank2(t, o, m));
    if (!sameSchemes(t, rt))
        tooGeneral(line, mem, rt, t);
    if (nonNull(preds)) {
        preds = scSimplify(preds);
        cantEstablish(line, wh, mem, t, ps);
    }
}

/* --------------------------------------------------------------------------
 * Type check bodies of bindings:
 * ------------------------------------------------------------------------*/

static Void local typeBind(Cell b) /* Type check binding                   */
{
    if (isVar(fst(b))) { /* function binding */
        Cell ass = findTopBinding(fst(b));
        Int beta;

        if (isNull(ass))
            internal("typeBind");

        beta = intOf(defType(snd(ass)));
        enterPendingBtyvs();
        map2Proc(typeDefAlt, beta, fst(b), snd(snd(b)));
        leavePendingBtyvs();
    } else { /* pattern binding  */
        static String lhsPat = "lhs pattern";
        static String rhs = "right hand side";
        Int beta = newTyvars(1);
        Pair pb = snd(snd(b));
        Int l = rhsLine(snd(pb));

        tcMode = OLD_PATTERN;
        enterPendingBtyvs();
        fst(pb) = patBtyvs(fst(pb));
        fst(pb) = check(l, fst(pb), NIL, lhsPat, aVar, beta);
        tcMode = EXPRESSION;
        snd(pb) = typeRhs(snd(pb));
        shouldBe(l, rhsExpr(snd(pb)), NIL, rhs, aVar, beta);
        doneBtyvs(l);
        leavePendingBtyvs();
    }
}

static Void local typeDefAlt(Int beta, Cell v,
                             Pair a) /* type check alt in func. binding  */
{
    static String valDef = "function binding";
    typeAlt(valDef, v, a, aVar, beta, 0);
}

static Cell local typeRhs(Cell e) /* check type of rhs of definition  */
{
    switch (whatIs(e)) {
    case GUARDED: {
        Int beta = newTyvars(1);
        map1Proc(guardedType, beta, snd(e));
        tyvarType(beta);
    } break;

    case LETREC:
        enterBindings();
        enterScope();
        mapProc(typeBindings, fst(snd(e)));
        snd(snd(e)) = typeRhs(snd(snd(e)));
        leaveBindings();
        leaveScope(rhsLine(snd(snd(e))), typeIs, typeOff, 0);
        break;

    case RSIGN:
        fst(snd(e)) = typeRhs(fst(snd(e)));
        shouldBe(rhsLine(fst(snd(e))), rhsExpr(fst(snd(e))), NIL, "result type",
                 snd(snd(e)), 0);
        return fst(snd(e));

    default:
        snd(e) = typeExpr(intOf(fst(e)), snd(e));
        break;
    }
    return e;
}

static Void local guardedType(Int beta,
                              Cell gded) /* check type of guard (li,(gd,ex))*/
                                         /* should have gd :: Bool,        */
{                                        /*           ex :: (var,beta)     */
    static String guarded = "guarded expression";
    static String guard = "guard";
    Int line = intOf(fst(gded));

    gded = snd(gded);
    fst(gded) = check(line, fst(gded), NIL, guard, typeBool, 0);
    snd(gded) = check(line, snd(gded), NIL, guarded, aVar, beta);
}

Cell rhsExpr(Cell rhs) /* find first expression on a rhs   */
{
    stackCheck();
    switch (whatIs(rhs)) {
    case GUARDED:
        return snd(snd(hd(snd(rhs))));
    case LETREC:
        return rhsExpr(snd(snd(rhs)));
    case RSIGN:
        return rhsExpr(fst(snd(rhs)));
    default:
        return snd(rhs);
    }
}

Int rhsLine(Cell rhs) /* find line number associated with */
{                     /* a right hand side                 */
    stackCheck();
    switch (whatIs(rhs)) {
    case GUARDED:
        return intOf(fst(hd(snd(rhs))));
    case LETREC:
        return rhsLine(snd(snd(rhs)));
    case RSIGN:
        return rhsLine(fst(snd(rhs)));
    default:
        return intOf(fst(rhs));
    }
}

/* --------------------------------------------------------------------------
 * Calculate generalization of types and compare with declared type schemes:
 * ------------------------------------------------------------------------*/

static Void local genBind(List ps, Cell b) /* Generalize the type of each var */
                                           /* defined in binding b, qualifying*/
{                                          /* each with the predicates in ps. */
    Cell v = fst(b);
    Cell t = fst(snd(b));

    if (isVar(fst(b)))
        genAss(rhsLine(snd(hd(snd(snd(b))))), ps, v, t);
    else {
        Int line = rhsLine(snd(snd(snd(b))));
        for (; nonNull(v); v = tl(v)) {
            Type ty = NIL;
            if (nonNull(t)) {
                ty = hd(t);
                t = tl(t);
            }
            genAss(line, ps, hd(v), ty);
        }
    }
}

static Void local genAss(Int l, List ps, Cell v,
                         Type dt) /* Calculate inferred type of v and*/
                                  /* compare with declared type, dt, */
                                  /* if given & check for ambiguity. */
{
    Cell ass = findTopBinding(v);

    if (isNull(ass))
        internal("genAss");

    snd(ass) = genTest(l, v, ps, dt, aVar, intOf(defType(snd(ass))));
}

static Type local genTest(Int l, Cell v, List ps, Type dt, Type t,
                          Int o) /* Generalize and test inferred           */
                                 /* type (t,o) with context ps     */
                                 /* against declared type dt for v. */
{
    Type bt = NIL; /* Body of inferred type        */
    Type it = NIL; /* Full inferred type                   */

    resetGenerics(); /* Calculate Haskell typing           */
    ps = copyPreds(ps);
    bt = copyType(t, o);
    it = generalize(ps, bt);

    if (nonNull(dt)) {   /* If a declared type was given,   */
        instantiate(dt); /* check body for match.          */
        if (!equalTypes(typeIs, bt))
            tooGeneral(l, v, dt, it);
    } else if (nonNull(ps))  /* Otherwise test for ambiguity in */
        if (isAmbiguous(it)) /* inferred type.             */
            ambigError(l, "inferred type", v, it);

    return it;
}

static Type local generalize(List qs,
                             Type t) /* calculate generalization of t   */
                                     /* having already marked fixed vars*/
{                                    /* with qualifying preds qs           */
    assertPredList(qs);
    assertType(t);
    if (nonNull(qs))
        t = ap(QUAL, pair(qs, t));
    if (nonNull(genericVars)) {
        Kind k = STAR;
        List vs = genericVars;
        for (; nonNull(vs); vs = tl(vs)) {
            Tyvar *tyv = tyvar(intOf(hd(vs)));
            Kind ka = tyv->kind;
            k = ap(ka, k);
        }
        t = mkPolyType(k, t);
    }
    return t;
}

static Bool local equalTypes(Type t1,
                             Type t2) /* Compare simple types for equality*/
{
    stackCheck();
et:
    if (whatIs(t1) != whatIs(t2))
        return FALSE;

    switch (whatIs(t1)) {
    case TYCON:
    case OFFSET:
    case TUPLE:
        return t1 == t2;

    case INTCELL:
        return intOf(t1) != intOf(t2);

    case AP:
        if (equalTypes(fun(t1), fun(t2))) {
            t1 = arg(t1);
            t2 = arg(t2);
            goto et;
        }
        return FALSE;

    default:
        internal("equalTypes");
    }

    return TRUE; /*NOTREACHED*/
}

/* --------------------------------------------------------------------------
 * Entry points to type checker:
 * ------------------------------------------------------------------------*/

Type typeCheckExp(Bool useDefs) /* Type check top level expression */
{                               /* using defaults if reqd          */
    Type type;
    List ctxt;
    Int beta;

    typeChecker(RESET);
    emptySubstitution();
    enterBindings();
    inputExpr = typeExpr(0, inputExpr);
    type = typeIs;
    beta = typeOff;
    clearMarks();
    improve(0, NIL, preds);
    normPreds(0);
    elimTauts();
    preds = scSimplify(preds);
    if (useDefs && nonNull(preds)) {
        clearMarks();
        reducePreds();
        if (nonNull(preds) && resolveDefs(NIL, TRUE)) /* Nearly Haskell 1.4? */
            elimTauts();
    }
    resetGenerics();
    ctxt = copyPreds(preds);
    type = generalize(ctxt, copyType(type, beta));
    inputExpr = qualifyExpr(0, preds, inputExpr);
    typeChecker(RESET);
    emptySubstitution();
    return type;
}

Void typeCheckDefns() { /* Type check top level bindings    */
    Target t = length(selDefns) + length(valDefns) + length(instDefns) +
               length(classDefns);
    Target i = 0;
    List gs;

    typeChecker(RESET);
    emptySubstitution();
    enterScope();
    enterBindings();
    setGoal("Type checking", t);

    for (gs = selDefns; nonNull(gs); gs = tl(gs)) {
        mapOver(typeSel, hd(gs));
        soFar(i++);
    }
    for (gs = valDefns; nonNull(gs); gs = tl(gs)) {
        typeDefnGroup(hd(gs));
        soFar(i++);
    }
    mapProc(typeForeignExport, foreignExports); /* ToDo: soFar magic */
    clearTypeIns();
    for (gs = classDefns; nonNull(gs); gs = tl(gs)) {
        emptySubstitution();
        typeClassDefn(hd(gs));
        soFar(i++);
    }
    for (gs = instDefns; nonNull(gs); gs = tl(gs)) {
        emptySubstitution();
        typeInstDefn(hd(gs));
        soFar(i++);
    }

    typeChecker(RESET);
    emptySubstitution();
    done();
}

static Void local typeDefnGroup(List bs) /* type check group of value defns */
{                                        /* (one top level scc)            */
    List as;

    emptySubstitution();
    hd(defnBounds) = NIL;
    preds = NIL;
    setTypeIns(bs);
    typeBindings(bs); /* find types for vars in bindings */

    if (nonNull(preds)) {
        Cell v = fst(hd(hd(varsBounds)));
        Name n = findName(textOf(v));
        Int l = nonNull(n) ? nameRec(n)->line : 0;
        preds = scSimplify(preds);
        errHead(l);
        errPrintf("Instance%s of ", (length(preds) == 1 ? "" : "s"));
        errContext(copyPreds(preds));
        errPrintf(" required for definition of ");
        errExpr(nonNull(n) ? n : v);
        errPrintf("\n");
        errFail();
    }

    if (nonNull(scope()->skolems)) {
        Cell b = hd(bs);
        Name n = findName(isVar(fst(b)) ? textOf(fst(b)) : textOf(hd(fst(b))));
        Int l = nonNull(n) ? nameRec(n)->line : 0;
        leaveScope(l, typeUnit, 0, 0);
        enterScope();
    }

    for (as = hd(varsBounds); nonNull(as); as = tl(as)) {
        Cell a = hd(as); /* add infered types to environment*/
        Name n = findName(textOf(fst(a)));
        if (isNull(n))
            internal("typeDefnGroup");
        nameRec(n)->type = snd(a);
    }
    hd(varsBounds) = NIL;
}

static Void local
typeForeignExport(Name n) /* Typecheck a foreign export decl */
{
    Int line = nameRec(n)->line;
    /* todo */
    errHead(line);
    errPrintf("Foreign export not implemented yet.");
    errFail();
}

static Pair local typeSel(Name s) /* Calculate a suitable type for a */
{                                 /* particular selector, s.       */
    List cns = nameRec(s)->defn;
    Int line = nameRec(s)->line;
    Type dom = NIL; /* Inferred domain             */
    Type rng = NIL; /* Inferred range              */
    Cell nv = inventVar();
    List alts = NIL;
    Int o;
    Int m;

    emptySubstitution();
    preds = NIL;

    for (; nonNull(cns); cns = tl(cns)) {
        Name c = fst(hd(cns));
        Int n = intOf(snd(hd(cns)));
        Int a = nameRec(c)->arity;
        Cell pat = c;
        Type dom1;
        Type rng1;
        Int o1;
        Int m1;

        instantiate(
            nameRec(c)->type); /* Instantiate constructor type         */
        o1 = typeOff;
        m1 = typeFree;
        for (; nonNull(predsAre); predsAre = tl(predsAre))
            assumeEvid(hd(predsAre), o1);

        if (whatIs(typeIs) == RANK2) /* Skip rank2 annotation, if any   */
            typeIs = snd(snd(typeIs));
        for (; --n > 0; a--) { /* Get range                        */
            pat = ap(pat, WILDCARD);
            typeIs = arg(typeIs);
        }
        rng1 = dropRank1(arg(fun(typeIs)), o1, m1);
        pat = ap(pat, nv);
        typeIs = arg(typeIs);
        while (--a > 0) { /* And then look for domain      */
            pat = ap(pat, WILDCARD);
            typeIs = arg(typeIs);
        }
        dom1 = typeIs;

        if (isNull(dom)) { /* Save first domain type and then */
            dom = dom1;    /* unify with subsequent domains to*/
            o = o1;        /* match up preds and range types  */
            m = m1;
        } else if (!unify(dom1, o1, dom, o))
            internal("typeSel1");

        if (isNull(rng)) /* Compare component types        */
            rng = rng1;
        else if (!sameSchemes(rng1, rng)) {
            clearMarks();
            rng = liftRank1(rng, o, m);
            rng1 = liftRank1(rng1, o1, m1);
            errHead(nameRec(s)->line);
            errPrintf("Mismatch in field types for selector \"%s\"",
                      textToStr(nameRec(s)->text));
            errPrintf("\n*** Field type     : ");
            errType(rng1);
            errPrintf("\n*** Does not match : ");
            errType(rng);
            errPrintf("\n");
            errFail();
        }
        alts = cons(pair(singleton(pat), pair(mkInt(line), nv)), alts);
    }
    alts = rev(alts);

    if (isNull(dom) || isNull(rng)) /* Should have been initialized by */
        internal("typeSel2");       /* now, assuming length cns >= 1.  */

    clearMarks();              /* No fixed variables here          */
    preds = scSimplify(preds); /* Simplify context                 */
    dom = copyType(dom, o);    /* Calculate domain type    */
    instantiate(rng);
    rng = copyType(typeIs, typeOff);
    if (nonNull(predsAre)) {
        List ps = makePredAss(predsAre, typeOff);
        List alts1 = alts;
        for (; nonNull(alts1); alts1 = tl(alts1)) {
            Cell body = nv;
            List qs = ps;
            for (; nonNull(qs); qs = tl(qs))
                body = ap(body, thd3(hd(qs)));
            snd(snd(hd(alts1))) = body;
        }
        preds = appendOnto(preds, ps);
    }
    nameRec(s)->type = generalize(copyPreds(preds), fn(dom, rng));
    nameRec(s)->arity = 1 + length(preds);
    map1Proc(qualify, preds, alts);

    return pair(s, alts);
}

/* --------------------------------------------------------------------------
 * Type checker control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const typeRoots[] = {
    &defnBounds,   &varsBounds,  &depends,        &pendingBtyvs, &dummyVar,
    &preds,        &stdDefaults, &arrow,          &boundPair,    &listof,
    &typeVarToVar, &predNum,     &predFractional, &predIntegral, &starToStar,
    &predMonad,    &typeProgIO,
};

Void typeChecker(Int what) {
    switch (what) {
    case RESET:
        tcMode = EXPRESSION;
        preds = NIL;
        pendingBtyvs = NIL;
        emptyAssumption();
        break;

    case MARK: {
        Int i = 0;                   /* the scope stack; the rest  */
        for (; i < numScopes; i++) { /* is in typeRoots    */
            mark(scopes[i].skolems);
            mark(scopes[i].givens);
            mark(scopes[i].savedPreds);
        }
    } break;

    case INSTALL:
        registerRoots(typeRoots);
        typeChecker(RESET);

        dummyVar = inventVar();

        modulePrelude = newModule(textPrelude);
        moduleUserPrelude = 0;
        setCurrModule(modulePrelude);

        starToStar = simpleKind(1);

        typeUnit = addPrimTycon(findText("()"), STAR, 0, DATATYPE, NIL);
        typeArrow =
            addPrimTycon(findText("(->)"), simpleKind(2), 2, DATATYPE, NIL);
        typeList = addPrimTycon(findText("[]"), starToStar, 1, DATATYPE, NIL);

        arrow = fn(aVar, bVar);
        listof = ap(typeList, aVar);
        boundPair = ap(ap(mkTuple(2), aVar), aVar);

        nameUnit = addPrimCfun(findText("()"), 0, 0, typeUnit);
        tyconRec(typeUnit)->defn = singleton(nameUnit);

        nameNil =
            addPrimCfun(findText("[]"), 0, 1, mkPolyType(starToStar, listof));
        nameCons =
            addPrimCfun(findText(":"), 2, 2,
                        mkPolyType(starToStar, fn(aVar, fn(listof, listof))));
        nameRec(nameCons)->syntax = mkSyntax(RIGHT_ASS, 5);

        tyconRec(typeList)->defn = cons(nameNil, cons(nameCons, NIL));

        typeVarToVar = fn(aVar, aVar);
        /* bogus definitions to avoid changing the prelude */
        addPrimCfun(findText("Rec"), 0, 0, typeUnit);
        addPrimCfun(findText("EmptyRow"), 0, 0, typeUnit);
        addPrimCfun(findText("EmptyRec"), 0, 0, typeUnit);

        break;
    }
}

static Name local linkName(String s) {
    Name n = findName(findText(s));
    if (isNull(n)) {
        errHead(0);
        errPrintf("Prelude does not define standard name \"%s\"", s);
        errFail();
    }
    return n;
}

static Tycon local linkTycon(String s) {
    Tycon tc = findTycon(findText(s));
    if (isNull(tc)) {
        errHead(0);
        errPrintf("Prelude does not define standard type \"%s\"", s);
        errFail();
    }
    return tc;
}

static Class local linkClass(String s) {
    Class cc = findClass(findText(s));
    if (isNull(cc)) {
        errHead(0);
        errPrintf("Prelude does not define standard class \"%s\"", s);
        errFail();
    }
    return cc;
}

Void linkPreludeTC() {      /* Hook to tycons and classes in   */
    if (isNull(typeBool)) { /* prelude when first loaded           */
        Int i;

        typeInt = linkTycon("Int");
        typeInt8 = linkTycon("Int8");
        typeInt16 = linkTycon("Int16");
        typeInt32 = linkTycon("Int32");
        typeInt64 = linkTycon("Int64");
        typeWord = linkTycon("Word"); /* deprecated */
        typeWord8 = linkTycon("Word8");
        typeWord16 = linkTycon("Word16");
        typeWord32 = linkTycon("Word32");
        typeWord64 = linkTycon("Word64");
        typeFunPtr = linkTycon("FunPtr");
        typePtr = linkTycon("Ptr");
        typeAddr = linkTycon("Addr"); /* deprecated */
        typeFloat = linkTycon("Float");
        typeDouble = linkTycon("Double");
        typeChar = linkTycon("Char");
        typeForeignP = linkTycon("ForeignPtr");
        typeForeign = linkTycon("ForeignObj"); /* deprecated */
        typeStable = linkTycon("StablePtr");
        typeBool = linkTycon("Bool");
        typeString = linkTycon("String");
        typeInteger = linkTycon("Integer");
        typeMaybe = linkTycon("Maybe");
        typeOrdering = linkTycon("Ordering");

        stdDefaults = cons(typeInteger, cons(typeDouble, NIL));

        classEq = linkClass("Eq");
        classOrd = linkClass("Ord");
        classIx = linkClass("Ix");
        classEnum = linkClass("Enum");
        classShow = linkClass("Show");
        classRead = linkClass("Read");
        classBounded = linkClass("Bounded");

        classReal = linkClass("Real");
        classIntegral = linkClass("Integral");
        classRealFrac = linkClass("RealFrac");
        classRealFloat = linkClass("RealFloat");
        classFractional = linkClass("Fractional");
        classFloating = linkClass("Floating");
        classNum = linkClass("Num");
        predNum = ap(classNum, aVar);
        predFractional = ap(classFractional, aVar);
        predIntegral = ap(classIntegral, aVar);

        classMonad = linkClass("Monad");
        predMonad = ap(classMonad, aVar);

        typeIO = linkTycon("IO");
        typeProgIO = ap(typeIO, aVar);

        /* The following primitives are referred to in derived instances and
         * hence require types; the following types are a little more general
         * than we might like, but they are the closest we can get without a
         * special datatype class.
         */
        nameRec(nameConCmp)->type =
            mkPolyType(starToStar, fn(aVar, fn(aVar, typeOrdering)));
        nameRec(nameEnRange)->type =
            mkPolyType(starToStar, fn(boundPair, listof));
        nameRec(nameEnIndex)->type =
            mkPolyType(starToStar, fn(boundPair, fn(aVar, typeInt)));
        nameRec(nameEnInRng)->type =
            mkPolyType(starToStar, fn(boundPair, fn(aVar, typeBool)));
        nameRec(nameEnToEn)->type =
            mkPolyType(starToStar, fn(aVar, fn(typeInt, aVar)));
        nameRec(nameEnFrEn)->type = mkPolyType(starToStar, fn(aVar, typeInt));
        nameRec(nameEnFrom)->type = mkPolyType(starToStar, fn(aVar, listof));
        nameRec(nameEnFrTo)->type = nameRec(nameEnFrTh)->type =
            mkPolyType(starToStar, fn(aVar, fn(aVar, listof)));

        for (i = 2; i <= NUM_DTUPLES;
             i++) { /* Add derived instances of tuples */
            addTupInst(classEq, i);
            addTupInst(classOrd, i);
            addTupInst(classShow, i);
            addTupInst(classRead, i);
            addTupInst(classIx, i);
        }
    }
}

Void linkPreludeCM() {       /* Hook to cfuns and mfuns in         */
    if (isNull(nameFalse)) { /* prelude when first loaded          */
        nameFalse = linkName("False");
        nameTrue = linkName("True");
        nameJust = linkName("Just");
        nameNothing = linkName("Nothing");
        nameLeft = linkName("Left");
        nameRight = linkName("Right");
        nameLT = linkName("LT");
        nameEQ = linkName("EQ");
        nameGT = linkName("GT");

        nameFromInt = linkName("fromInt");
        nameFromInteger = linkName("fromInteger");
        nameFromDouble = linkName("fromDouble");
        nameEq = linkName("==");
        nameCompare = linkName("compare");
        nameLe = linkName("<=");
        nameGt = linkName(">");
        nameShowsPrec = linkName("showsPrec");
        nameReadsPrec = linkName("readsPrec");
        nameIndex = linkName("index");
        nameInRange = linkName("inRange");
        nameRange = linkName("range");
        nameMult = linkName("*");
        namePlus = linkName("+");
        nameMinBnd = linkName("minBound");
        nameMaxBnd = linkName("maxBound");
        nameReturn = linkName("return");
        nameBind = linkName(">>=");
        nameThen = linkName(">>");
        nameMFail = linkName("fail");

        /* The constructor names better match up with the defn
           of IOErrorType in Prelude. 
        */
        nameIOError = linkName("IOError");
        nameAlreadyExists = linkName("AlreadyExists");
        nameDoesNotExist = linkName("NoSuchThing");
        nameAlreadyInUse = linkName("ResourceBusy");
        nameIsFull = linkName("ResourceExhausted");
        nameEOFErr = linkName("EOF");
        nameProtocolError = linkName("ProtocolError");
        nameIllegal = linkName("IllegalOperation");
        namePermDenied = linkName("PermissionDenied");
        nameUserErr = linkName("UserError");

        nameArithException = linkName("ArithException");
        nameArrayException = linkName("ArrayException");
        nameErrorCall = linkName("ErrorCall");
        nameIOException = linkName("IOException");
        nameNoMethodError = linkName("NoMethodError");
        nameNonTermination = linkName("NonTermination");
        namePatternMatchFail = linkName("PatternMatchFail");
        nameRecConError = linkName("RecConError");
        nameRecSelError = linkName("RecSelError");
        nameRecUpdError = linkName("RecUpdError");

        nameOverflow = linkName("Overflow");
        nameDivideByZero = linkName("DivideByZero");

        nameIndexOutOfBounds = linkName("IndexOutOfBounds");
        nameUndefinedElement = linkName("UndefinedElement");
    }
}

Void linkPreludeFuns() { /* Hook to cfuns and mfuns in     */
                         /* prelude when first loaded      */
}
/*-------------------------------------------------------------------------*/
