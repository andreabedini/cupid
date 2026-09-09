/* --------------------------------------------------------------------------
 * Dependency analysis of value bindings and expressions.
 *
 * Walks every expression of a binding group with a stack of the bindings
 * in scope, resolving each variable to the binding it refers to and
 * noting which bindings depend on which, and then sorts the group into
 * strongly connected components for the type checker.  The stacks are
 * also what lookupSyntax consults for the fixity of a local operator.
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
#include "goal.h"
#include "scc.h"
#include "fixity.h"
#include "typeexp.h"
#include "tycon.h"
#include "pattern.h"
#include "binding.h"
#include "depend.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static Cell local bindPat(Int, Cell);
static Void local bindPats(Int, List);
static List local dependencyAnal(List);
static Void local addDepField(Cell);
static Void local remDepField(List);
static Void local remDepField1(Cell);
static Void local saveSyntax(Cell, Cell);
static Void local dropNameClash(Cell);
static Void local depBinding(Cell);
static Void local depClassBindings(List);
static Void local depAlt(Cell);
static Cell local depLetRec(Bool, Int, Cell);
static Void local depRhs(Cell);
static Void local depGuard(Cell);
static Void local depPair(Int, Cell);
static Void local depTriple(Int, Cell);
static Void local depComp(Int, Cell, List);
static Void local depCaseAlt(Int, Cell);
static Cell local depVar(Int, Cell, Bool);
static Void local depUpdFlds(Int, Cell);
static List local depFields(Int, Cell, List, Bool);
static Void local checkNameAmbig(Int, Text, Cell);
static List local bscc(List);

/* --------------------------------------------------------------------------
 * Maintaining lists of bound variables and local definitions, for
 * dependency and scope analysis.
 * ------------------------------------------------------------------------*/

static List bounds;   /* list of lists of bound vars       */
static List bindings; /* list of lists of binds in scope  */
static List depends;  /* list of lists of dependents       */

/* bounds   :: [[Var]]        -- var equality used on Vars     */
/* bindings :: [[([Var],?)]]  -- var equality used on Vars     */
/* depends  :: [[Var]]        -- pointer equality used on Vars */

#define saveBvars() hd(bounds)           /* list of bvars in current scope   */
#define restoreBvars(bs) hd(bounds) = bs /* restore list of bound variables  */

static Cell local bindPat(Int line,
                          Cell p) /* add new bound vars for pattern   */
{
    patVars = NIL;
    p = checkPat(line, p);
    hd(bounds) = dupOnto(patVars, hd(bounds));
    return p;
}

static Void local bindPats(Int line,
                           List ps) /* add new bound vars for patterns  */
{
    patVars = NIL;
    map1Over(checkPat, line, ps);
    hd(bounds) = revOnto(patVars, hd(bounds));
}

/* lookupSyntax walks the scope stacks that the dependency analysis below
 * maintains, so it lives here rather than with the infix resolution in
 * fixity.c, which is its only caller. */
Syntax lookupSyntax(Text t) /* Try to find fixity for var in  */
{                           /* enclosing bindings            */
    List bounds1 = bounds;
    List bindings1 = bindings;

    while (nonNull(bindings1)) {
        if (nonNull(varIsMember(t, hd(bounds1)))) {
            return DEF_OPSYNTAX;
        } else {
            Cell b = findBinding(t, hd(bindings1));
            if (nonNull(b)) {
                Cell a = fst(snd(b));
                if (isVar(fst(b))) { /* Function binding                   */
                    if (nonNull(a) && nonNull(snd(a))) {
                        return intOf(snd(a));
                    }
                } else { /* Pattern binding                */
                    List vs = fst(b);
                    while (nonNull(vs) && nonNull(a)) {
                        if (t == textOf(hd(vs))) {
                            if (nonNull(hd(a)) && isInt(snd(hd(a)))) {
                                return intOf(snd(hd(a)));
                            }
                            break;
                        }
                        vs = tl(vs);
                        a = tl(a);
                    }
                }
                return DEF_OPSYNTAX;
            }
        }
        bounds1 = tl(bounds1);
        bindings1 = tl(bindings1);
    }
    return NO_SYNTAX;
}

/* --------------------------------------------------------------------------
 * To facilitate dependency analysis, lists of bindings are temporarily
 * augmented with an additional field, which is used in two ways:
 * - to build the `adjacency lists' for the dependency graph. Represented by
 *   a list of pointers to other bindings in the same list of bindings.
 * - to hold strictly positive integer values (depth first search numbers) of
 *   elements `on the stack' during the strongly connected components search
 *   algorithm, or a special value mkInt(0), once the binding has been added
 *   to a particular strongly connected component.
 *
 * Using this extra field, the type of each list of declarations during
 * dependency analysis is [Binding'] where:
 *
 *    Binding' ::= (Var, (Attr, (Dep, [Alt])))         -- function binding
 *              |  ([Var], ([Attr], (Dep, (Pat,Rhs)))) -- pattern binding
 *
 * ------------------------------------------------------------------------*/

#define depVal(d) (fst(snd(snd(d)))) /* Access to dependency information*/

static List local dependencyAnal(List bs) /* Separate lists of bindings into */
{ /* mutually recursive groups in          */
    /* order of dependency                   */

    mapProc(addDepField, bs); /* add extra field for dependents  */
    mapProc(depBinding, bs);  /* find dependents of each binding */
    bs = bscc(bs);            /* sort to strongly connected comps*/
    mapProc(remDepField, bs); /* remove dependency info field      */
    return bs;
}

List topDependAnal(List bs) /* Like dependencyAnal(), but at   */
{                           /* top level, reporting on progress*/
    List xs;
    Int i = 0;

    setGoal("Dependency analysis", (Target)(length(bs)));
    mapProc(addDepField, bs); /* add extra field for dependents   */
    for (xs = bs; nonNull(xs); xs = tl(xs)) {
        emptySubstitution();
        depBinding(hd(xs));
        soFar((Target)(i++));
    }
    bs = bscc(bs);            /* sort to strongly connected comps */
    mapProc(remDepField, bs); /* remove dependency info field      */
    done();
    return bs;
}

static Void local addDepField(Cell b) /* add extra field to binding to    */
{                                     /* hold list of dependents           */
    snd(snd(b)) = pair(NIL, snd(snd(b)));
}

static Void local remDepField(List bs) /* remove dependency field from     */
{                                      /* list of bindings                 */
    mapProc(remDepField1, bs);
}

static Void local remDepField1(Cell b) /* remove dependency field from     */
{                                      /* single binding                   */
    snd(snd(b)) = snd(snd(snd(b)));
}

Void clearScope() { /* initialise dependency scoping    */
    bounds = NIL;
    bindings = NIL;
    depends = NIL;
}

Void withinScope(List bs) /* Enter scope of bindings bs       */
{
    bounds = cons(NIL, bounds);
    bindings = cons(bs, bindings);
    depends = cons(NIL, depends);
}

Void leaveScope() {         /* Leave scope of last withinScope */
    List bs = hd(bindings); /* Remove fixity info from binds   */
    Bool toplevel = isNull(tl(bindings));
    for (; nonNull(bs); bs = tl(bs)) {
        Cell b = hd(bs);
        if (isVar(fst(b))) { /* Variable binding                   */
            Cell a = fst(snd(b));
            dropNameClash(fst(b));
            if (isPair(a)) {
                if (toplevel) {
                    saveSyntax(fst(b), snd(a));
                }
                fst(snd(b)) = fst(a);
            }
        } else { /* Pattern binding                */
            List vs = fst(b);
            List as = fst(snd(b));
            while (nonNull(vs) && nonNull(as)) {
                if (isPair(hd(as))) {
                    dropNameClash(hd(vs));
                    if (toplevel) {
                        saveSyntax(hd(vs), snd(hd(as)));
                    }
                    hd(as) = fst(hd(as));
                }
                vs = tl(vs);
                as = tl(as);
            }
        }
    }
    bounds = tl(bounds);
    bindings = tl(bindings);
    depends = tl(depends);
}

static Void local dropNameClash(Cell v) {
    Name n = findName(textOf(v));
    if (!isNull(n) && nonNull(nameRec(n)->clashes)) {
        nameRec(n)->clashes = tl(nameRec(n)->clashes);
    }
}

static Void local saveSyntax(Cell v,
                             Cell sy) /* Save syntax of top-level var      */
                                      /* in corresponding Name     */
{
    Name n = findName(textOf(v));
    if (isNull(n) || nameRec(n)->syntax != NO_SYNTAX) {
        internal("saveSyntax");
    }
    if (nonNull(sy)) {
        nameRec(n)->syntax = intOf(sy);
    }
}

/* --------------------------------------------------------------------------
 * As a side effect of the dependency analysis we also make the following
 * checks:
 * - Each lhs is a valid pattern/function lhs, all constructor functions
 *   have been defined and are used with the correct number of arguments.
 * - No lhs contains repeated pattern variables.
 * - Expressions used on the rhs of an eqn should be well formed.  This
 *   includes:
 *   - Checking for valid patterns (including repeated vars) in lambda,
 *     case, and list comprehension expressions.
 *   - Recursively checking local lists of equations.
 * - No free (i.e. unbound) variables are used in the declaration list.
 * ------------------------------------------------------------------------*/

static Void local depBinding(Cell b) /* find dependents of binding         */
{
    Cell defpart = snd(snd(snd(b))); /* definition part of binding         */

    hd(depends) = NIL;

    if (isVar(fst(b))) { /* function-binding?              */
        mapProc(depAlt, defpart);
        if (isNull(fst(snd(b)))) { /* Save dep info if no type sig         */
            fst(snd(b)) = pair(ap(IMPDEPS, hd(depends)), NIL);
        } else if (isNull(fst(fst(snd(b))))) {
            fst(fst(snd(b))) = ap(IMPDEPS, hd(depends));
        }
    } else { /* pattern-binding?                   */
        Int line = rhsLine(snd(defpart));
        enterBtyvs();
        patVars = NIL;
        fst(defpart) = checkPat(line, fst(defpart));
        depRhs(snd(defpart));
        fst(defpart) = applyBtyvs(fst(defpart));
    }
    depVal(b) = hd(depends);
}

Void depDefaults(Class c) /* dependency analysis on defaults  */
{                         /* from class definition            */
    depClassBindings(cclassRec(c)->defaults);
}

Void depInsts(Inst in) /* dependency analysis on instance  */
{                      /* bindings                         */
    depClassBindings(instRec(in)->implements);
}

static Void local
depClassBindings(List bs)                 /* dependency analysis on list of   */
{                                         /* bindings, possibly containing    */
    for (; nonNull(bs); bs = tl(bs)) {    /* NIL bindings ...                 */
        if (nonNull(hd(bs))) {            /* No need to add extra field for   */
            mapProc(depAlt, snd(hd(bs))); /* dependency information...     */
        }
    }
}

static Cell local depLetRec(Bool isRhs, Int line,
                            Cell e) /* dependency analysis on a letrec    */
{                                   /* expr, containing a set of bindings */

    fst(snd(e)) = eqnsToBindings(fst(snd(e)), NIL, NIL, NIL);
    withinScope(fst(snd(e)));
    fst(snd(e)) = dependencyAnal(fst(snd(e)));
    hd(depends) = fst(snd(e));
    if (isRhs) {
        depRhs(snd(snd(e)));
    } else {
        snd(snd(e)) = depExpr(line, snd(snd(e)));
    }
    leaveScope();
    return e;
}

static Void local depAlt(Cell a) /* Find dependents of alternative  */
{
    List obvs = saveBvars(); /* Save list of bound variables       */
    enterBtyvs();
    bindPats(rhsLine(snd(a)), fst(a)); /* add new bound vars for patterns */
    depRhs(snd(a));                    /* find dependents of rhs           */
    fst(a) = applyBtyvs(fst(a));
    restoreBvars(obvs); /* restore original list of bvars  */
}

static Void local depRhs(Cell r) /* Find dependents of rhs         */
{
    switch (whatIs(r)) {
    case GUARDED:
        mapProc(depGuard, snd(r));
        break;

    case LETREC:
        r = depLetRec(TRUE, rhsLine(snd(snd(r))), r);
        break;

    case RSIGN:
        snd(snd(r)) = checkPatType(rhsLine(fst(snd(r))), "result",
                                   rhsExpr(fst(snd(r))), snd(snd(r)));
        depRhs(fst(snd(r)));
        break;

    default:
        snd(r) = depExpr(intOf(fst(r)), snd(r));
        break;
    }
}

static Void local depGuard(Cell g) /* find dependents of single guarded*/
{                                  /* expression                           */
    depPair(intOf(fst(g)), snd(g));
}

Cell depExpr(Int line, Cell e) /* find dependents of expression    */
{
    stackCheck();
    switch (whatIs(e)) {

    case VARIDCELL:
    case VAROPCELL:
        return depVar(line, e, TRUE);

    case CONIDCELL:
    case CONOPCELL:
        return conDefined(line, e, TRUE);

    case QUALIDENT:
        if (isQVar(e)) {
            return depQVar(line, e, TRUE);
        } else { /* QConOrConOp */
            return conDefined(line, e, TRUE);
        }

    case INFIX:
        return depExpr(line, tidyInfix(line, snd(e)));

    case AP:
        depPair(line, e);
        break;

    case ZERONUM:
    case POSNUM:
    case NEGNUM:
    case NAME:
    case TUPLE:
    case STRCELL:
    case CHARCELL:
    case DOUBLECELL:
    case INTCELL:
        break;

    case COND:
        depTriple(line, snd(e));
        break;

    case FINLIST:
        map1Over(depExpr, line, snd(e));
        break;

    case LETREC:
        e = depLetRec(FALSE, line, e);
        break;

    case LAMBDA:
        depAlt(snd(e));
        break;

    case DOCOMP: /* fall-thru */
    case COMP:
        depComp(line, snd(e), snd(snd(e)));
        break;

    case ESIGN:
        fst(snd(e)) = depExpr(line, fst(snd(e)));
        snd(snd(e)) =
            checkSigType(line, "expression", fst(snd(e)), snd(snd(e)));
        break;

    case CASE:
        fst(snd(e)) = depExpr(line, fst(snd(e)));
        map1Proc(depCaseAlt, line, snd(snd(e)));
        break;

    case CONFLDS:
        depConFlds(line, e, FALSE);
        break;

    case UPDFLDS:
        depUpdFlds(line, e);
        break;

    case ASPAT:
        errHead(line);
        errPrintf("Illegal `@' in expression");
        errFail();

    case LAZYPAT:
        errHead(line);
        errPrintf("Illegal `~' in expression");
        errFail();

    case WILDCARD:
        errHead(line);
        errPrintf("Illegal `_' in expression");
        errFail();

    default:
        internal("depExpr");
    }
    return e;
}

static Void local depPair(Int line,
                          Cell e) /* find dependents of pair of exprs*/
{
    fst(e) = depExpr(line, fst(e));
    snd(e) = depExpr(line, snd(e));
}

static Void local depTriple(Int line,
                            Cell e) /* find dependents of triple exprs */
{
    fst3(e) = depExpr(line, fst3(e));
    snd3(e) = depExpr(line, snd3(e));
    thd3(e) = depExpr(line, thd3(e));
}

static Void local depComp(Int l, Cell e,
                          List qs) /* find dependents of comprehension*/
{
    if (isNull(qs)) {
        fst(e) = depExpr(l, fst(e));
    } else {
        Cell q = hd(qs);
        List qs1 = tl(qs);
        switch (whatIs(q)) {
        case FROMQUAL: {
            List obvs = saveBvars();
            snd(snd(q)) = depExpr(l, snd(snd(q)));
            enterBtyvs();
            fst(snd(q)) = bindPat(l, fst(snd(q)));
            depComp(l, e, qs1);
            fst(snd(q)) = applyBtyvs(fst(snd(q)));
            restoreBvars(obvs);
        } break;

        case QWHERE:
            snd(q) = eqnsToBindings(snd(q), NIL, NIL, NIL);
            withinScope(snd(q));
            snd(q) = dependencyAnal(snd(q));
            hd(depends) = snd(q);
            depComp(l, e, qs1);
            leaveScope();
            break;

        case DOQUAL: /* fall-thru */
        case BOOLQUAL:
            snd(q) = depExpr(l, snd(q));
            depComp(l, e, qs1);
            break;
        default:
            break;
        }
    }
}

static Void local depCaseAlt(Int line,
                             Cell a) /* Find dependents of case altern. */
{
    List obvs = saveBvars(); /* Save list of bound variables       */
    enterBtyvs();
    fst(a) = bindPat(line, fst(a)); /* Add new bound vars for pats         */
    depRhs(snd(a));                 /* Find dependents of rhs      */
    fst(a) = applyBtyvs(fst(a));
    restoreBvars(obvs); /* Restore original list of bvars  */
}

Void checkNameAmbigName(Int line, Cell n, Bool isV) {
    String kind = (isV ? "variable" : "data constructor");
    if (!isNull(n) && nonNull(nameRec(n)->clashes)) {
        Text t = nameRec(n)->text;
        List ls = nameRec(n)->clashes;
        errHead(line);
        errPrintf("Ambiguous %s occurrence \"%s\"", kind, textToStr(t));
        errPrintf("\n*** Could refer to: ");
        errPrintf("%s.%s ", textToStr(moduleRec(nameRec(n)->mod)->text),
                  textToStr(nameRec(n)->text));
        for (; nonNull(ls); ls = tl(ls)) {
            errPrintf("%s.%s ",
                      textToStr(moduleRec(nameRec(hd(ls))->mod)->text),
                      textToStr(nameRec(hd(ls))->text));
        }
        errPrintf("\n");
        errFail();
    }
}

static Void local checkNameAmbig(Int line, Text t, Cell e) {
    Name n;

    if (isName(e)) {
        n = e;
    } else {
        n = findName(t);
    }
    checkNameAmbigName(line, n, TRUE);
}

static Cell local depVar(Int line, Cell e,
                         Bool check) /* Register occurrence of variable */
{
    List bounds1 = bounds;
    List bindings1 = bindings;
    List depends1 = depends;
    Text t = textOf(e);
    Cell n;

    while (nonNull(bindings1)) {
        n = varIsMember(t, hd(bounds1)); /* look for t in bound variables */
        if (nonNull(n)) {
            return (n);
        }

        n = findBinding(t, hd(bindings1)); /* look for t in var bindings    */
        if (nonNull(n)) {
            if (!cellIsMember(n, hd(depends1))) {
                hd(depends1) = cons(n, hd(depends1));
            }

            if (check) {
                checkNameAmbig(line, t, (isVar(fst(n)) ? fst(n) : e));
            }
            return (isVar(fst(n)) ? fst(n) : e);
        }

        bounds1 = tl(bounds1);
        bindings1 = tl(bindings1);
        depends1 = tl(depends1);
    }

    if (isNull(n = findName(t))) { /* check global definitions */
        errHead(line);
        errPrintf("Undefined variable \"%s\"", textToStr(t));
        errFail();
    }
    /* Check whether there's no ambiguity about which global entity */
    if (check) {
        checkNameAmbig(line, t, e);
    }

    if (!moduleThisScript(nameRec(n)->mod)) {
        return n;
    }
    /* Later phases of the system cannot cope if we resolve references
     * to unprocessed objects too early.  This is the main reason that
     * we cannot cope with recursive modules at the moment.
     */
    return e;
}

Cell depQVar(Int line, Cell e,
             Bool isV) /* register occurrence of qualified variable */
{
    List ns = findQualNames(e);
    String kind = (isV ? "variable" : "data constructor");
    Name n;
    if (isNull(ns)) { /* check global definitions */
        errHead(line);
        errPrintf("Undefined qualified %s \"%s\"", kind, identToStr(e));
        errFail();
    }
    if (!isNull(tl(ns))) {
        List ls = ns;
        errHead(line);
        errPrintf("Ambiguous qualified %s occurrence \"%s\"", kind,
                  identToStr(e));
        errPrintf("\n*** Could refer to: ");
        for (; nonNull(ls); ls = tl(ls)) {
            errPrintf("%s.%s ",
                      textToStr(moduleRec(nameRec(hd(ls))->mod)->text),
                      textToStr(nameRec(hd(ls))->text));
        }
        errPrintf("\n");
        errFail();
    }

    n = hd(ns);
    if (nameRec(n)->mod != currentModule) {
        return n;
    }
    if (fst(e) == VARIDCELL) {
        e = mkVar(qtextOf(e));
    } else {
        e = mkVarop(qtextOf(e));
    }
    return depVar(line, e, FALSE);
}

Void depConFlds(Int line, Cell e,
                Bool isP) /* check construction using fields */
{
    Name c = conDefined(line, fst(snd(e)), TRUE);
    if (isNull(snd(snd(e))) ||
        nonNull(cellIsMember(c, depFields(line, e, snd(snd(e)), isP)))) {
        fst(snd(e)) = c;
    } else {
        errHead(line);
        errPrintf("Constructor \"%s\" does not have selected fields in ",
                  textToStr(nameRec(c)->text));
        errExpr(e);
        errPrintf("\n");
        errFail();
    }
    if (!isP &&
        isPair(nameRec(c)->defn)) {       /* Check that banged fields defined*/
        List scs = fst(nameRec(c)->defn); /* List of strict components       */
        Type t = nameRec(c)->type;
        Int a = userArity(c);
        List fs = snd(snd(e));
        List ss;
        if (isPolyType(t)) { /* Find tycon that c belongs to       */
            t = monotypeOf(t);
        }
        if (isQualType(t)) {
            t = snd(snd(t));
        }
        if (whatIs(t) == CDICTS) {
            t = snd(snd(t));
        }
        while (0 < a--) {
            t = arg(t);
        }
        while (isAp(t)) {
            t = fun(t);
        }
        for (ss = tyconRec(t)->defn; hasCfun(ss); ss = tl(ss)) {
        }
        /* Now we know the tycon t that c belongs to, and the corresponding
         * list of selectors for that type, ss.  Now we have to check that
         * each of the fields identified by scs appears in fs, using ss to
         * cross reference, and convert integers to selector names.
         */
        for (; nonNull(scs); scs = tl(scs)) {
            Int i = intOf(hd(scs));
            List ss1 = ss;
            for (; nonNull(ss1); ss1 = tl(ss1)) {
                List cns = nameRec(hd(ss1))->defn;
                for (; nonNull(cns); cns = tl(cns)) {
                    if (fst(hd(cns)) == c) {
                        break;
                    }
                }
                if (nonNull(cns) && intOf(snd(hd(cns))) == i) {
                    break;
                }
            }
            if (isNull(ss1)) {
                internal("depConFlds");
            } else {
                Name s = hd(ss1);
                List fs1 = fs;
                for (; nonNull(fs1) && s != fst(hd(fs1)); fs1 = tl(fs1)) {
                }
                if (isNull(fs1)) {
                    errHead(line);
                    errPrintf("Construction does not define strict field");

                    errPrintf("\nExpression : ");
                    errExpr(e);
                    errPrintf("\nField      : ");
                    errExpr(s);
                    errPrintf("\n");
                    errFail();
                }
            }
        }
    }
}

static Void local depUpdFlds(Int line,
                             Cell e) /* check update using fields          */
{
    if (isNull(thd3(snd(e)))) {
        errHead(line);
        errPrintf("Empty field list in update");
        errFail();
    }
    fst3(snd(e)) = depExpr(line, fst3(snd(e)));
    snd3(snd(e)) = depFields(line, e, thd3(snd(e)), FALSE);
}

static List local depFields(Int l, Cell e, List fs,
                            Bool isP) /* check field binding list          */
{
    List cs = NIL;
    List ss = NIL;

    for (; nonNull(fs); fs = tl(fs)) { /* for each field binding           */
        Cell fb = hd(fs);
        Name s;

        if (isVar(fb)) { /* expand  var  to  var = var     */
            fb = hd(fs) = pair(fb, fb);
        }

        s = findQualName(fst(fb)); /* check for selector                   */
        if (nonNull(s) && isSfun(s)) {
            fst(fb) = s;
        } else {
            if (isQualIdent(fst(fb))) {
                errHead(l);
                errPrintf("\"%s.%s\" is not a selector function/field name",
                          textToStr(qmodOf(fst(fb))),
                          textToStr(qtextOf(fst(fb))));
                errFail();
            } else {
                errHead(l);
                errPrintf("\"%s\" is not a selector function/field name",
                          textToStr(textOf(fst(fb))));
                errFail();
            }
        }

        if (isNull(ss)) { /* for first named selector           */
            List scs = nameRec(s)->defn; /* calculate list of constructors  */
            for (; nonNull(scs); scs = tl(scs)) {
                cs = cons(fst(hd(scs)), cs);
            }
            ss = singleton(s); /* initialize selector list         */
        } else {               /* for subsequent selectors         */
            List ds = cs;      /* intersect constructor lists      */
            for (cs = NIL; nonNull(ds);) {
                List scs = nameRec(s)->defn;
                while (nonNull(scs) && fst(hd(scs)) != hd(ds)) {
                    scs = tl(scs);
                }
                if (isNull(scs)) {
                    ds = tl(ds);
                } else {
                    List next = tl(ds);
                    tl(ds) = cs;
                    cs = ds;
                    ds = next;
                }
            }

            if (cellIsMember(s, ss)) { /* check for repeated uses          */
                errHead(l);
                errPrintf("Repeated field name \"%s\" in field list",
                          textToStr(nameRec(s)->text));
                errFail();
            }
            ss = cons(s, ss);
        }

        if (isNull(cs)) { /* Are there any matching constrs? */
            errHead(l);
            errPrintf("No constructor has all of the fields specified in ");
            errExpr(e);
            errPrintf("\n");
            errFail();
        }

        snd(fb) = (isP ? checkPat(l, snd(fb)) : depExpr(l, snd(fb)));
    }
    return cs;
}

/* --------------------------------------------------------------------------
 * Dependency ordering, using the strongly connected components sort in
 * scc.c.  Type constructors and classes keep their dependency list in the
 * kind field until kinds are inferred; bindings keep theirs in depVal.
 * ------------------------------------------------------------------------*/

static Cell local tcDepends(Cell c) {
    return isTycon(c) ? tyconRec(c)->kind : cclassRec(c)->kinds;
}

static Void local tcSetDepends(Cell c, Cell v) {
    if (isTycon(c))
        tyconRec(c)->kind = v;
    else
        cclassRec(c)->kinds = v;
}

List tcscc(List ts, List cs) /* scc for tycons and classes    */
{
    return sccSort(ts, cs, tcDepends, tcSetDepends);
}

static Cell local bDepends(Cell b) {
    return depVal(b);
}

static Void local bSetDepends(Cell b, Cell v) {
    depVal(b) = v;
}

static List local bscc(List bs) /* scc for bindings                */
{
    return sccSort(bs, NIL, bDepends, bSetDepends);
}

/* --------------------------------------------------------------------------
 * Control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const dependRoots[] = {
    &bounds,
    &bindings,
    &depends,
};

Void dependControl(Int what) {
    switch (what) {
    case RESET:
        bounds = NIL;
        bindings = NIL;
        depends = NIL;
        break;

    case INSTALL:
        registerRoots(dependRoots);
        dependControl(RESET);
        break;
    }
}

/*-------------------------------------------------------------------------*/
