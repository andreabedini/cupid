/* --------------------------------------------------------------------------
 * Builtin printer, used as an alternative to overloaded "show", and also
 * used for certain primitive types.
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
#include "builtin.h"
#include "char.h"

#include <inttypes.h>

static void printer(Name, int);
static void outName(Name);
static void outVar(Name);
static void outOp(Name);
static void outStr(char *);
static void outPr(Name, int, Cell);
static void outLPr(Name, Cell);
static void outException(Cell);
static void outBadRedex(Cell);
static Cell printDException(Cell);

static Name nameLPrint, nameNLPrint; /* list printing primitives           */
static Name nameSPrint, nameNSPrint; /* string printing primitives         */

static Cell out; /* GC'd var used by printer code   */

#define outCh(c) out = ap(consChar(c), out)
#define outSCh(c) outStr(unlexChar(c, '\"'))
#define updOutRoot(ss)                                                         \
    out = revOnto(out, ss);                                                    \
    updapRoot(fst(out), snd(out));                                             \
    out = NIL;

/* --------------------------------------------------------------------------
 * Printer control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const printerRoots[] = {
    &out,
};

/* Clear the GC'd variable the printer builds its result in. */
static void resetPrinter(void) {
    out = NIL;
}

/* Register the roots, predefine the printer's own primitives, and
 * reset. */
static void installPrinter(void) {
    registerRoots(printerRoots);
    setCurrModule(modulePrelude);
    namePrint = predefPrim("_print", "print");
    nameNPrint = predefPrim("_nprint", "nprint");
    nameLPrint = predefPrim("_lprint", "lprint");
    nameNLPrint = predefPrim("_nlprint", "nlprint");
    nameSPrint = predefPrim("_sprint", "sprint");
    nameNSPrint = predefPrim("_nsprint", "nsprint");
    resetPrinter();
}

/* The hooks builtIn's INSTALL registers this module under. */
static const Subsystem printerSubsystem = {
    "printer", installPrinter, resetPrinter, NULL, NULL, NULL,
};

PROTO_PRIM(primPrint);
PROTO_PRIM(primBPrint);
PROTO_PRIM(primNPrint);
PROTO_PRIM(primLPrint);
PROTO_PRIM(primNLPrint);
PROTO_PRIM(primSPrint);
PROTO_PRIM(primNSPrint);

static struct primitive printerPrimTable[] = {
    {"print", 3, primPrint},
    {"nprint", 3, primNPrint},
    {"lprint", 2, primLPrint},
    {"nlprint", 2, primNLPrint},
    {"sprint", 2, primSPrint},
    {"nsprint", 2, primNSPrint},
    {"primShowsInt", 3, primBPrint},
    {"primShowsPtr", 3, primBPrint},
    {"primShowsInteger", 3, primBPrint},
    {"primShowsFloat", 3, primBPrint},
    {"primShowsDouble", 3, primBPrint},
    {0, 0, 0}};

struct primInfo printerPrims = {&printerSubsystem, printerPrimTable, 0};

/*-------------------------------------------------------------------------*/

/* Evaluate and print term */
primFun(primPrint) {
    Cell temp; /*    :: Int->Expr->[Char]->[Char] */
    int d;
    eval(primArg(3));
    d = whnfInt;
    temp = evalWithNoError(primArg(2));
    out = NIL;
    if (nonNull(temp)) {
        push(temp);
        outException(top());
    } else
        printer(namePrint, d);
    updOutRoot(primArg(1));
}

/* Eval and print value of basic type -- Int, Integer, Float, or Double -- as a
 * string. */
primFun(primBPrint) {
    int d;
    eval(primArg(3));
    d = whnfInt;
    eval(primArg(2));      /* Differs from primPrint only in  */
    out = NIL;             /* its use of weaker error handling*/
    printer(namePrint, d); /* to make showsPrec strict.    */
    updOutRoot(primArg(1));
}

/* Print term without evaluation */
primFun(primNPrint) {
    int d; /*     :: Int->Expr->[Char]->[Char] */
    eval(primArg(3));
    d = whnfInt;
    unwind(primArg(2));
    out = NIL;
    printer(nameNPrint, d);
    updOutRoot(primArg(1));
}

/* Main part: primPrint/primNPrint printer to use on components precedence
 * level */
static void printer(Name pr, int d) {
    int used = 0; /* Output, in reverse, to "out"          */

    allowBreak();

    switch (whatIs(whnfHead)) {

    case NAME: {
        Syntax sy = syntaxOf(whnfHead);

        if (!isCfun(whnfHead) || nameRec(whnfHead)->arity > whnfArgs)
            pr = nameNPrint;

        /* don't print dictionary arguments */
        while (used < whnfArgs) {
            Cell h;
            int w;
            h = pushed(used);
            for (;;) {
                w = whatIs(h);
                if (w == AP)
                    h = fun(h);
                else if (w == INDIRECT)
                    h = arg(h);
                else
                    break;
            }
            if (w == NAME && isNull(nameRec(h)->type) &&
                (whatIs(nameRec(h)->parent) == INSTANCE || isDfun(h) ||
                 strncmp(textToStr(nameRec(h)->text), "Make.", 5) == 0))
                used++;
            else
                break;
        }

        if (whnfHead == nameCons && whnfArgs == 2) { /*list */
            StackPtr ksp = sp;
            used = 2;
            if (pr == namePrint) {
                Cell temp = evalWithNoError(top());
                if (nonNull(temp)) {
                    push(temp);
                    outCh('[');
                    outException(top());
                    sp = ksp;
                    outLPr(nameLPrint, pushed(1));
                } else if (isChar(whnfHead) && whnfArgs == 0) {
                    outCh('"');
                    outSCh(charOf(whnfHead));
                    outLPr(nameSPrint, pushed(1));
                } else {
                    sp = ksp;
                    outCh('[');
                    outPr(namePrint, MIN_PREC, top());
                    outLPr(nameLPrint, pushed(1));
                }
            } else {
                unwind(top());
                if (isChar(whnfHead) && whnfArgs == 0) {
                    outCh('"');
                    outSCh(charOf(whnfHead));
                    outLPr(nameNSPrint, pushed(1));
                } else {
                    sp = ksp;
                    outCh('[');
                    outPr(nameNPrint, MIN_PREC, top());
                    outLPr(nameNLPrint, pushed(1));
                }
            }
            return;
        }
        /* hide fromInt/fromDouble */
        else if (whnfArgs == 2 &&
                 ((whnfHead == nameFromInt && whatIs(pushed(1)) == INTCELL) ||
                  (whnfHead == nameFromDouble &&
                   (whatIs(pushed(1)) == FLOATCELL ||
                    whatIs(pushed(1)) == DOUBLECELL)))) {
            unwind(pushed(1));
            printer(pr, d);
            return;
        } else if (whnfArgs - used == 1 && sy != APPLIC) { /* (e1+) */
            outCh('(');
            outPr(pr, FUN_PREC - 1, pushed(used));
            outCh(' ');
            outOp(whnfHead);
            outCh(')');
            used++;
        } else if (whnfArgs - used >= 2 && sy != APPLIC) { /* e1+e2 */
            Syntax a = assocOf(sy);
            int p = precOf(sy);
            if (whnfArgs - used > 2 || d > p)
                outCh('(');
            outPr(pr, (a == LEFT_ASS ? p : 1 + p), pushed(used));
            used++;
            outCh(' ');
            outOp(whnfHead);
            outCh(' ');
            outPr(pr, (a == RIGHT_ASS ? p : 1 + p), pushed(used));
            used++;
            if (whnfArgs - used > 2 || d > p)
                outCh(')');
        } else /* f ... */
            outVar(whnfHead);
    } break;

    case NEGNUM:
    case ZERONUM:
    case POSNUM:
        out = rev(bigOut(whnfHead, NIL, d >= UMINUS_PREC));
        pr = nameNPrint;
        break;

    case INTCELL: {
        int digit;

        if (intOf(whnfHead) < 0 && d >= UMINUS_PREC)
            outCh(')');

        do {
            digit = whnfInt % 10;
            if (digit < 0)
                digit = (-digit);
            outCh('0' + digit);
        } while ((whnfInt /= 10) != 0);

        if (intOf(whnfHead) < 0) {
            outCh('-');
            if (d >= UMINUS_PREC)
                outCh('(');
        }

        out = rev(out);
        pr = nameNPrint;
    } break;

    case PTRCELL: {
        void *p = ptrOf(whnfHead);
        char buffer[32];
        /* Fall into line with how GHC shows Addrs.
                             * (long)p truncated a 64-bit address on LP64
                             * platforms where %x only takes 32 bits from
                             * the vararg; use PRIxPTR/uintptr_t instead. */
        snprintf(buffer, sizeof buffer, "0x%0*" PRIxPTR, (SIZEOF_INTP) * 2,
                 (uintptr_t)p);
        outStr(buffer);
        pr = nameNPrint;
    } break;

    case TUPLE: {
        int tn = tupleOf(whnfHead);
        int punc = '(';
        int i;

        used = tn < whnfArgs ? tn : whnfArgs;
        for (i = 0; i < used; ++i) {
            outCh(punc);
            outPr(pr, MIN_PREC, pushed(i));
            punc = ',';
        }
        for (; i < tn; ++i) {
            outCh(punc);
            punc = ',';
        }
        outCh(')');
    }
        pr = nameNPrint;
        break;

    case CHARCELL:
        outCh('\'');
        outStr(unlexChar(charOf(whnfHead), '\''));
        outCh('\'');
        pr = nameNPrint;
        break;

    case FLOATCELL:
        if (whnfFloat < 0.0 && d >= UMINUS_PREC)
            outCh('(');
        outStr(floatToString(whnfFloat));
        if (whnfFloat < 0.0 && d >= UMINUS_PREC)
            outCh(')');
        pr = nameNPrint;
        break;

    case DOUBLECELL:
        if (whnfDouble < 0.0 && d >= UMINUS_PREC)
            outCh('(');
        outStr(doubleToString(whnfDouble));
        if (whnfDouble < 0.0 && d >= UMINUS_PREC)
            outCh(')');
        pr = nameNPrint;
        break;

    case ARRAY:
        outStr("{array}");
        pr = nameNPrint;
        break;

    case MUTVAR:
        outStr("{mutable variable}");
        pr = nameNPrint;
        break;

    case CUDDLESOBJECT:
        outStr("{Cell ...}");
        pr = nameNPrint;
        break;

    case HANDCELL:
        outStr("{handle}");
        pr = nameNPrint;
        break;

    default:
        internal("Error in graph");
        break;
    }

    if (used < whnfArgs) { /* Add remaining args to output         */
        do {
            outCh(' ');
            outPr(pr, FUN_PREC, pushed(used));
        } while (++used < whnfArgs);

        if (d >= FUN_PREC) { /* Determine if parens are needed  */
            out = appendOnto(out, singleton(consChar('(')));
            outCh(')');
        }
    }
}

/* --------------------------------------------------------------------------
 * List printing primitives:
 * ------------------------------------------------------------------------*/

/* Evaluate and print list */
primFun(primLPrint) {
    Cell temp = evalWithNoError(primArg(2));
    out = NIL;
    if (nonNull(temp)) {
        push(temp);
        outStr("] ++ ");
        outException(top());
    } else if (whnfHead == nameCons && whnfArgs == 2) {
        outCh(',');
        outPr(namePrint, MIN_PREC, pushed(0));
        outLPr(nameLPrint, pushed(1));
    } else if (whnfHead == nameNil && whnfArgs == 0)
        outCh(']');
    else {
        outStr("] ++ ");
        outBadRedex(primArg(2));
    }
    updOutRoot(primArg(1));
}

/* Print list without evaluation */
primFun(primNLPrint) {
    unwind(primArg(2));
    out = NIL;
    if (whnfHead == nameCons && whnfArgs == 2) {
        outCh(',');
        outPr(nameNPrint, MIN_PREC, pushed(0));
        outLPr(nameNLPrint, pushed(1));
    } else if (whnfHead == nameNil && whnfArgs == 0)
        outCh(']');
    else {
        outStr("] ++ ");
        outPr(nameNPrint, FUN_PREC - 1, primArg(2));
    }
    updOutRoot(primArg(1));
}

/* Evaluate and print string */
primFun(primSPrint) {
    Cell temp = evalWithNoError(primArg(2));
    out = NIL;
    if (nonNull(temp)) {
        push(temp);
        outStr("\" ++ ");
        outException(top());
    } else if (whnfHead == nameCons && whnfArgs == 2) {
        temp = evalWithNoError(top()); /* primArg(4), primArg(3) contain  */
        out = NIL;                     /* the head and tail of list, resp */
        if (nonNull(temp)) {
            push(temp);
            outStr("\" ++ [");
            outException(top());
            outLPr(nameLPrint, primArg(3));
        } else if (isChar(whnfHead) && whnfArgs == 0) {
            outSCh(charOf(whnfHead));
            outLPr(nameSPrint, primArg(3));
        } else {
            outStr("\" ++ [");
            outBadRedex(primArg(4));
            outLPr(nameLPrint, primArg(3));
        }
    } else if (whnfHead == nameNil && whnfArgs == 0)
        outCh('"');
    else {
        outStr("\" ++ ");
        outBadRedex(primArg(2));
    }
    updOutRoot(primArg(1));
}

/* Print string without eval */
primFun(primNSPrint) {
    unwind(primArg(2));
    out = NIL;
    if (whnfHead == nameCons && whnfArgs == 2) {
        unwind(pushed(0));
        if (isChar(whnfHead) && whnfArgs == 0) {
            outSCh(charOf(whnfHead));
            outLPr(nameNSPrint, primArg(3));
        } else {
            outStr("\" ++ [");
            outPr(nameNPrint, MIN_PREC, primArg(4));
            outLPr(nameNLPrint, primArg(3));
        }
    } else if (whnfHead == nameNil && whnfArgs == 0)
        outCh('"');
    else {
        outStr("\" ++ ");
        outPr(nameNPrint, FUN_PREC - 1, primArg(2));
    }
    updOutRoot(primArg(1));
}

/* --------------------------------------------------------------------------
 * Auxiliary functions for printer(s):
 * ------------------------------------------------------------------------*/

/* Output nm using parent field if possible */
static void outName(Name nm) {
    Cell p = nameRec(nm)->parent;
    switch (whatIs(p)) {

    case NAME:
        outName(p);
        outCh('_');
        break;
    default:
        break;
    }
    outStr(textToStr(nameRec(nm)->text));
}

/* Output nm as function symbol */
static void outVar(Name nm) {
    char *s = textToStr(nameRec(nm)->text);
    if ((isascii(*s) && isalpha(*s)) || *s == '_' || *s == '[' || *s == '(')
        outName(nm);
    else {
        outCh('(');
        outName(nm);
        outCh(')');
    }
}

/* Output nm as operator symbol */
static void outOp(Name nm) {
    char *s = textToStr(nameRec(nm)->text);
    if (isascii(s[0]) && isalpha(s[0])) {
        outCh('`');
        outName(nm);
        outCh('`');
    } else
        outName(nm);
}

/* Output string s */
static void outStr(char *s) {
    while (*s)
        outCh(ExtractChar(s));
}

/* Output expr e with printer pr, precedence d */
static void outPr(Name pr, int d, Cell e) {
    out = ap(NIL, out);
    fst(out) = ap(NIL, e);
    fst(fst(out)) = ap(pr, mkInt(d));
}

/* Output list xs with printer pr */
static void outLPr(Name pr, Cell xs) {
    out = ap(NIL, out);
    fst(out) = ap(pr, xs);
}

/* Produce expr to print exception */
static void outException(Cell ex) {
    outCh('{');
    if (isAp(ex) && fun(ex) == nameErrorCall) {
        outStr("error ");
        outPr(nameNPrint, FUN_PREC, arg(ex));
    } else {
        outStr("throw ");
        outPr(nameNPrint, FUN_PREC, ex);
    }
    outCh('}');
}

/* Produce expr to print bad redex */
static void outBadRedex(Cell rx) {
    outCh('{');
    outPr(nameNPrint, MIN_PREC, rx);
    outCh('}');
}

/* Produce expression for exception with special handling */
static Cell printDException(Cell ex) {
    if (isAp(ex) && fun(ex) == nameErrorCall) /* of {error str} exceptions    */
        return arg(ex);
    else
        return printException(ex, nameNil);
}

/* Produce expression for exception */
Cell printException(Cell ex, Cell rs) {
    out = NIL;
    outException(ex);
    return revOnto(out, rs);
}

/* Abandon computation */
void abandon(char *what, Cell ex) {
    push(printDException(ex));
    out = NIL;
    outCh('\n');
    outStr(what);
    outStr(" error: ");
    top() = revOnto(out, top());
    out = NIL;
    outputString(errorStream);
    errAbort();
}

/*-------------------------------------------------------------------------*/
