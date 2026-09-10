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
#include "deriving.h"
#include "typeexp.h"
#include "binding.h"
#include "tycon.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static Void local depConstrs(Tycon, List, Cell);
static List local addSels(Int, Name, List, List);
static List local selectCtxt(List, List);
static List local visitSyn(List, Tycon, List);
static Type local instantiateNewtype(Name, Type);
static Type local instantiateSyn(Type, Type);

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

Void tyconDefn(Int line, Cell lhs, Cell rhs,
               Cell what) /* process new type definition           */
                          /* definition line number        */
                          /* left hand side of definition          */
                          /* right hand side of definition   */
{                         /* SYNONYM/DATATYPE/etc...       */
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

Void setTypeIns(List bs) /* set local synonyms for given           */
{                        /* binding group                  */
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

Void clearTypeIns() { /* clear list of local synonyms      */
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

Void checkTyconDefn(Tycon d) /* validate type constructor defn  */
{
    Cell lhs = fst(tyconRec(d)->defn);
    Cell rhs = snd(tyconRec(d)->defn);
    Int line = tyconRec(d)->line;
    List tyvars = getArgs(lhs);
    List temp;
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

static Void local depConstrs(Tycon t, List tyvars, Cell cd)
/* Define constructor functions and*/
/* do dependency analysis for data */
{ /* definitions (w or w/o deriving) */
    Int line = tyconRec(t)->line;
    List ctxt = NIL;
    Int conNo = 1;
    Type lhs = t;
    List cs = fst(cd);
    List derivs = snd(cd);
    List compTypes = NIL;
    List sels = NIL;
    Int i;

    for (i = 0; i < tyconRec(t)->arity;
         ++i)                       /* build representation for tycon  */
        lhs = ap(lhs, mkOffset(i)); /* applied to full comp. of args   */

    if (isQualType(cs)) { /* allow for possible context    */
        ctxt = fst(snd(cs));
        cs = snd(snd(cs));
        map2Over(depPredExp, line, tyvars, ctxt);
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
        Int arity = 0;     /* arity of constructor function   */
        Int nr2 = 0;       /* Number of rank 2 args        */
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
                Bool banged = whatIs(t) == BANG;
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
            Int compNo;
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
        map3Proc(checkDerive, t, ctxt, compTypes, derivs);
    }
}

Int userArity(Name c) /* Find arity for cfun, ignoring   */
{                     /* CDICTS parameters            */
    Int a = nameRec(c)->arity;
    Type t = nameRec(c)->type;
    Int w;
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

static List local addSels(Int line, Name c, List fs,
                          List ss) /* Add fields to selector list          */
                                   /* line number of constructor           */
                                   /* corresponding constr function   */
                                   /* list of fields (varids)      */
{                                  /* list of existing selectors           */
    Int sn = 1;
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

static List local selectCtxt(List ctxt,
                             List vs) /* calculate subset of context       */
{
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

Void checkSynonyms(List ts) /* Check for mutually recursive           */
{                           /* synonyms                       */
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

static List local visitSyn(List path, Tycon t,
                           List syns) /* visit synonym definition to look*/
                                      /* for cycles                        */
{
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

Type fullerExpand(Type t) /* find full expansion of type exp */
{                         /* assuming that all relevant type */
    Cell h = t;           /* synonym defns of lower rank have*/
    Int n = 0;            /* already been fully expanded but */
    List args;            /* not assuming same for newtypes  */
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

Bool hasIOResultType(
    Type ty) /* return TRUE if FFI/primitive type sig is an IO action. */
{
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

static Type local instantiateNewtype(Name c,
                                     Type env) /* instantiate type using      */
/* env to determine appropriate    */
{ /* values for OFFSET type vars           */
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

Type fullExpand(Type t) /* find full expansion of type exp */
{                       /* assuming that all relevant      */
    Cell h = t;         /* synonym defns of lower rank have*/
    Int n = 0;          /* already been fully expanded     */
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

static Type local instantiateSyn(Type t,
                                 Type env) /* instantiate type according using*/
                                           /* env to determine appropriate    */
{ /* values for OFFSET type vars          */
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

Void tyconControl(Int what) {
    switch (what) {
    case RESET:
        cfunSfuns = NIL;
        break;

    case INSTALL:
        registerRoots(tyconRoots);
        tyconControl(RESET);
        break;
    default:
        break;
    }
}

/*-------------------------------------------------------------------------*/
