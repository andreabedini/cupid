/* --------------------------------------------------------------------------
 * Static analysis: the driver.
 *
 * The parser records declarations; checkDefns below runs the checks over a
 * whole script in the order they must happen, calling into tycon.c,
 * class.c, typeexp.c, kind.c, pattern.c, binding.c, depend.c, deriving.c
 * and ffi.c.  What stays here is the module import checks, the primitive
 * declarations, the entry points and the per-script reset.
 *
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "machdep.h"
#include "errors.h"
#include "subst.h"
#include "module.h"
#include "deriving.h"
#include "ffi.h"
#include "kind.h"
#include "tycon.h"
#include "typeexp.h"
#include "class.h"
#include "pattern.h"
#include "binding.h"
#include "depend.h"
#include "static.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static void checkQualImport(Pair);
static void checkUnqualImport(Pair);
static List checkPrimDefn(Triple);
static Name addNewPrim(int, Text, char *, Cell);
static void addRSsigdecls(Pair);
static void allNoPrevDef(Cell);
static void noPrevDef(int, Cell);
static void checkTypeIn(Pair);

/* --------------------------------------------------------------------------
 * Static analysis of modules:
 * ------------------------------------------------------------------------*/

/* Switch to a new module */
void startModule(Cell nm) {
    Module m;
    Text t = textOf(nm);
    if (!isCon(nm))
        internal("startModule");
    if (isNull(m = findModule(t))) {
        m = newModule(t);
        if (moduleUserPrelude == 0 && t == textUserPrelude) {
            moduleUserPrelude = m;
        }
    } else if (!isPreludeScript()) {
        /* You're allowed to break the rules in the Prelude! */
        errHead(0);
        errPrintf("Module \"%s\" already loaded", textToStr(t));
        errFail();
    }
    setCurrModule(m);
}

/* Add export list to current module */
void setExportList(List exps) {
    moduleRec(currentModule)->exports = exps;
}

/* Process qualified import */
static void checkQualImport(Pair i) {
    Module m = findModid(snd(i));
    if (isNull(m)) {
        errHead(0);
        errPrintf("Module \"%s\" not previously loaded",
                  textToStr(textOf(snd(i))));
        errFail();
    }
    snd(i) = m;
}

/* Process unqualified import */
static void checkUnqualImport(Pair i) {
    Module m = findModid(fst(i));
    if (isNull(m)) {
        errHead(0);
        errPrintf("Module \"%s\" not previously loaded",
                  textToStr(textOf(fst(i))));
        errFail();
    }
    fst(i) = m;
}

/* the bulk of the module system implementation now resides in module.c */

/* --------------------------------------------------------------------------
 * Primitive definitions are usually only included in the first script
 * file read - the prelude.  A primitive definition associates a variable
 * name with a string (which identifies a built-in primitive) and a type.
 * ------------------------------------------------------------------------*/

/* Handle primitive definitions */
void primDefn(Cell line, List prims, Cell type) {
    primDefns = cons(triple(line, prims, type), primDefns);
}

/* Check primitive definition */
static List checkPrimDefn(Triple pd) {
    int line = intOf(fst3(pd));
    List prims = snd3(pd);
    Type type = thd3(pd);
    emptySubstitution();
    type = checkSigType(line, "primitive declaration", fst(hd(prims)), type);
    for (; nonNull(prims); prims = tl(prims)) {
        Cell p = hd(prims);
        bool same = isVar(p);
        Text pt = textOf(same ? p : fst(p));
        char *pr = textToStr(textOf(same ? p : snd(p)));
        hd(prims) = addNewPrim(line, pt, pr, type);
    }
    return snd3(pd);
}

/* Make binding of variable vn to primitive function referred to by s, with
 * given type t */
static Name addNewPrim(int l, Text vn, char *s, Cell t) {
    Name n = findName(vn);

    if (isNull(n)) {
        n = newName(vn, NIL);
    } else if (nameRec(n)->defn != PREDEFINED) {
        duplicateError(l, nameRec(n)->mod, vn, "primitive");
    }

    addPrim(l, n, s, currentModule, t);
    return n;
}

/* --------------------------------------------------------------------------
 * Main static analysis:
 * ------------------------------------------------------------------------*/

/* Top level static check on Expr */
void checkExp(void) {
    resetStaticAnalysis();
    clearScope();     /* Analyse expression in the scope */
    withinScope(NIL); /* of no local bindings              */
    inputExpr = depExpr(0, inputExpr);
    leaveScope();
    resetStaticAnalysis();
}

#if EXPLAIN_INSTANCE_RESOLUTION
/* Top level static check on Expr */
void checkContext(void) {
    List vs, qs;

    resetStaticAnalysis();
    clearScope();     /* Analyse expression in the scope */
    withinScope(NIL); /* of no local bindings              */
    qs = inputContext;
    for (vs = NIL; nonNull(qs); qs = tl(qs)) {
        vs = typeVarsIn(hd(qs), NIL, NIL, vs);
    }
    map2Proc(depPredExp, 0, vs, inputContext);
    leaveScope();
    resetStaticAnalysis();
}
#endif

/* Top level static analysis */
void checkDefns(void) {
    List tcs;
    Module thisModule = lastModule();
    resetStaticAnalysis();

    setCurrModule(thisModule);

    /* Resolve module references */
    mapProc(checkQualImport, moduleRec(thisModule)->modAliases);
    mapProc(checkUnqualImport, unqualImports);
    /* Add "import Prelude" if there's no explicit import */
    if ((thisModule != modulePrelude && thisModule != moduleUserPrelude) &&
        isNull(cellAssoc(moduleUserPrelude, unqualImports)) &&
        isNull(cellRevAssoc(moduleUserPrelude,
                            moduleRec(thisModule)->modAliases))) {
        addUnqualImport(moduleUserPrelude, mkCon(textUserPrelude), DOTDOT);
    }
    map1Proc(checkImportList, false, unqualImports);
    mapProc(checkQualImportList, moduleRec(thisModule)->qualImports);

    /* And,  finally, fix-up the effective import lists. */
    fixupImportExports(moduleRec(currentModule)->modImports);

    /* Note: there's a lot of side-effecting going on here, so
       don't monkey about with the order of operations here unless
       you know what you are doing */
    linkPreludeTC();                     /* Get prelude tycons and classes  */
    mapProc(checkTyconDefn, tyconDefns); /* validate tycon definitions     */
    checkSynonyms(tyconDefns);           /* check synonym definitions      */
    mapProc(checkClassDefn, classDefns); /* process class definitions      */
    tcs = tcscc(tyconDefns, classDefns); /* calc dependencies for type      */
                                         /* constructors and classes        */
    mapProc(kindTCGroup, tcs); /* attach kinds                           */
    mapProc(visitClass, classDefns);    /* check class hierarchy          */
    mapProc(extendFundeps, classDefns); /* finish class definitions       */
                                        /* (convenient if we do this after */
                                        /* calling `visitClass' so that we */
                                        /* know the class hierarchy is     */
                                        /* acyclic)                        */
    mapProc(checkClassDefn2_, tcs);     /* process class definitions again */
    mapProc(addMembers, classDefns);    /* add definitions for member funs */
    linkPreludeCM();                    /* Get prelude cfuns and mfuns    */

    mapOver(checkPrimDefn, primDefns); /* check primitive declarations     */

    instDefns = rev(instDefns); /* process instance definitions    */
    mapProc(checkInstDefn, instDefns);

    setCurrModule(thisModule);
    mapProc(addRSsigdecls,
            typeInDefns); /* add sigdecls for RESTRICTSYN           */
    valDefns = eqnsToBindings(valDefns, tyconDefns, classDefns, primDefns);
    mapProc(allNoPrevDef, valDefns); /* check against previous defns    */
    instDefns = appendOnto(instDefns, derivedInstances());
    checkDefaultDefns(); /* validate default definitions           */

    tyconDefns = NIL;
    primDefns = NIL;

    if (nonNull(foreignImports) || nonNull(foreignExports)) {
        /* If generate_ffi is set, we generate a C file which defines
         * appropriate primitives.
         * Otherwise, we try to load a C file and look for those
         * primitives.
         */
        bool need_stubs = foreignNeedStubs(foreignImports, foreignExports);

        if (generate_ffi && need_stubs) {
            foreignHeader(scriptFile);
        }
        mapProc(checkForeignImport, foreignImports);
        mapProc(checkForeignExport, foreignExports);
        if (generate_ffi && need_stubs) {
            foreignFooter(scriptFile, moduleRec(thisModule)->text,
                          foreignImports, foreignExports);
        }
        if (need_stubs && (generate_ffi || !generateFFI)) {
            needPrims(0, NULL);
            mapProc(linkForeign, foreignImports);
        }

        /* We are now finished with foreign import declarations but
         * foreign export declarations need to pass through the
         * typechecker so that we can check that the exported type is
         * an instance of the actual type and so that we can gnerate
         * code which inserts any dictionaries we might need.
         */
        foreignImports = NIL;
    }

    /* Every top-level name has now been created - so we can build the     */
    /* export list.  Note that this has to happen before dependency        */
    /* analysis so that references to Prelude.foo will be resolved         */
    /* when compiling the prelude.                                         */
    moduleRec(thisModule)->exports =
        checkExports(moduleRec(thisModule)->exports);

    mapProc(checkTypeIn, typeInDefns); /* check restricted synonym defns  */

    clearScope();
    withinScope(valDefns);
    valDefns = topDependAnal(valDefns); /* top level dependency ordering   */
    linkPreludeFuns();                  /* Get prelude funs        */
    mapProc(depDefaults, classDefns);   /* dep. analysis on class defaults */
    mapProc(depInsts, instDefns);       /* dep. analysis on inst defns     */
    leaveScope();

    /* ToDo: evalDefaults should match current evaluation module */
    evalDefaults = defaultDefns; /* Set defaults for evaluator     */

    /* A module's 'modImports' list is only used to construct a precise export
     * list in the presence of module re-exportation. We've now finished
     * computing the export list, so 'modImports' can now be stubbed out.
     *
     * 10/02 refinement: hold on to the 'modImports' a little bit longer;
     * until we switch to another module. This lets the user have access
     * to the effective imports when eval/querying from the read-eval-print
     * loop. However, once we switch to another module, the info is gone.
     * (i.e., there's potential for confusion wrt. the :m command.)
     *
     * This is an approximation to simply holding on to a module's
     * effective imports until it is unloaded. This isn't done due to the
     * storage overhead imposed.
     */
    resetStaticAnalysis();
}

/* Add sigdecls from TYPE ... IN .. */
static void addRSsigdecls(Pair pr) {
    List vs = snd(pr); /* get list of variables    */
    for (; nonNull(vs); vs = tl(vs)) {
        if (fst(hd(vs)) == SIGDECL) { /* find a sigdecl                   */
            valDefns = cons(hd(vs), valDefns); /* add to valDefns          */
            hd(vs) = hd(snd3(snd(hd(vs))));    /* and replace with var     */
        }
    }
}

/* Ensure no previous bindings for variables in new binding */
static void allNoPrevDef(Cell b) {
    if (isVar(fst(b))) {
        noPrevDef(rhsLine(snd(hd(snd(snd(b))))), fst(b));
    } else {
        int line = rhsLine(snd(snd(snd(b))));
        map1Proc(noPrevDef, line, fst(b));
    }
}

/* Ensure no previous binding for new variable */
static void noPrevDef(int line, Cell v) {
    Name n = findName(textOf(v));

    if (isNull(n)) {
        n = newName(textOf(v), NIL);
        nameRec(n)->defn = PREDEFINED;
    } else if (nameRec(n)->defn != PREDEFINED &&
               nameRec(n)->mod == currentModule) {
        /* A local repeated definition */
        duplicateError(line, nameRec(n)->mod, nameRec(n)->text, "variable");
    } else if (nameRec(n)->defn != PREDEFINED) {
        Name oldnm = n;
        removeName(n);
        n = newName(textOf(v), NIL);
        nameRec(n)->defn = PREDEFINED;
        nameRec(n)->clashes = cons(oldnm, nameRec(n)->clashes);
    }
    nameRec(n)->line = line;
}

/* Report duplicate defn */
void duplicateError(int line, Module mod, Text t, char *kind) {
    if (mod == currentModule) {
        errHead(line);
        errPrintf("Multiple declarations for %s \"%s\"", kind, textToStr(t));
        errFail();
    } else {
        errHead(line);
        errPrintf("Declaration of %s \"%s\" clashes with import", kind,
                  textToStr(t));
        errFail();
    }
}

/* Check that vars in restricted synonym are defined */
static void checkTypeIn(Pair cvs) {
    Tycon c = fst(cvs);
    List vs = snd(cvs);

    for (; nonNull(vs); vs = tl(vs)) {
        if (isNull(findName(textOf(hd(vs))))) {
            errHead(tyconRec(c)->line);
            errPrintf(
                "No top level binding of \"%s\" for restricted synonym \"%s\"",
                textToStr(textOf(hd(vs))), textToStr(tyconRec(c)->text));
            errFail();
        }
    }
}
/* --------------------------------------------------------------------------
 * Static Analysis control:
 * ------------------------------------------------------------------------*/

/* Register the six sub-modules' roots and reset them; done once, before
 * the Prelude is read. */
void installStaticAnalysis(void) {
    installKind();
    installTycon();
    installTypeexp();
    installPattern();
    installDepend();
    installDeriving();
}

/* Reset the six sub-modules' per-script state together, as they always
 * reset together. */
void resetStaticAnalysis(void) {
    resetKind();
    resetTycon();
    resetTypeexp();
    resetPattern();
    resetDepend();
    resetDeriving();
}

/*-------------------------------------------------------------------------*/
