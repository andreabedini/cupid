/* --------------------------------------------------------------------------
 * Static analysis of patterns.
 *
 * Patterns are parsed as ordinary expressions; this turns each into a
 * proper pattern, checks that constructors are applied to the right
 * number of arguments, and collects the variables it binds in patVars
 * for the dependency analysis to put into scope.
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
#include "fixity.h"
#include "typeexp.h"
#include "tycon.h"
#include "depend.h"
#include "pattern.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static Cell checkMaybeCnkPat(int, Cell);
static Cell checkApPat(int, int, Cell);
static void addToPatVars(int, Cell);
static void checkIsCfun(int, Name);
static void checkCfunArgs(int, Cell, int);

/* --------------------------------------------------------------------------
 * Static analysis of patterns:
 *
 * Patterns are parsed as ordinary (atomic) expressions.  Static analysis
 * makes the following checks:
 *  - Patterns are well formed (according to pattern syntax), including the
 *    special case of (n+k) patterns.
 *  - All constructor functions have been defined and are used with the
 *    correct number of arguments.
 *  - No variable name is used more than once in a pattern.
 *
 * The list of pattern variables occuring in each pattern is accumulated in
 * a global list `patVars', which must be initialised to NIL at appropriate
 * points before using these routines to check for valid patterns.  This
 * mechanism enables the pattern checking routine to be mapped over a list
 * of patterns, ensuring that no variable occurs more than once in the
 * complete pattern list (as is required on the lhs of a function defn).
 * ------------------------------------------------------------------------*/

List patVars; /* List of vars bound in pattern    */

/* Check valid pattern syntax */
Cell checkPat(int line, Cell p) {
    stackCheck();
    switch (whatIs(p)) {
    case VARIDCELL:
    case VAROPCELL:
        addToPatVars(line, p);
        break;

    case INFIX:
        return checkPat(line, tidyInfix(line, snd(p)));

    case AP:
        return checkMaybeCnkPat(line, p);

    case NAME:
    case QUALIDENT:
    case CONIDCELL:
    case CONOPCELL:
        return checkApPat(line, 0, p);

    case ZERONUM:
    case POSNUM:
    case NEGNUM:
    case WILDCARD:
    case STRCELL:
    case CHARCELL:
    case DOUBLECELL:
    case INTCELL:
        break;

    case ASPAT:
        addToPatVars(line, fst(snd(p)));
        snd(snd(p)) = checkPat(line, snd(snd(p)));
        break;

    case LAZYPAT:
        snd(p) = checkPat(line, snd(p));
        break;

    case FINLIST:
        map1Over(checkPat, line, snd(p));
        break;

    case CONFLDS:
        depConFlds(line, p, true);
        break;

    case ESIGN:
        snd(snd(p)) = checkPatType(line, "pattern", fst(snd(p)), snd(snd(p)));
        fst(snd(p)) = checkPat(line, fst(snd(p)));
        break;

    default:
        errHead(line);
        errPrintf("Illegal pattern syntax");
        errFail();
    }
    return p;
}

/* Check applicative pattern with the possibility of n+k pattern */
static Cell checkMaybeCnkPat(int l, Cell p) {
    return checkApPat(l, 0, p);
}

/* Check validity of application of constructor to arguments */
static Cell checkApPat(int line, int args, Cell p) {
    switch (whatIs(p)) {
    case AP:
        fun(p) = checkApPat(line, args + 1, fun(p));
        arg(p) = checkPat(line, arg(p));
        break;

    case TUPLE:
        if (tupleOf(p) != args) {
            errHead(line);
            errPrintf("Illegal tuple pattern");
            errFail();
        }
        break;

    case QUALIDENT:
        if (!isQCon(p)) {
            errHead(line);
            errPrintf("Illegal use of qualified variable in pattern");
            errFail();
        }
        /* deliberate fall through */
    case CONIDCELL:
    case CONOPCELL:
        p = conDefined(line, p, true);
        checkCfunArgs(line, p, args);
        break;

    case NAME:
        checkIsCfun(line, p);
        checkCfunArgs(line, p, args);
        break;

    default:
        errHead(line);
        errPrintf("Illegal pattern syntax");
        errFail();
    }
    return p;
}

/* Add variable v to list of vars in current pattern, checking for repeated
 * variables. */
static void addToPatVars(int line, Cell v) {
    Text t = textOf(v);
    List p = NIL;
    List n = patVars;

    for (; nonNull(n); p = n, n = tl(n)) {
        if (textOf(hd(n)) == t) {
            errHead(line);
            errPrintf("Repeated variable \"%s\" in pattern", textToStr(t));
            errFail();
        }
    }

    if (isNull(p)) {
        patVars = cons(v, NIL);
    } else {
        tl(p) = cons(v, NIL);
    }
}

/* Check that nm is the name of a previously defined constructor function (and
 * only one.) */
Name conDefined(int line, Cell nm, bool check) {
    Name n = findQualName(nm);
    if (isNull(n)) {
        errHead(line);
        errPrintf("Undefined data constructor \"%s\"", identToStr(nm));
        errFail();
    }
    checkIsCfun(line, n);
    if (check) {
        if (isQualIdent(nm)) {
            depQVar(line, nm, false);
        } else {
            checkNameAmbigName(line, n, false);
        }
    }
    return n;
}

/* Check that c is a constructor fn */
static void checkIsCfun(int line, Name c) {
    if (!isCfun(c)) {
        errHead(line);
        errPrintf("\"%s\" is not a data constructor",
                  textToStr(nameRec(c)->text));
        errFail();
    }
}

/* Check constructor applied with correct number of arguments */
static void checkCfunArgs(int line, Cell c, int args) {
    int a = userArity(c);
    if (a != args) {
        errHead(line);
        errPrintf(
            "Constructor \"%s\" must have exactly %d argument%s in pattern",
            textToStr(nameRec(c)->text), a, ((a == 1) ? "" : "s"));
        errFail();
    }
}

/* Check type appearing in pattern */
Cell checkPatType(int l, char *wh, Cell e, Type t) {
    List tvs = typeVarsIn(t, NIL, NIL, NIL);
    for (; nonNull(tvs); tvs = tl(tvs)) {
        int beta = newKindvars(1);
        hd(btyvars) = cons(pair(hd(tvs), mkInt(beta)), hd(btyvars));
    }
    t = checkSigType(l, "pattern type", e, t);
    if (isPolyOrQualType(t) || whatIs(t) == RANK2) {
        errHead(l);
        errPrintf("Illegal syntax in %s type annotation", wh);
        errFail();
    }
    return t;
}

/* Record bound type vars in pat */
Cell applyBtyvs(Cell pat) {
    List bts = hd(btyvars);
    leaveBtyvs();
    if (nonNull(bts)) {
        pat = ap(BIGLAM, pair(bts, pat));
        for (; nonNull(bts); bts = tl(bts)) {
            snd(hd(bts)) = copyKindvar(intOf(snd(hd(bts))));
        }
    }
    return pat;
}

/* --------------------------------------------------------------------------
 * Control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const patternRoots[] = {
    &patVars,
};

/* Clear the variables bound by the pattern being checked. */
void resetPattern(void) {
    patVars = NIL;
}

/* Register the roots and reset; done once, before the Prelude is read. */
void installPattern(void) {
    registerRoots(patternRoots);
    resetPattern();
}

/*-------------------------------------------------------------------------*/
