/* --------------------------------------------------------------------------
 * Turning value declarations into bindings.
 *
 * The parser hands over a list of equations; this groups the ones that
 * define the same function, pairs each binding with its type signature
 * and fixity, and checks that no signature or fixity is left without a
 * definition.
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
#include "typeexp.h"
#include "binding.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static List getPatVars(int, Cell, List);
static List addPatVar(int, Cell, List);
static void notDefined(int, List, Cell);
static Cell getAttr(List, Cell);
static void addSigdecl(List, Cell);
static bool isTypeOperator(List, Text);
static void addFixdecl(List, List, List, List, Triple);

/* --------------------------------------------------------------------------
 * Before processing value and type signature declarations, all data and
 * type definitions have been processed so that:
 * - all valid type constructors (with their arities) are known.
 * - all valid constructor functions (with their arities and types) are
 *   known.
 *
 * The result of parsing a list of value declarations is a list of Eqns:
 *       Eqn ::= (SIGDECL,(Line,[Var],type))
 *            |  (FIXDECL,(Line,[Op],SyntaxInt))
 *            |  (Expr,Rhs)
 * The ordering of the equations in this list is the reverse of the original
 * ordering in the script parsed.  This is a consequence of the structure of
 * the parser ... but also turns out to be most convenient for the static
 * analysis.
 *
 * As the first stage of the static analysis of value declarations, each
 * list of Eqns is converted to a list of Bindings.  As part of this
 * process:
 * - The ordering of the list of Bindings produced is the same as in the
 *   original script.
 * - When a variable (function) is defined over a number of lines, all
 *   of the definitions should appear together and each should give the
 *   same arity to the variable being defined.
 * - No variable can have more than one definition.
 * - For pattern bindings:
 *   - Each lhs is a valid pattern/function lhs, all constructor functions
 *     have been defined and are used with the correct number of arguments.
 *   - Each lhs contains no repeated pattern variables.
 *   - An equation that defines no variable (e.g. True = False) is legal
 *     Haskell and is accepted; Gofer rejected it.
 * - Types appearing in type signatures are well formed:
 *    - Type constructors used are defined and used with correct number
 *      of arguments.
 *    - type variables are replaced by offsets, type constructor names
 *      by Tycons.
 * - Every variable named in a type signature declaration is defined by
 *   one or more equations elsewhere in the script.
 * - No variable has more than one type declaration.
 * - Similar properties for fixity declarations.
 *
 * ------------------------------------------------------------------------*/

#define bindingAttr(b) fst(snd(b)) /* type(s)/fixity(ies) for binding  */
#define fbindAlts(b) snd(snd(b))   /* alternatives for function binding*/

/* Extract the SIGDECLS from list of equations */
List extractSigdecls(List es) {
    List sigdecls = NIL; /* :: [(Line,[Var],Type)]       */

    for (; nonNull(es); es = tl(es)) {
        if (fst(hd(es)) == SIGDECL) { /* type-declaration?  */
            Pair sig = snd(hd(es));
            int line = intOf(fst3(sig));
            List vs = snd3(sig);
            for (; nonNull(vs); vs = tl(vs)) {
                if (isQualIdent(hd(vs))) {
                    errHead(line);
                    errPrintf(
                        "Type signature for qualified variable \"%s\" is not allowed",
                        identToStr(hd(vs)));
                    errFail();
                }
            }
            sigdecls = cons(sig, sigdecls); /* discard SIGDECL tag*/
        }
    }
    return sigdecls;
}

/* Extract the FIXDECLS from list of equations */
List extractFixdecls(List es) {
    List fixdecls = NIL; /* :: [(Line,SyntaxInt,[Op])]           */

    for (; nonNull(es); es = tl(es)) {
        if (fst(hd(es)) == FIXDECL) {               /* fixity declaration?*/
            fixdecls = cons(snd(hd(es)), fixdecls); /* discard FIXDECL tag*/
        }
    }
    return fixdecls;
}

/* Extract untyped bindings from given list of equations */
List extractBindings(List ds) {
    Cell lastVar = NIL; /* = var def'd in last eqn (if any)*/
    int lastArity = 0;  /* = number of args in last defn   */
    List bs = NIL;      /* :: [Binding]                         */

    for (; nonNull(ds); ds = tl(ds)) {
        Cell d = hd(ds);
        if (fst(d) == FUNBIND) { /* Function bindings              */
            Cell rhs = snd(snd(d));
            int line = rhsLine(rhs);
            Cell lhs = fst(snd(d));
            Cell v = getHead(lhs);
            Cell newAlt = pair(getArgs(lhs), rhs);
            if (!isVar(v)) {
                internal("FUNBIND");
            }
            if (nonNull(lastVar) && (textOf(v)) == textOf(lastVar)) {
                if (argCount != lastArity) {
                    errHead(line);
                    errPrintf("Equations give different arities for \"%s\"",
                              textToStr(textOf(v)));
                    errFail();
                }
                fbindAlts(hd(bs)) = cons(newAlt, fbindAlts(hd(bs)));
            } else {
                lastVar = v;
                lastArity = argCount;
                notDefined(line, bs, v);
                bs = cons(pair(v, pair(NIL, singleton(newAlt))), bs);
            }

        } else if (fst(d) == PATBIND) { /* Pattern bindings                */
            Cell rhs = snd(snd(d));
            int line = rhsLine(rhs);
            Cell pat = fst(snd(d));
            while (whatIs(pat) == ESIGN) { /* Move type annotations to rhs   */
                Cell p = fst(snd(pat));
                fst(snd(pat)) = rhs;
                snd(snd(d)) = rhs = pat;
                fst(snd(d)) = pat = p;
                /* Lift out the line number, i.e.,
                 * (ESIGN,((location, expr),ty)) ~=> (location,(ESIGN,(expr,ty)))
                 */
                p = snd(rhs);
                fst(rhs) = fst(fst(p));
                snd(rhs) = ap(ESIGN, ap(snd(fst(p)), snd(p)));
            }
            if (isVar(pat)) { /* Convert simple pattern bind to */
                notDefined(line, bs,
                           pat); /* a function binding                  */
                bs = cons(pair(pat, pair(NIL, singleton(pair(NIL, rhs)))), bs);
            } else {
                List vs = getPatVars(line, pat, NIL);
                map2Proc(notDefined, line, bs, vs);
                bs = cons(pair(vs, pair(NIL, snd(d))), bs);
            }
            lastVar = NIL;
        } else
            lastVar = NIL;
    }
    return bs;
}

/* Find list of variables bound in pattern p */
static List getPatVars(int line, Cell p, List vs) {
    switch (whatIs(p)) {
    case AP:
        do {
            vs = getPatVars(line, arg(p), vs);
            p = fun(p);
        } while (isAp(p));
        return vs; /* Ignore head of application           */

    case CONFLDS: {
        List pfs = snd(snd(p));
        for (; nonNull(pfs); pfs = tl(pfs)) {
            if (isVar(hd(pfs))) {
                vs = addPatVar(line, hd(pfs), vs);
            } else {
                vs = getPatVars(line, snd(hd(pfs)), vs);
            }
        }
    }
        return vs;

    case FINLIST: {
        List ps = snd(p);
        for (; nonNull(ps); ps = tl(ps)) {
            vs = getPatVars(line, hd(ps), vs);
        }
    }
        return vs;

    case ESIGN:
        return getPatVars(line, fst(snd(p)), vs);

    case LAZYPAT:
    case NEG:
    case ONLY:
    case INFIX:
    case SECTION:
        return getPatVars(line, snd(p), vs);

    case ASPAT:
        return addPatVar(line, fst(snd(p)), getPatVars(line, snd(snd(p)), vs));

    case VARIDCELL:
    case VAROPCELL:
        return addPatVar(line, p, vs);

    case CONIDCELL:
    case CONOPCELL:
    case QUALIDENT:
    case INTCELL:
    case DOUBLECELL:
    case CHARCELL:
    case STRCELL:
    case NAME:
    case WILDCARD:
        return vs;

    default:
        internal("getPatVars");
    }
    return vs;
}

/* Add var to list of previously encountered variables */
static List addPatVar(int line, Cell v, List vs) {
    if (varIsMember(textOf(v), vs)) {
        errHead(line);
        errPrintf("Repeated use of variable \"%s\" in pattern binding",
                  textToStr(textOf(v)));
        errFail();
    }
    return cons(v, vs);
}

/* Convert list of equations to list of typed bindings */
List eqnsToBindings(List es, List ts, List cs, List ps) {
    List bs = extractBindings(es);
    map1Proc(addSigdecl, bs, extractSigdecls(es));
    map4Proc(addFixdecl, bs, ts, cs, ps, extractFixdecls(es));
    return bs;
}

/* Check if name already defined in list of bindings */
static void notDefined(int line, List bs, Cell v) {
    if (nonNull(findBinding(textOf(v), bs))) {
        errHead(line);
        errPrintf("\"%s\" multiply defined", textToStr(textOf(v)));
        errFail();
    }
}

/* Look for binding for variable t in list of bindings bs */
Cell findBinding(Text t, List bs) {
    for (; nonNull(bs); bs = tl(bs)) {
        if (isVar(fst(hd(bs)))) { /* function-binding? */
            if (textOf(fst(hd(bs))) == t) {
                return hd(bs);
            }
        } else if (nonNull(varIsMember(t, fst(hd(bs))))) { /* pattern-binding?*/
            return hd(bs);
        }
    }
    return NIL;
}

/* Locate type/fixity attribute for variable v in bindings bs */
static Cell getAttr(List bs, Cell v) {
    Text t = textOf(v);
    Cell b = findBinding(t, bs);

    if (isNull(b)) { /* No binding         */
        return NIL;
    } else if (isVar(fst(b))) { /* func binding?   */
        if (isNull(bindingAttr(b))) {
            bindingAttr(b) = pair(NIL, NIL);
        }
        return bindingAttr(b);
    } else { /* pat binding?       */
        List vs = fst(b);
        List as = bindingAttr(b);

        if (isNull(as)) {
            bindingAttr(b) = as = replicate(length(vs), NIL);
        }

        while (nonNull(vs) && t != textOf(hd(vs))) {
            vs = tl(vs);
            as = tl(as);
        }

        if (isNull(vs)) {
            internal("getAttr");
        } else if (isNull(hd(as))) {
            hd(as) = pair(NIL, NIL);
        }
        return hd(as);
    }
}

/* Add type information to bindings :: [Binding] :: (Line,[Var],Type) */
static void addSigdecl(List bs, Cell sigdecl) {
    int l = intOf(fst3(sigdecl));
    List vs = snd3(sigdecl);
    Type type = checkSigType(l, "type declaration", hd(vs), thd3(sigdecl));

    for (; nonNull(vs); vs = tl(vs)) {
        Cell v = hd(vs);
        Pair attr = getAttr(bs, v);
        if (isNull(attr)) {
            errHead(l);
            errPrintf("Missing binding for variable \"%s\" in type signature",
                      textToStr(textOf(v)));
            errFail();
        } else if (nonNull(fst(attr))) {
            errHead(l);
            errPrintf("Multiple type signatures for \"%s\"",
                      textToStr(textOf(v)));
            errFail();
        }
        fst(attr) = type;
    }
}

/* Is t the name of one of the type constructors ts?  A fixity declaration
 * for a type operator that is not also a data constructor is accepted, and
 * has no effect, since fixities do not apply to types. */
static bool isTypeOperator(List ts, Text t) {
    for (; nonNull(ts); ts = tl(ts)) {
        if (tyconRec(hd(ts))->text == t) {
            return true;
        }
    }
    return false;
}

static void addFixdecl(List bs, List ts, List cs, List ps, Triple fixdecl) {
    int line = intOf(fst3(fixdecl));
    List ops = snd3(fixdecl);
    Cell sy = thd3(fixdecl);

    for (; nonNull(ops); ops = tl(ops)) {
        Cell op = hd(ops);
        Text t = textOf(op);
        Cell attr = getAttr(bs, op);
        if (nonNull(attr)) { /* Found name in binding?     */
            if (nonNull(snd(attr))) {
                dupFixity(line, t);
            }
            snd(attr) = sy;
        } else { /* Look in tycons, classes, prims  */
            Name n = NIL;
            List ts1 = ts;
            List cs1 = cs;
            List ps1 = ps;
            for (; isNull(n) && nonNull(ts1); ts1 = tl(ts1)) { /* tycons  */
                Tycon tc = hd(ts1);
                if (tyconRec(tc)->what == DATATYPE ||
                    tyconRec(tc)->what == NEWTYPE) {
                    n = nameIsMember(t, tyconRec(tc)->defn);
                }
            }
            for (; isNull(n) && nonNull(cs1); cs1 = tl(cs1)) { /* classes */
                n = nameIsMember(t, cclassRec(hd(cs1))->members);
            }
            for (; isNull(n) && nonNull(ps1); ps1 = tl(ps1)) { /* prims   */
                n = nameIsMember(t, hd(ps1));
            }

            if (isNull(n) && isTypeOperator(ts, t)) {
                continue; /* types take no fixity; see buildOpType */
            }
            if (isNull(n)) {
                missFixity(line, t);
            } else if (nameRec(n)->syntax != NO_SYNTAX) {
                dupFixity(line, t);
            }
            nameRec(n)->syntax = intOf(sy);
        }
    }
}

/* Report repeated fixity decl */
void dupFixity(int line, Text t) {
    errHead(line);
    errPrintf("Multiple fixity declarations for operator \"%s\"", textToStr(t));
    errFail();
}

/* Report missing op for fixity */
void missFixity(int line, Text t) {
    errHead(line);
    errPrintf("Cannot find binding for operator \"%s\" in fixity declaration",
              textToStr(t));
    errFail();
}

/*-------------------------------------------------------------------------*/
