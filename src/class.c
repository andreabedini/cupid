/* --------------------------------------------------------------------------
 * Static analysis of class, instance and default declarations.
 *
 * A class or instance is recorded as the parser meets it and checked
 * once the type constructors have their kinds: the superclasses and
 * functional dependencies are checked and inherited, member signatures
 * are checked, the member and dictionary selector names are made, and
 * each instance is checked against the class and inserted into it.
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
#include "typeexp.h"
#include "kind.h"
#include "binding.h"
#include "class.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static Void local checkClassDefn2(Class);
static Void local checkMems(Class, List, Cell);
static Void local checkMems2(Class, Cell);
static Name local newMember(Int, Int, Cell, Type, Class);
static Name local newDSel(Class, Int);
static Text local generateText(String, Class);
static Name local memberName(Class, Text);
static List local numInsert(Int, Cell, List);
static Bool local instCompare(Inst, Inst);
static List local inheritFundeps(Class, Cell, Int);

/* --------------------------------------------------------------------------
 * Static analysis of class declarations:
 *
 * Performed in a similar manner to that used for type declarations.
 *
 * The first part of the static analysis is performed as the declarations
 * are read during parsing.  The parser ensures that:
 * - the class header and all superclass predicates are of the form
 *   ``Class var''
 *
 * The classDefn() function:
 * - ensures that there is no previous definition for class
 * - checks that class name has not previously been used as a type constr.
 * - make new entry in class table
 * - record line number of declaration
 * - build list of classes defined in current script for use in later
 *   stages of static analysis.
 * ------------------------------------------------------------------------*/

Void classDefn(Int line, Cell head, List ms,
               List fds) /* process new class definition           */
                         /* definition line number         */
                         /* class header :: ([Supers],Class) */
                         /* class definition body                  */
{                        /* functional dependencies        */
    Text ct = textOf(getHead(snd(head)));
    Int arity = argCount;

    if (nonNull(findClass(ct))) {
        errHead(line);
        errPrintf("Multiple declarations of class \"%s\"", textToStr(ct));
        errFail();
    } else if (nonNull(findTycon(ct))) {
        errHead(line);
        errPrintf("\"%s\" used as both class and type constructor",
                  textToStr(ct));
        errFail();
    } else {
        Class nw = newClass(ct);
        cclassRec(nw)->line = line;
        cclassRec(nw)->arity = arity;
        cclassRec(nw)->head = snd(head);
        cclassRec(nw)->supers = fst(head);
        cclassRec(nw)->members = ms;
        cclassRec(nw)->level = 0;
        cclassRec(nw)->fds = fds;
        cclassRec(nw)->xfds = NIL;
        classDefns = cons(nw, classDefns);
    }
}

/* --------------------------------------------------------------------------
 * Further analysis of class declarations:
 *
 * Full static analysis of class definitions must be postponed until the
 * complete script has been read and all static analysis on type definitions
 * has been completed.
 *
 * Once this has been achieved, we carry out the following checks on each
 * class definition:
 * - check that variables in header are distinct
 * - replace head by skeleton
 * - check superclass declarations, replace by skeletons
 * - split body of class into members and declarations
 * - make new name entry for each member function
 * - record member function number (eventually an offset into dictionary!)
 * - no member function has a previous definition ...
 * - no member function is mentioned more than once in the list of members
 * - each member function type is valid, replace vars by offsets
 * - qualify each member function type by class header
 * - only bindings for members appear in defaults
 * - only function bindings appear in defaults
 * - check that extended class hierarchy does not contain any cycles
 * ------------------------------------------------------------------------*/

Void checkClassDefn(Class c) /* validate class definition       */
{
    List tyvars = NIL;
    Cell temp = cclassRec(c)->head;
    List fs = NIL;
    List ss = NIL;

    for (; isAp(temp); temp = fun(temp)) {
        if (!isVar(arg(temp))) {
            errHead(cclassRec(c)->line);
            errPrintf("Type variable required in class head");
            errFail();
        }
        if (nonNull(varIsMember(textOf(arg(temp)), tyvars))) {
            errHead(cclassRec(c)->line);
            errPrintf("Repeated type variable \"%s\" in class head",
                      textToStr(textOf(arg(temp))));
            errFail();
        }
        tyvars = cons(arg(temp), tyvars);
    }

    for (fs = cclassRec(c)->fds; nonNull(fs); fs = tl(fs)) {
        Pair fd = hd(fs);
        List vs = snd(fd);

        /* Check for trivial dependency
         */
        if (isNull(vs)) {
            errHead(cclassRec(c)->line);
            errPrintf("Functional dependency is trivial");
            errFail();
        }

        /* Check for duplicated vars on right hand side, and for vars on
         * right that also appear on the left:
         */
        for (vs = snd(fd); nonNull(vs); vs = tl(vs)) {
            if (varIsMember(textOf(hd(vs)), fst(fd))) {
                errHead(cclassRec(c)->line);
                errPrintf("Trivial dependency for variable \"%s\"",
                          textToStr(textOf(hd(vs))));
                errFail();
            }
            if (varIsMember(textOf(hd(vs)), tl(vs))) {
                errHead(cclassRec(c)->line);
                errPrintf("Repeated variable \"%s\" in functional dependency",
                          textToStr(textOf(hd(vs))));
                errFail();
            }
            hd(vs) = depTypeVar(cclassRec(c)->line, tyvars, textOf(hd(vs)));
        }

        /* Check for duplicated vars on left hand side:
         */
        for (vs = fst(fd); nonNull(vs); vs = tl(vs)) {
            if (varIsMember(textOf(hd(vs)), tl(vs))) {
                errHead(cclassRec(c)->line);
                errPrintf("Repeated variable \"%s\" in functional dependency",
                          textToStr(textOf(hd(vs))));
                errFail();
            }
            hd(vs) = depTypeVar(cclassRec(c)->line, tyvars, textOf(hd(vs)));
        }
    }

    /* add in the tyvars from the `supers' so that we don't
       prematurely complain about undefined tyvars */
    tyvars = typeVarsIn(cclassRec(c)->supers, NIL, NIL, tyvars);
    cclassRec(c)->tyvars = dupList(tyvars);

    if (cclassRec(c)->arity == 0) {
        cclassRec(c)->head = c;
    } else {
        Int args = cclassRec(c)->arity - 1;
        for (temp = cclassRec(c)->head; args > 0; temp = fun(temp), args--) {
            arg(temp) = mkOffset(args);
        }
        arg(temp) = mkOffset(0);
        fun(temp) = c;
    }

    tcDeps = NIL; /* find dependents               */
    map2Over(depPredExp, cclassRec(c)->line, tyvars, cclassRec(c)->supers);

    cclassRec(c)->numSupers = length(cclassRec(c)->supers);
    cclassRec(c)->defaults =
        extractBindings(cclassRec(c)->members); /* defaults*/
    ss = extractSigdecls(cclassRec(c)->members);
    fs = extractFixdecls(cclassRec(c)->members);
    cclassRec(c)->members = pair(ss, fs);
    map2Proc(checkMems, c, tyvars, ss);

    cclassRec(c)->kinds = tcDeps;
    tcDeps = NIL;
}

Void checkClassDefn2_(List cs) {
    mapProc(checkClassDefn2, cs);
}

static Void local checkClassDefn2(Class c) /* validate class definition, pt 2 */
{                                          /* can only finish this job after  */
                                           /* we've inherited fds             */
                                           /* and are in dependency order     */
    if (!isTycon(c)) {
        List tvts = offsetTyvarsIn(cclassRec(c)->head, NIL);
        List tvps = offsetTyvarsIn(cclassRec(c)->supers, NIL);
        List fds = calcFunDeps(cclassRec(c)->supers);
        tvts = oclose(fds, tvts);
        tvts = odiff(tvps, tvts);

        if (!isNull(tvts)) {
            errHead(cclassRec(c)->line);
            errPrintf("Undefined type variable \"%s\"",
                      textToStr(textOf(
                          nth(offsetOf(hd(tvts)), cclassRec(c)->tyvars))));
            errFail();
        }
    }
}

/* --------------------------------------------------------------------------
 * Functional dependencies are inherited from superclasses.
 * For example, if I've got the following classes:
 *
 * class C a b | a -> b
 * class C [b] a => D a b
 *
 * then C will have the dependency ([a], [b]) as expected, and D will inherit
 * the dependency ([b], [a]) from C.
 * When doing pairwise improvement, we have to consider not just improving
 * when we see a pair of Cs or a pair of Ds in the context, but when we've
 * got a C and a D as well.  In this case, we only improve when the
 * predicate in question matches the type skeleton in the relevant superclass
 * constraint.  E.g., we improve the pair (C [Int] a, D b Int) (unifying
 * a and b), but we don't improve the pair (C Int a, D b Int).
 * To implement functional dependency inheritance, we calculate
 * the closure of all functional dependencies, and store the result
 * in an additional field `xfds' (extended functional dependencies).
 * The `xfds' field is a list of functional dependency lists, annotated
 * with a list of predicate skeletons constraining when improvement can
 * happen against this dependency list.  For example, the xfds field
 * for C above would be:
 *     [([C a b], [([a], [b])])]
 * and the xfds field for D would be:
 *     [([C [b] a, D a b], [([b], [a])])]
 * Self-improvement (of a C with a C, or a D with a D) is treated as a
 * special case of an inherited dependency.
 * ------------------------------------------------------------------------*/
static List local inheritFundeps(Class c, Cell pi, Int o) {
    Int alpha = newKindedVars(cclassRec(c)->kinds);
    List scs = cclassRec(c)->supers;
    List xfds = NIL;
    Cell this = NIL;
    /* alloc additional vars for any vars in supers not in the head */
    newKindvars(length(cclassRec(c)->tyvars) - cclassRec(c)->arity);

    /* better not fail ;-) */
    if (!matchPred(pi, o, cclassRec(c)->head, alpha)) {
        /* If the qualified type is not valid, for instance by
         * having type variables occurring free in the context,
         * but not in the head -- we will end up here.
         *
         * Silently give up & assume that checkClassDefn2() will
         * catch the error condition.
         */
        return xfds;
    }
    this = copyPred(pi, o);
    for (; nonNull(scs); scs = tl(scs)) {
        Class s = getHead(hd(scs));
        if (isClass(s)) {
            List sfds = inheritFundeps(s, hd(scs), alpha);
            for (; nonNull(sfds); sfds = tl(sfds)) {
                Cell h = hd(sfds);
                xfds = cons(pair(cons(this, fst(h)), snd(h)), xfds);
            }
        }
    }
    if (nonNull(cclassRec(c)->fds)) {
        List fds = NIL, fs = cclassRec(c)->fds;
        for (; nonNull(fs); fs = tl(fs)) {
            fds =
                cons(pair(otvars(this, fst(hd(fs))), otvars(this, snd(hd(fs)))),
                     fds);
        }
        xfds = cons(pair(cons(this, NIL), fds), xfds);
    }
    return xfds;
}

Void extendFundeps(Class c) {
    Int alpha;
    emptySubstitution();
    alpha = newKindedVars(cclassRec(c)->kinds);
    cclassRec(c)->xfds = inheritFundeps(c, cclassRec(c)->head, alpha);

    /* we can now check for ambiguity */
    map1Proc(checkMems2, c, fst(cclassRec(c)->members));
}

static Void local checkMems(Class c, List tyvars,
                            Cell m) /* check member function details   */
{
    Int line = intOf(fst3(m));
    Type t = thd3(m);
    List sig = NIL;
    List tvs = NIL;
    List xtvs = NIL;

    if (isPolyType(t)) {
        xtvs = fst(snd(t));
        t = monotypeOf(t);
    }

    tyvars = typeVarsIn(t, NIL, xtvs, tyvars);
    /* Look for extra type vars.           */
    checkOptQuantVars(line, xtvs, tyvars);

    if (isQualType(t)) { /* Overloaded member signatures?   */
        map2Over(depPredExp, line, tyvars, fst(snd(t)));
    } else {
        t = ap(QUAL, pair(NIL, t));
    }

    fst(snd(t)) =
        cons(cclassRec(c)->head, fst(snd(t))); /* Add main predicate   */
    snd(snd(t)) = depTopType(line, tyvars, snd(snd(t)));

    for (tvs = tyvars; nonNull(tvs);
         tvs = tl(tvs)) { /* Quantify              */
        sig = ap(NIL, sig);
    }
    if (nonNull(sig)) {
        t = mkPolyType(sig, t);
    }
    thd3(m) = t;                       /* Save type            */
    take(cclassRec(c)->arity, tyvars); /* Delete extra type vars  */
}

static Void local checkMems2(Class c,
                             Cell m) /* check member function details   */
{
    Int line = intOf(fst3(m));
    List vs = snd3(m);
    Type t = thd3(m);

    if (isAmbiguous(t)) {
        ambigError(line, "class declaration", hd(vs), t);
    }
}

Void addMembers(Class c) /* Add definitions of member funs  */
{                        /* and other parts of class struct.*/
    List ms = fst(cclassRec(c)->members);
    List fs = snd(cclassRec(c)->members);
    List ns = NIL; /* List of names                */
    Int mno;       /* Member function number       */

    for (mno = 0; mno < cclassRec(c)->numSupers; mno++) {
        ns = cons(newDSel(c, mno), ns);
    }
    cclassRec(c)->dsels = rev(ns); /* Save dictionary selectors        */

    for (mno = 1, ns = NIL; nonNull(ms); ms = tl(ms)) {
        Int line = intOf(fst3(hd(ms)));
        List vs = rev(snd3(hd(ms)));
        Type t = thd3(hd(ms));
        for (; nonNull(vs); vs = tl(vs)) {
            ns = cons(newMember(line, mno++, hd(vs), t, c), ns);
        }
    }
    cclassRec(c)->members =
        rev(ns); /* Save list of members                   */
    cclassRec(c)->numMembers = length(cclassRec(c)->members);

    for (; nonNull(fs); fs = tl(fs)) { /* fixity declarations              */
        Int line = intOf(fst3(hd(fs)));
        List ops = snd3(hd(fs));
        Syntax s = intOf(thd3(hd(fs)));
        for (; nonNull(ops); ops = tl(ops)) {
            Name n = nameIsMember(textOf(hd(ops)), cclassRec(c)->members);
            if (isNull(n)) {
                missFixity(line, textOf(hd(ops)));
            } else if (nameRec(n)->syntax != NO_SYNTAX) {
                dupFixity(line, textOf(hd(ops)));
            }
            nameRec(n)->syntax = s;
        }
    }

    /*  Not actually needed just yet; for the time being, dictionary code will
    not be passed through the type checker.

    cclassRec(c)->dtycon   = addPrimTycon(generateText("Dict.%s",c),
                                      NIL,
                                      cclassRec(c)->arity,
                                      DATATYPE,
                                      NIL);
*/

    mno = cclassRec(c)->numSupers + cclassRec(c)->numMembers;
    cclassRec(c)->dcon = addPrimCfun(generateText("Make.%s", c), mno, 0, NIL);
    if (mno == 1) { /* Single entry dicts use newtype  */
        nameRec(cclassRec(c)->dcon)->defn = nameId;
        if (nonNull(cclassRec(c)->members)) {
            nameRec(hd(cclassRec(c)->members))->number = mfunNo(0);
        }
    }
    cclassRec(c)->defaults = classBindings("class", c, cclassRec(c)->defaults);
}

static Name local newMember(Int l, Int no, Cell v, Type t, Class parent)
/* Make definition for member fn   */
{
    Name m = findName(textOf(v));

    if (isNull(m)) {
        m = newName(textOf(v), parent);
    } else if (nameRec(m)->defn != PREDEFINED &&
               nameRec(m)->mod == currentModule) {
        errHead(l);
        errPrintf("Multiple declarations for member function \"%s\"",
                  textToStr(nameRec(m)->text));
        errFail();
    } else if (nameRec(m)->defn != PREDEFINED) {
        Name oldnm = m;
        removeName(m);
        m = newName(textOf(v), parent);
        nameRec(m)->defn = PREDEFINED;
        nameRec(m)->clashes = cons(oldnm, nameRec(m)->clashes);
    } else if (nameRec(m)->parent == NIL) {
        /* (try) improving the parent info. */
        nameRec(m)->parent = parent;
    }
    nameRec(m)->line = l;
    nameRec(m)->arity = 1;
    nameRec(m)->number = mfunNo(no);
    nameRec(m)->type = t;
    return m;
}

static Name local newDSel(Class c, Int no) /* Make definition for dict selectr*/
{
    Name s;
    char buf[16];

    sprintf(buf, "_sc%d_%s", no, "%s");
    s = newName(generateText(buf, c), c);
    nameRec(s)->line = cclassRec(c)->line;
    nameRec(s)->arity = 1;
    nameRec(s)->number = DFUNNAME;
    return s;
}

#define MAX_GEN 128

static Text local generateText(String sk,
                               Class c) /* We need to generate names for   */
                                        /* certain objects corresponding   */
{                                       /* to each class.                  */
    String cname = textToStr(cclassRec(c)->text);
    char buffer[MAX_GEN + 1];

    if ((strlen(sk) + strlen(cname)) >= MAX_GEN) {
        errHead(0);
        errPrintf("Please use a shorter name for class \"%s\"", cname);
        errFail();
    }
    sprintf(buffer, sk, cname);
    return findText(buffer);
}

Int visitClass(Class c)            /* visit class defn to check that  */
{                                  /* class hierarchy is acyclic         */
    if (cclassRec(c)->level < 0) { /* already visiting this class?       */
        errHead(cclassRec(c)->line);
        errPrintf("Superclass relation for \"%s\" is cyclic",
                  textToStr(cclassRec(c)->text));
        errFail();
    } else if (cclassRec(c)->level == 0) { /* visiting class for first time   */
        List scs = cclassRec(c)->supers;
        Int lev = 0;
        cclassRec(c)->level = (-1);
        for (; nonNull(scs); scs = tl(scs)) {
            Int l = visitClass(getHead(hd(scs)));
            if (l > lev)
                lev = l;
        }
        cclassRec(c)->level = 1 + lev; /* level = 1 + max level of supers */
    }
    return cclassRec(c)->level;
}

/* --------------------------------------------------------------------------
 * Process class and instance declaration binding groups:
 * ------------------------------------------------------------------------*/

List classBindings(String where, Class c, List bs)
/* Check validity of bindings bs   */
/* for class c (or an inst of c)   */
{ /* sort into approp. member order  */
    List nbs = NIL;
    Text nm;

    for (; nonNull(bs); bs = tl(bs)) {
        Cell b = hd(bs);
        Cell body = snd(snd(b));
        Name mnm;

        if (!(isVar(fst(b)))) { /* Only allow function bindings    */
            errHead(rhsLine(snd(body)));
            errPrintf("Pattern binding illegal in %s declaration", where);
            errFail();
        }

        nm = textOf(fst(b));

        if (isNull(mnm = memberName(c, nm))) {
            errHead(rhsLine(snd(hd(body))));
            errPrintf("No member \"%s\" in class \"%s\"", textToStr(nm),
                      textToStr(cclassRec(c)->text));
            errFail();
        }
        snd(b) = body;
        nbs = numInsert(mfunOf(mnm) - 1, b, nbs);
    }
    return nbs;
}

static Name local memberName(Class c,
                             Text t) /* return name of member function  */
                                     /* with name t in class c     */
{                                    /* return NIL if not a member         */
    List ms = cclassRec(c)->members;
    for (; nonNull(ms); ms = tl(ms)) {
        if (t == nameRec(hd(ms))->text) {
            return hd(ms);
        }
    }
    return NIL;
}

static List local numInsert(Int n, Cell x,
                            List xs) /* insert x at nth position in xs,  */
                                     /* filling gaps with NIL            */
{
    List start = isNull(xs) ? cons(NIL, NIL) : xs;

    for (xs = start; 0 < n--; xs = tl(xs)) {
        if (isNull(tl(xs))) {
            tl(xs) = cons(NIL, NIL);
        }
    }
    hd(xs) = x;
    return start;
}

/* --------------------------------------------------------------------------
 * Static analysis of instance declarations:
 *
 * The first part of the static analysis is performed as the declarations
 * are read during parsing:
 * - make new entry in instance table
 * - record line number of declaration
 * - build list of instances defined in current script for use in later
 *   stages of static analysis.
 * ------------------------------------------------------------------------*/

Void instDefn(Int line, Cell head,
              List ms) /* process new instance definition  */
                       /* definition line number           */
                       /* inst header :: (context,Class)   */
{                      /* instance members                 */
    Inst nw = newInst();
    instRec(nw)->line = line;
    instRec(nw)->specifics = fst(head);
    instRec(nw)->head = snd(head);
    instRec(nw)->implements = ms;
    instDefns = cons(nw, instDefns);
}

/* --------------------------------------------------------------------------
 * Further static analysis of instance declarations:
 *
 * Makes the following checks:
 * - Class part of header has form C (T a1 ... an) where C is a known
 *   class, and T is a known datatype constructor (or restricted synonym),
 *   and there is no previous C-T instance, and (T a1 ... an) has a kind
 *   appropriate for the class C.
 * - Each element of context is a valid class expression, with type vars
 *   drawn from a1, ..., an.
 * - All bindings are function bindings
 * - All bindings define member functions for class C
 * - Arrange bindings into appropriate order for member list
 * - No top level type signature declarations
 * ------------------------------------------------------------------------*/

Bool allowOverlap = FALSE;       /* TRUE => allow overlapping insts */
Bool allowUnsafeOverlap = FALSE; /* TRUE => in addition, allow      */
                                 /* potentially inconsistent        */
                                 /* overlapping instances           */
Name nameListMonad = NIL;        /* builder function for List Monad */

Void checkInstDefn(Inst in) /* Validate instance declaration   */
{
    Int line = instRec(in)->line;
    List tyvars = typeVarsIn(instRec(in)->head, NIL, NIL, NIL);
    List tvps = NIL, tvts = NIL;
    List fds = NIL;

    /* add in the tyvars from the `specifics' so that we don't
       prematurely complain about undefined tyvars */
    tyvars = typeVarsIn(instRec(in)->specifics, NIL, NIL, tyvars);
    instRec(in)->head = depPredExp(line, tyvars, instRec(in)->head);

    map2Over(depPredExp, line, tyvars, instRec(in)->specifics);

    /* OK, now we start over, and test for ambiguity */
    tvts = offsetTyvarsIn(instRec(in)->head, NIL);
    tvps = offsetTyvarsIn(instRec(in)->specifics, NIL);
    fds = calcFunDeps(instRec(in)->specifics);
    tvts = oclose(fds, tvts);
    tvts = odiff(tvps, tvts);
    if (!isNull(tvts)) {
        errHead(line);
        errPrintf("Ambiguous type variable \"%s\"",
                  textToStr(textOf(nth(offsetOf(hd(tvts)), tyvars))));
        errFail();
    }

    instRec(in)->numSpecifics = length(instRec(in)->specifics);
    instRec(in)->c = getHead(instRec(in)->head);
    if (!isClass(instRec(in)->c)) {
        errHead(line);
        errPrintf("Illegal predicate in instance declaration");
        errFail();
    }

    /* should this be over xfds? */
    if (nonNull(cclassRec(instRec(in)->c)->fds)) {
        List fds = cclassRec(instRec(in)->c)->fds;
        for (; nonNull(fds); fds = tl(fds)) {
            List as = otvars(instRec(in)->head, fst(hd(fds)));
            List bs = otvars(instRec(in)->head, snd(hd(fds)));
            List fs = calcFunDeps(instRec(in)->specifics);
            as = oclose(fs, as);
            if (!osubset(bs, as)) {
                errHead(instRec(in)->line);
                errPrintf("Instance is more general than a dependency allows");

                errPrintf("\n*** Instance         : ");
                errPred(instRec(in)->head);
                errPrintf("\n*** For class        : ");
                errPred(cclassRec(instRec(in)->c)->head);
                errPrintf("\n*** Under dependency : ");
                errFD(hd(fds));
                errPrintf("\n");
                errFail();
            }
        }
    }

    kindInst(in, length(tyvars));
    insertInst(in);

    if (nonNull(extractSigdecls(instRec(in)->implements))) {
        errHead(line);
        errPrintf(
            "Type signature declarations not permitted in instance declaration");
        errFail();
    }
    if (nonNull(extractFixdecls(instRec(in)->implements))) {
        errHead(line);
        errPrintf("Fixity declarations not permitted in instance declaration");
        errFail();
    }
    instRec(in)->implements = classBindings(
        "instance", instRec(in)->c, extractBindings(instRec(in)->implements));
    instRec(in)->builder = newInstImp(in);
    if (!preludeLoaded && isNull(nameListMonad) && isAp(instRec(in)->head) &&
        fun(instRec(in)->head) == classMonad &&
        arg(instRec(in)->head) == typeList) {
        nameListMonad = instRec(in)->builder;
    }
}

Void insertInst(Inst in) /* Insert instance into class        */
{
    Class c = instRec(in)->c;
    List ins = cclassRec(c)->instances;
    List prev = NIL;

    if (nonNull(
            cclassRec(c)->fds)) { /* Check for conflicts with fds          */
        List ins1 = cclassRec(c)->instances;
        for (; nonNull(ins1); ins1 = tl(ins1)) {
            List fds = cclassRec(c)->fds;
            substitution(RESET);
            for (; nonNull(fds); fds = tl(fds)) {
                Int alpha = newKindedVars(instRec(in)->kinds);
                Int beta = newKindedVars(instRec(hd(ins1))->kinds);
                List as = fst(hd(fds));
                Bool same = TRUE;
                for (; same && nonNull(as); as = tl(as)) {
                    Int n = offsetOf(hd(as));
                    same &= unify(nthArg(n, instRec(in)->head), alpha,
                                  nthArg(n, instRec(hd(ins1))->head), beta);
                }
                if (isNull(as) && same) {
                    for (as = snd(hd(fds)); same && nonNull(as); as = tl(as)) {
                        Int n = offsetOf(hd(as));
                        same &=
                            sameType(nthArg(n, instRec(in)->head), alpha,
                                     nthArg(n, instRec(hd(ins1))->head), beta);
                    }
                    if (!same) {
                        errHead(instRec(in)->line);
                        errPrintf(
                            "Instances are not consistent with dependencies");

                        errPrintf("\n*** This instance    : ");
                        errPred(instRec(in)->head);
                        errPrintf("\n*** Conflicts with   : ");
                        errPred(instRec(hd(ins1))->head);
                        errPrintf("\n*** For class        : ");
                        errPred(cclassRec(c)->head);
                        errPrintf("\n*** Under dependency : ");
                        errFD(hd(fds));
                        errPrintf("\n");
                        errFail();
                    }
                }
            }
        }
    }

    substitution(RESET);
    while (nonNull(ins)) { /* Look for overlap w/ other insts */
        Int alpha = newKindedVars(instRec(in)->kinds);
        Int beta = newKindedVars(instRec(hd(ins))->kinds);
        if (unifyPred(instRec(in)->head, alpha, instRec(hd(ins))->head, beta)) {
            Cell pi = copyPred(instRec(in)->head, alpha);
            if (allowOverlap || allowUnsafeOverlap) {
                Bool bef = instCompare(in, hd(ins));
                Bool aft = instCompare(hd(ins), in);
                if (bef && !aft) { /* in comes strictly before hd(ins)*/
                    break;
                }
                if (aft && !bef) { /* in comes strictly after hd(ins) */
                    prev = ins;
                    ins = tl(ins);
                    continue;
                }
            }
            errHead(instRec(in)->line);
            errPrintf("Overlapping instances for class \"%s\"",
                      textToStr(cclassRec(c)->text));

            errPrintf("\n*** This instance   : ");
            errPred(instRec(in)->head);
            errPrintf("\n*** Overlaps with   : ");
            errPred(instRec(hd(ins))->head);
            errPrintf("\n*** Common instance : ");
            errPred(pi);
            errPrintf("\n");
            errFail();
        }
        prev = ins;    /* No overlap detected, so move on */
        ins = tl(ins); /* to next instance                 */
    }
    substitution(RESET);

    if (nonNull(prev)) { /* Insert instance at this point   */
        tl(prev) = cons(in, ins);
    } else {
        cclassRec(c)->instances = cons(in, ins);
    }
}

static Bool local instCompare(Inst ia,
                              Inst ib) /* See if ia is an instance of ib  */
{
    Int alpha = newKindedVars(instRec(ia)->kinds);
    Int beta = newKindedVars(instRec(ib)->kinds);
    return matchPred(instRec(ia)->head, alpha, instRec(ib)->head, beta);
}

Name newInstImp(Inst in) /* Make definition for inst builder*/
{
    Name b = newName(inventText(), in);
    nameRec(b)->line = instRec(in)->line;
    nameRec(b)->arity = instRec(in)->numSpecifics;
    nameRec(b)->number = DFUNNAME;
    return b;
}

/* --------------------------------------------------------------------------
 * Default definitions; only one default definition is permitted in a
 * given script file.  If no default is supplied, then a standard system
 * default will be used where necessary.
 * ------------------------------------------------------------------------*/

Void defaultDefn(Int line, List defs) /* Handle default types definition */
{
    if (defaultLine != 0) {
        errHead(line);
        errPrintf("Multiple default declarations are not permitted in");
        errPrintf(" a single script file.\n");
        errFail();
    }
    defaultDefns = defs;
    defaultLine = line;
}

Void checkDefaultDefns() { /* check that default types are    */
    List ds = NIL;         /* well-kinded instances of Num    */

    if (defaultLine != 0) {
        map2Over(depTypeExp, defaultLine, NIL, defaultDefns);
        emptySubstitution();
        unkindTypes = NIL;
        map2Proc(kindType, defaultLine, "default type", defaultDefns);
        fixKinds();
        emptySubstitution();
        mapOver(fullExpand, defaultDefns);
    } else {
        defaultDefns = stdDefaults;
    }

    if (isNull(classNum)) {
        classNum = findClass(findText("Num"));
    }

    for (ds = defaultDefns; nonNull(ds); ds = tl(ds)) {
        if (isNull(provePred(NIL, NIL, ap(classNum, hd(ds))))) {
            errHead(defaultLine);
            errPrintf("Default types must be instances of the Num class");
            errFail();
        }
    }
}

/*-------------------------------------------------------------------------*/
