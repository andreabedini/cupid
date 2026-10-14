/* --------------------------------------------------------------------------
 * Static analysis of data, type and newtype declarations.
 *
 * A declaration is recorded as the parser meets it and checked in full
 * once the whole script has been read: constructor types are resolved and
 * given their tyvar offsets, field selectors are made, synonyms are checked
 * for cycles, and deriving requests are handed to deriving.c.  The
 * expansion of synonyms and newtypes in a type also lives here.
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
#include "static.h"
#include "kind.h"
#include "deriving.h"
#include "typeexp.h"
#include "binding.h"
#include "tycon.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static void depConstrs(Tycon, List, Cell);
static void noEqualities(int, List, Tycon);
static List addSels(int, Name, List, List);
static List selectCtxt(List, List);
static List visitSyn(List, Tycon, List);
static Type instantiateNewtype(Name, Type);
static Type instantiateSyn(Type, Type);

/* --------------------------------------------------------------------------
 * Static analysis of type declarations:
 *
 * Type declarations come in two forms:
 * - data declarations - define new constructed data types
 * - type declarations - define new type synonyms
 *
 * A certain amount of work is carried out as the declarations are
 * read during parsing.  In particular, for each type constructor
 * definition encountered:
 * - check that there is no previous definition of constructor
 * - ensure type constructor not previously used as a class name
 * - make a new entry in the type constructor table
 * - record line number of declaration
 * - Build separate lists of newly defined constructors for later use.
 * ------------------------------------------------------------------------*/

/* Process new type definition definition line number left hand side of
 * definition right hand side of definition SYNONYM/DATATYPE/etc... */
void tyconDefn(int line, Cell lhs, Cell rhs, Cell what) {
    Text t = textOf(getHead(lhs));
    Tycon tc = findTycon(t);

    if (nonNull(tc)) {
        errHead(line);
        errPrintf("Multiple declarations of type constructor \"%s\"",
                  textToStr(t));
        errFail();
    } else if (nonNull(tc) && nonNull(tyconRec(tc)->clashes)) {
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
    } else if (nonNull(findClass(t))) {
        errHead(line);
        errPrintf("\"%s\" used as both class and type constructor",
                  textToStr(t));
        errFail();
    } else {
        Tycon nw = newTycon(t);
        tyconDefns = cons(nw, tyconDefns);
        tyconRec(nw)->line = line;
        tyconRec(nw)->arity = argCount;
        tyconRec(nw)->what = what;
        if (what == RESTRICTSYN) {
            typeInDefns = cons(pair(nw, snd(rhs)), typeInDefns);
            rhs = fst(rhs);
        }
        tyconRec(nw)->defn = pair(lhs, rhs);
    }
}

/* Set local synonyms for given binding group */
void setTypeIns(List bs) {
    List cvs = typeInDefns;
    for (; nonNull(cvs); cvs = tl(cvs)) {
        Tycon c = fst(hd(cvs));
        List vs = snd(hd(cvs));
        for (tyconRec(c)->what = RESTRICTSYN; nonNull(vs); vs = tl(vs)) {
            if (nonNull(findBinding(textOf(hd(vs)), bs))) {
                tyconRec(c)->what = SYNONYM;
                break;
            }
        }
    }
}

/* Clear list of local synonyms */
void clearTypeIns(void) {
    for (; nonNull(typeInDefns); typeInDefns = tl(typeInDefns))
        tyconRec(fst(hd(typeInDefns)))->what = RESTRICTSYN;
}

/* --------------------------------------------------------------------------
 * Further analysis of Type declarations:
 *
 * In order to allow the definition of mutually recursive families of
 * data types, the static analysis of the right hand sides of type
 * declarations cannot be performed until all of the type declarations
 * have been read.
 *
 * Once parsing is complete, we carry out the following:
 *
 * - check format of lhs, extracting list of bound vars and ensuring that
 *   there are no repeated variables and no Skolem variables.
 * - run dependency analysis on rhs to check that only bound type vars
 *   appear in type and that all constructors are defined.
 *   Replace type variables by offsets, constructors by Tycons.
 * - use list of dependents to sort into strongly connected components.
 * - ensure that there is not more than one synonym in each group.
 * - kind-check each group of type definitions.
 *
 * - check that there are no previous definitions for constructor
 *   functions in data type definitions.
 * - install synonym expansions and constructor definitions.
 * ------------------------------------------------------------------------*/

/* Validate type constructor defn */
void checkTyconDefn(Tycon d) {
    Cell lhs = fst(tyconRec(d)->defn);
    Cell rhs = snd(tyconRec(d)->defn);
    int line = tyconRec(d)->line;
    List tyvars;
    List temp;

    stripBinderKinds(line, tyconRec(d)->text, lhs);
    tyvars = getArgs(lhs);
    /* check for repeated tyvars on lhs*/
    for (temp = tyvars; nonNull(temp); temp = tl(temp))
        if (nonNull(varIsMember(textOf(hd(temp)), tl(temp)))) {
            errHead(line);
            errPrintf("Repeated type variable \"%s\" on left hand side",
                      textToStr(textOf(hd(temp))));
            errFail();
        }

    tcDeps = NIL; /* find dependents               */
    switch (whatIs(tyconRec(d)->what)) {
    case RESTRICTSYN:
    case SYNONYM:
        rhs = depTopType(line, tyvars, rhs);
        if (cellIsMember(d, tcDeps)) {
            errHead(line);
            errPrintf("Recursive type synonym \"%s\"",
                      textToStr(tyconRec(d)->text));
            errFail();
        }
        break;

    case DATATYPE:
    case NEWTYPE:
        depConstrs(d, tyvars, rhs);
        rhs = fst(rhs);
        break;

    default:
        internal("checkTyconDefn");
        break;
    }

    tyconRec(d)->defn = rhs;
    tyconRec(d)->kind = tcDeps;
    tcDeps = NIL;
}

/* --------------------------------------------------------------------------
 * GADT syntax for ordinary data declarations:
 *
 *     data T a b where
 *         C :: forall c. Show c => a -> !c -> T a b
 *         D :: { f :: b } -> T a b
 *
 * is the declaration written one constructor signature at a time.  Each
 * signature becomes the constructor the equation form would have had, so
 * that nothing after the parser knows the difference: the result must be T
 * applied to distinct variables, which are renamed to the ones the head
 * binds; every other variable is existential and gets a fresh name; a
 * context becomes the constructor's local context.  A result that refines
 * T's arguments is a GADT proper and is not accepted.
 * ------------------------------------------------------------------------*/

/* The variable a head binds, with or without a kind */
static Cell headVar(Cell a) {
    return whatIs(a) == KINDED ? fst(snd(a)) : a;
}

/* A copy of t with its free variables renamed through map :: [(Var,Var)];
 * the variables a nested forall binds, listed in bound, are left alone */
static Type renameTyvars(Type t, List map, List bound) {
    switch (whatIs(t)) {
    case AP: {
        Type l = renameTyvars(fst(t), map, bound); /* ensure correct */
        Type r = renameTyvars(snd(t), map, bound); /* eval. order    */
        return ap(l, r);
    }
    case VARIDCELL:
    case VAROPCELL:
        if (!varIsMember(textOf(t), bound)) {
            List m = map;
            for (; nonNull(m); m = tl(m)) {
                if (textOf(fst(hd(m))) == textOf(t)) {
                    return snd(hd(m));
                }
            }
        }
        return t;
    case BANG:
        return ap(BANG, renameTyvars(snd(t), map, bound));
    case QUAL: {
        List ps = renameTyvars(fst(snd(t)), map, bound);
        Type b = renameTyvars(snd(snd(t)), map, bound);
        return ap(QUAL, pair(ps, b));
    }
    case POLYTYPE: {
        List inner = dupOnto(polySigOf(t), bound);
        Type b = renameTyvars(monotypeOf(t), map, inner);
        return ap(POLYTYPE, pair(polySigOf(t), b));
    }
    default:
        return t;
    }
}

/* The constructor c with signature sig, for the type tc of the given arity
 * whose head binds vs */
static Cell gadtConstr(int line, Text tc, int arity, List vs, Cell c,
                       Cell sig) {
    List ctxt = NIL;   /* the context, if any           */
    List fields = NIL; /* the fields of a record form   */
    List args = NIL;   /* the argument types otherwise  */
    List map = NIL;    /* [(Var,Var)] the renaming       */
    List evs = NIL;    /* the existential variables     */
    List rvs;          /* the result's variables        */
    List us, fs;
    Type r;
    Cell con;

    if (whatIs(sig) == POLYTYPE) { /* every variable is quantified anyway */
        sig = monotypeOf(sig);
    }
    if (isQualType(sig)) {
        ctxt = fst(snd(sig));
        sig = snd(snd(sig));
    }
    if (whatIs(sig) == LABC) {
        fields = fst(snd(sig));
        r = snd(snd(sig));
    } else {
        for (r = sig; isAp(r) && isAp(fun(r)) && fun(fun(r)) == typeArrow;
             r = arg(r)) {
            args = cons(arg(fun(r)), args);
        }
        args = rev(args);
    }

    /* The result must be the type being declared, applied to distinct
     * variables */
    if (whatIs(getHead(r)) != CONIDCELL || textOf(getHead(r)) != tc ||
        argCount != arity) {
        errHead(line);
        errPrintf("Constructor \"%s\" must build \"%s\" with %d argument%s, "
                  "the type being declared",
                  textToStr(textOf(c)), textToStr(tc), arity,
                  arity == 1 ? "" : "s");
        errFail();
    }
    rvs = getArgs(r);
    for (us = rvs; nonNull(us); us = tl(us)) {
        if (!isVar(hd(us)) || varIsMember(textOf(hd(us)), tl(us))) {
            errHead(line);
            errPrintf("Constructor \"%s\" refines the result type of \"%s\", "
                      "which is not supported: the result must be \"%s\" "
                      "applied to distinct type variables",
                      textToStr(textOf(c)), textToStr(tc), textToStr(tc));
            errFail();
        }
    }

    /* Rename the result's variables to the head's, and the rest, the
     * existential ones, to fresh names that cannot clash with those */
    for (us = rvs; nonNull(us); us = tl(us), vs = tl(vs)) {
        map = cons(pair(hd(us), headVar(hd(vs))), map);
    }
    us = typeVarsIn(ctxt, NIL, NIL, NIL);
    us = typeVarsIn(args, NIL, NIL, us);
    for (fs = fields; nonNull(fs); fs = tl(fs)) {
        us = typeVarsIn(snd(hd(fs)), NIL, NIL, us);
    }
    for (; nonNull(us); us = tl(us)) {
        if (!varIsMember(textOf(hd(us)), rvs)) {
            Cell v = inventVar();
            map = cons(pair(hd(us), v), map);
            evs = cons(v, evs);
        }
    }

    /* Build the constructor as the equation form would have */
    con = c;
    if (whatIs(sig) == LABC) {
        for (fs = NIL; nonNull(fields); fields = tl(fields)) {
            Type t = renameTyvars(snd(hd(fields)), map, NIL);
            fs = cons(pair(fst(hd(fields)), t), fs);
        }
        con = ap(LABC, pair(c, rev(fs)));
    } else {
        for (; nonNull(args); args = tl(args)) {
            con = ap(con, renameTyvars(hd(args), map, NIL));
        }
    }
    if (nonNull(ctxt)) {
        con = ap(QUAL, pair(renameTyvars(ctxt, map, NIL), con));
    }
    if (nonNull(evs)) {
        con = ap(POLYTYPE, pair(rev(evs), con));
    }
    return con;
}

/* The constructors of a declaration in GADT syntax, lhs being its head and
 * sigs the signatures as the parser read them, [(line, [Con], Type)] */
List gadtConstrs(Cell line, Cell lhs, List sigs, Cell what) {
    Text tc = textOf(getHead(lhs));
    int arity = argCount;
    List vs = getArgs(lhs);
    List cs = NIL;

    for (; nonNull(sigs); sigs = tl(sigs)) {
        Triple s = hd(sigs);
        List names = snd3(s);
        for (; nonNull(names); names = tl(names)) {
            cs = cons(
                gadtConstr(intOf(fst3(s)), tc, arity, vs, hd(names), thd3(s)),
                cs);
        }
    }
    if (what == NEWTYPE && (isNull(cs) || nonNull(tl(cs)))) {
        errHead(intOf(line));
        errPrintf("A newtype declaration must have exactly one constructor");
        errFail();
    }
    return rev(cs);
}

/* The head of data T :: k1 -> .. -> kn -> * where ..., which binds n fresh
 * variables of those kinds */
Cell kindedHeader(Cell line, Cell lhs, Kind k) {
    push(lhs);
    for (; isAp(k); k = snd(k)) {
        Cell v = inventVar();
        top() = ap(top(), ap(KINDED, pair(v, fst(k))));
    }
    if (k != STAR) {
        errHead(intOf(line));
        errPrintf("The kind of a data type ends in *, not Constraint");
        errFail();
    }
    return pop();
}

/* Define constructor functions and do dependency analysis for data definitions
 * (w or w/o deriving) */
/* An equality in the context of a data declaration or of one of its
 * constructors would refine the result type on a match, which makes a GADT
 * proper, and that waits for given equalities in the solver. */
static void noEqualities(int line, List ps, Tycon t) {
    for (; nonNull(ps); ps = tl(ps))
        if (isEquality(hd(ps))) {
            errHead(line);
            errPrintf("Equality constraint in a context of \"%s\", which is "
                      "not supported: a constructor cannot refine its result "
                      "type",
                      textToStr(tyconRec(t)->text));
            errFail();
        }
}

static void depConstrs(Tycon t, List tyvars, Cell cd) {
    int line = tyconRec(t)->line;
    List ctxt = NIL;
    int conNo = 1;
    Type lhs = t;
    List cs = fst(cd);
    List derivs = snd(cd);
    List compTypes = NIL;
    List sels = NIL;
    int i;

    for (i = 0; i < tyconRec(t)->arity;
         ++i)                       /* build representation for tycon  */
        lhs = ap(lhs, mkOffset(i)); /* applied to full comp. of args   */

    if (isQualType(cs)) { /* allow for possible context    */
        ctxt = fst(snd(cs));
        cs = snd(snd(cs));
        map2Over(depPredExp, line, tyvars, ctxt);
        noEqualities(line, ctxt, t);
    }

    if (nonNull(cs) && isNull(tl(cs))) /* Single constructor datatype?     */
        conNo = 0;

    for (; nonNull(cs); cs = tl(cs)) { /* For each constructor function:  */
        Cell con = hd(cs);
        List sig = dupList(tyvars);
        List evs = NIL;    /* locally quantified vars      */
        List lps = NIL;    /* locally bound predicates     */
        List ctxt1 = ctxt; /* constructor function context         */
        List scs = NIL;    /* strict components            */
        List fs = NONE;    /* selector names               */
        Type type = lhs;   /* constructor function type    */
        int arity = 0;     /* arity of constructor function   */
        int nr2 = 0;       /* Number of rank 2 args        */
        Name n;            /* name for constructor function   */

        if (whatIs(con) == POLYTYPE) { /* Locally quantified vars          */
            evs = fst(snd(con));
            con = snd(snd(con));
            sig = checkQuantVars(line, evs, sig, con);
        }

        if (isQualType(con)) { /* Local predicates                 */
            List us;
            lps = fst(snd(con));
            for (us = typeVarsIn(lps, NIL, NIL, NIL); nonNull(us); us = tl(us))
                if (!varIsMember(textOf(hd(us)), tyvars) &&
                    !varIsMember(textOf(hd(us)), evs)) {
                    errHead(line);
                    errPrintf(
                        "Variable \"%s\" in constraint is not locally bound",
                        textToStr(textOf(hd(us))));
                    errFail();
                }
            map2Over(depPredExp, line, sig, lps);
            noEqualities(line, lps, t);
            con = snd(snd(con));
            arity = length(lps);
        }

        if (whatIs(con) == LABC) {    /* Skeletize constr components       */
            Cell fls = snd(snd(con)); /* get field specifications          */
            con = fst(snd(con));
            fs = NIL;
            for (; nonNull(fls); fls = tl(fls)) { /* for each field spec:    */
                List vs = fst(hd(fls));
                Type t = snd(hd(fls)); /* - scrutinize type        */
                bool banged = whatIs(t) == BANG;
                t = depCompType(line, sig, (banged ? arg(t) : t));
                while (nonNull(vs)) { /* - add named components  */
                    Cell us = tl(vs);
                    tl(vs) = fs;
                    fs = vs;
                    vs = us;
                    con = ap(con, t);
                    arity++;
                    if (banged)
                        scs = cons(mkInt(arity), scs);
                }
            }
            fs = rev(fs);
            scs = rev(scs); /* put strict comps in ascend ord  */
        } else {            /* Non-labelled constructor    */
            Cell c = con;
            int compNo;
            for (; isAp(c); c = fun(c))
                arity++;
            for (compNo = arity, c = con; isAp(c); c = fun(c)) {
                Type t = arg(c);
                if (whatIs(t) == BANG) {
                    scs = cons(mkInt(compNo), scs);
                    t = arg(t);
                }
                compNo--;
                arg(c) = depCompType(line, sig, t);
            }
        }

        if (nonNull(ctxt1)) /* Extract relevant part of context*/
            ctxt1 = selectCtxt(ctxt1, offsetTyvarsIn(con, NIL));

        for (i = arity; isAp(con); i--) { /* Calculate type of constructor   */
            Type ty = fun(con);
            Type cmp = arg(con);
            fun(con) = typeArrow;
            if (isPolyOrQualType(cmp)) {
                if (nonNull(derivs)) {
                    errHead(line);
                    errPrintf("Cannot derive instances for types");
                    errPrintf(" with polymorphic or qualified components");
                    errFail();
                }
                if (nr2 == 0)
                    nr2 = i;
            }
            if (nonNull(derivs)) /* and build list of components           */
                compTypes = cons(cmp, compTypes);
            type = ap(con, type);
            con = ty;
        }

        if (nr2 > 0) { /* Add rank 2 annotation    */
            type = ap(RANK2, pair(mkInt(nr2 - length(lps)), type));
        }

        if (nonNull(evs)) { /* Add existential annotation          */
            if (nonNull(derivs)) {
                errHead(line);
                errPrintf("Cannot derive instances for types");
                errPrintf(" with existentially typed components");
                errFail();
            }
            if (fs != NONE) {
                errHead(line);
                errPrintf(
                    "Cannot use selectors with existentially typed components");
                errFail();
            }
            type = ap(EXIST, pair(mkInt(length(evs)), type));
        }

        if (nonNull(lps)) { /* Add local preds part to type        */
            type = ap(CDICTS, pair(lps, type));
        }

        if (nonNull(ctxt1)) { /* Add context part to type          */
            type = ap(QUAL, pair(ctxt1, type));
        }

        if (nonNull(sig)) { /* Add quantifiers to type     */
            List ts1 = sig;
            for (; nonNull(ts1); ts1 = tl(ts1)) {
                hd(ts1) = NIL;
            }
            type = mkPolyType(sig, type);
        }

        n = findName(textOf(con)); /* Allocate constructor fun name   */
        if (isNull(n)) {
            n = newName(textOf(con), NIL);
        } else if (nameRec(n)->defn != PREDEFINED &&
                   nameRec(n)->mod == currentModule) {
            /* A local repeated definition */
            duplicateError(line, nameRec(n)->mod, nameRec(n)->text,
                           "data constructor");
        } else if (nameRec(n)->defn != PREDEFINED) {
            Name oldnm = n;
            removeName(n);
            n = newName(textOf(con), NIL);
            nameRec(n)->defn = PREDEFINED;
            nameRec(n)->clashes = cons(oldnm, nameRec(n)->clashes);
        }

        nameRec(n)->arity = arity; /* Save constructor fun details     */
        nameRec(n)->line = line;
        nameRec(n)->parent = t;
        nameRec(n)->number = cfunNo(conNo++);
        nameRec(n)->type = type;
        if (tyconRec(t)->what == NEWTYPE) {
            if (nonNull(lps)) {
                errHead(line);
                errPrintf(
                    "A newtype constructor cannot have class constraints");
                errFail();
            }
            if (arity != 1) {
                errHead(line);
                errPrintf(
                    "A newtype constructor must have exactly one argument");
                errFail();
            }
            if (nonNull(scs)) {
                errHead(line);
                errPrintf(
                    "Illegal strictness annotation for newtype constructor");
                errFail();
            }
            nameRec(n)->defn = nameId;
        } else {
            implementCfun(n, scs);
        }

        hd(cs) = n;
        if (fs != NONE) {
            sels = addSels(line, n, fs, sels);
        }
    }

    if (nonNull(sels)) {
        sels = rev(sels);
        fst(cd) = appendOnto(fst(cd), sels);
        selDefns = cons(sels, selDefns);
    }

    if (nonNull(derivs)) { /* Generate derived instances           */
        List ds = derivs;
        List done = NIL; /* (as parsed, resolved): a via clause's classes */
        for (; nonNull(ds); ds = tl(ds)) { /* share one type             */
            if (whatIs(hd(ds)) == DERIVE && nonNull(snd(snd(hd(ds))))) {
                Cell v = snd(snd(hd(ds)));
                List xs = done;
                for (; nonNull(xs) && fst(hd(xs)) != v; xs = tl(xs)) {
                }
                if (isNull(xs)) {
                    done = cons(pair(v, depTypeExp(line, tyvars, v)), done);
                    xs = done;
                }
                snd(snd(hd(ds))) = snd(hd(xs));
            }
        }
        map3Proc(checkDerive, t, ctxt, compTypes, derivs);
    }
}

/* Find arity for cfun, ignoring CDICTS parameters */
int userArity(Name c) {
    int a = nameRec(c)->arity;
    Type t = nameRec(c)->type;
    int w;
    if (isPolyType(t)) {
        t = monotypeOf(t);
    }
    if ((w = whatIs(t)) == QUAL) {
        t = snd(snd(t));
        w = whatIs(t);
    }
    if (w == CDICTS) {
        a -= length(fst(snd(t)));
    }
    return a;
}

List cfunSfuns; /* List of (Cfun,[SelectorVar])     */
                /*  - used for deriving Show       */

/* Add fields to selector list line number of constructor corresponding constr
 * function list of fields (varids) list of existing selectors */
static List addSels(int line, Name c, List fs, List ss) {
    int sn = 1;
    cfunSfuns = cons(pair(c, fs), cfunSfuns);
    for (; nonNull(fs); fs = tl(fs), ++sn) {
        List ns = ss;
        Text t = textOf(hd(fs));

        if (nonNull(varIsMember(t, tl(fs)))) {
            errHead(line);
            errPrintf("Repeated field name \"%s\" for constructor \"%s\"",
                      textToStr(t), textToStr(nameRec(c)->text));
            errFail();
        }

        while (nonNull(ns) && t != nameRec(hd(ns))->text) {
            ns = tl(ns);
        }

        if (nonNull(ns)) {
            nameRec(hd(ns))->defn =
                cons(pair(c, mkInt(sn)), nameRec(hd(ns))->defn);
        } else {
            Name n;
            Name oldnm = findName(t);
            if (nonNull(oldnm)) {
                if (nameRec(oldnm)->mod == currentModule) {
                    errHead(line);
                    errPrintf("Multiple declarations for selector \"%s\"",
                              textToStr(t));
                    errFail();
                } else {
                    removeName(oldnm);
                }
            }
            n = newName(t, c);
            nameRec(n)->line = line;
            nameRec(n)->number = SELNAME;
            nameRec(n)->defn = singleton(pair(c, mkInt(sn)));
            if (nonNull(oldnm)) {
                nameRec(n)->clashes = cons(oldnm, nameRec(n)->clashes);
            }
            ss = cons(n, ss);
        }
    }
    return ss;
}

/* Calculate subset of context */
static List selectCtxt(List ctxt, List vs) {
    if (isNull(vs)) {
        return NIL;
    } else {
        List ps = NIL;
        for (; nonNull(ctxt); ctxt = tl(ctxt)) {
            List us = offsetTyvarsIn(hd(ctxt), NIL);
            for (; nonNull(us) && cellIsMember(hd(us), vs); us = tl(us)) {
            }
            if (isNull(us)) {
                ps = cons(hd(ctxt), ps);
            }
        }
        return rev(ps);
    }
}

/* Check for mutually recursive synonyms */
void checkSynonyms(List ts) {
    List syns = NIL;
    for (; nonNull(ts); ts = tl(ts)) { /* build list of all synonyms       */
        Tycon t = hd(ts);
        switch (whatIs(tyconRec(t)->what)) {
        case SYNONYM:
        case RESTRICTSYN:
            syns = cons(t, syns);
            break;
        default:
            break;
        }
    }
    while (nonNull(syns)) { /* then visit each synonym     */
        syns = visitSyn(NIL, hd(syns), syns);
    }
}

/* Visit synonym definition to look for cycles */
static List visitSyn(List path, Tycon t, List syns) {
    if (cellIsMember(t, path)) { /* every elt in path depends on t  */
        errHead(tyconRec(t)->line);
        errPrintf("Type synonyms \"%s\" and \"%s\" are mutually recursive",
                  textToStr(tyconRec(t)->text),
                  textToStr(tyconRec(hd(path))->text));
        errFail();
    } else {
        List ds = tyconRec(t)->kind;
        List path1 = NIL;
        for (; nonNull(ds); ds = tl(ds)) {
            if (cellIsMember(hd(ds), syns)) {
                if (isNull(path1)) {
                    path1 = cons(t, path);
                }
                syns = visitSyn(path1, hd(ds), syns);
            }
        }
    }
    tyconRec(t)->defn = fullExpand(tyconRec(t)->defn);
    return removeCell(t, syns);
}

/* --------------------------------------------------------------------------
 * Expanding out all type synonyms and newtypes in a type expression:
 * ------------------------------------------------------------------------*/

/* Find full expansion of type exp assuming that all relevant type synonym
 * defns of lower rank have already been fully expanded but not assuming same
 * for newtypes */
Type fullerExpand(Type t) {
    Cell h = t;
    int n = 0;
    List args;
    for (args = NIL; isAp(h); h = fun(h), n++) {
        /* Does not apply recursively because the ffi is only interested
         * in the top level constructors
         */
        args = cons(arg(h), args);
    }
    t = applyToArgs(h, args);
    if (isSynonym(h) && n >= tyconRec(h)->arity) {
        if (n == tyconRec(h)->arity) {
            t = instantiateSyn(tyconRec(h)->defn, t);
        } else {
            Type p = t;
            while (--n > tyconRec(h)->arity) {
                p = fun(p);
            }
            fun(p) = instantiateSyn(tyconRec(h)->defn, fun(p));
        }
        t = fullerExpand(t); /* chase synonym chains. */
    } else if (isNewtype(h) && n == tyconRec(h)->arity && h != typeIO) {
        if (n != 0) {
            /* Not supported because I don't understand the typechecker
             * well enough.  For those that grok the data structures, it
             * should be simple.
             */
            errHead(nameRec(h)->line);
            errPrintf("Use of polymorphic newtype '");
            errType(t);
            errPrintf("' not supported in foreign function declarations.");
            errFail();
        }
        t = instantiateNewtype(hd(tyconRec(h)->defn), t);
        t = fullerExpand(t); /* chase chains of newtypes */
    }
    return t;
}

/* Return true if FFI/primitive type sig is an IO action. */
bool hasIOResultType(Type ty) {
    Type t = ty;
    if (isPolyType(t)) {
        t = monotypeOf(t);
    }
    t = fullerExpand(t);
    while (getHead(t) == typeArrow && argCount == 2) {
        t = fullerExpand(arg(t));
    }
    return (getHead(t) == typeIO && argCount == 1);
}

/* Instantiate type using env to determine appropriate values for OFFSET type
 * vars */
static Type instantiateNewtype(Name c, Type env) {
    Type t = NIL;
    assert(isName(c));
    t = nameRec(c)->type;
    if (isPolyType(t)) {
        t = monotypeOf(t);
    }
    assert(getHead(t) == typeArrow && argCount == 2);
    t = arg(fun(t));
    /* This is probably where we should invoke instantiateSyn(t,env) */
    return t;
}

/* --------------------------------------------------------------------------
 * Expanding out all type synonyms in a type expression:
 * ------------------------------------------------------------------------*/

/* Find full expansion of type exp assuming that all relevant synonym defns of
 * lower rank have already been fully expanded */
Type fullExpand(Type t) {
    Cell h = t;
    int n = 0;
    List args;
    for (args = NIL; isAp(h); h = fun(h), n++) {
        args = cons(fullExpand(arg(h)), args);
    }
    t = applyToArgs(h, args);
    if (isSynonym(h) && n >= tyconRec(h)->arity) {
        if (n == tyconRec(h)->arity) {
            t = instantiateSyn(tyconRec(h)->defn, t);
        } else {
            Type p = t;
            while (--n > tyconRec(h)->arity) {
                p = fun(p);
            }
            fun(p) = instantiateSyn(tyconRec(h)->defn, fun(p));
        }
    }
    return t;
}

/* Instantiate type according using env to determine appropriate values for
 * OFFSET type vars */
static Type instantiateSyn(Type t, Type env) {
    switch (whatIs(t)) {
    case AP:
        return ap(instantiateSyn(fun(t), env), instantiateSyn(arg(t), env));

    case OFFSET:
        return nthArg(offsetOf(t), env);

    default:
        return t;
    }
}

/* --------------------------------------------------------------------------
 * Control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const tyconRoots[] = {
    &cfunSfuns,
};

/* Clear the list of constructors' selector variables. */
void resetTycon(void) {
    cfunSfuns = NIL;
}

/* Register the roots and reset; done once, before the Prelude is read. */
void installTycon(void) {
    registerRoots(tyconRoots);
    resetTycon();
}

/*-------------------------------------------------------------------------*/
