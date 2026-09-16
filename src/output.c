/* --------------------------------------------------------------------------
 * Unparse expressions and types - for use in error messages, type checker
 * and for debugging.
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
#include <ctype.h>

#define DEPTH_LIMIT 15

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static void putChr(int);
static void putStr(char *);
static void putInt(int);

static void put(int, Cell);
static void putFlds(Cell, List);
static void putComp(Cell, List);
static void putQual(Cell);
static bool isDictVal(Cell);
static Cell maySkipDict(Cell);
static void putAp(int, Cell);
static void putOverInfix(int, Text, Syntax, Cell);
static void putInfix(int, Text, Syntax, Cell, Cell);
static void putSimpleAp(Cell, int);
static void putTuple(int, Cell);
static int unusedTups(int, Cell);
static void unlexVar(Text);
static void unlexOp(Text);
static void unlexCharConst(Char);
static void unlexStrConst(Text);

static void putSigType(Cell);
static void putContext(List, List, int);
static void putPred(Cell, int);
static void putType(Cell, int, int);
static void putModule(Module);
static void putTyVar(int);
static bool putTupleType(Cell, int);
static void putApType(Type, int, int);

static void putKind(Kind);
static void putKinds(Kinds);

static bool isStrConst(Cell);
static void putStrConst(Cell);
static bool isCharCell(Cell);
static Char getCellChar(Cell);

/* --------------------------------------------------------------------------
 * Basic output routines:
 * ------------------------------------------------------------------------*/

static FILE *outputStream; /* current output stream                */

#define OPEN(b)                                                                \
    if (b)                                                                     \
        putChr('(');
#define CLOSE(b)                                                               \
    if (b)                                                                     \
        putChr(')');

/* Print single character */
static void putChr(int c) {
    Putc(c, outputStream);
}

/* Print string */
static void putStr(char *s) {
    for (; *s; s++) {
        Putc(*s, outputStream);
    }
}

/* Print integer */
static void putInt(int n) {
    static char intBuf[16];
    snprintf(intBuf, sizeof intBuf, "%d", n);
    putStr(intBuf);
}

/* --------------------------------------------------------------------------
 * Precedence values (See Haskell 1.3 report, p.12):
 * ------------------------------------------------------------------------*/

#define ALWAYS FUN_PREC           /* Always use parens (unless atomic)*/
                                  /* User defined operators have prec */
                                  /* in the range MIN_PREC..MAX_PREC  */
#define ARROW_PREC MAX_PREC       /* for printing -> in type exprs    */
#define COCO_PREC (MIN_PREC - 1)  /* :: is left assoc, low precedence */
#define COND_PREC (MIN_PREC - 2)  /* conditional expressions          */
#define WHERE_PREC (MIN_PREC - 3) /* where expressions                */
#define LAM_PREC (MIN_PREC - 4)   /* lambda abstraction               */
#define NEVER LAM_PREC            /* Never use parentheses            */

/* --------------------------------------------------------------------------
 * Print an expression (used to display context of type errors):
 * ------------------------------------------------------------------------*/

static int putDepth = 0; /* limits depth of printing DBG     */

/* Print expression e in context of operator of precedence d */
static void put(int d, Cell e) {
    List xs;

    if (putDepth > DEPTH_LIMIT) {
        putStr("...");
        return;
    } else
        putDepth++;

    switch (whatIs(e)) {
    case FINLIST:
        putChr('[');
        xs = snd(e);
        if (nonNull(xs)) {
            put(NEVER, hd(xs));
            while (nonNull(xs = tl(xs))) {
                putChr(',');
                put(NEVER, hd(xs));
            }
        }
        putChr(']');
        break;

    case AP:
        putAp(d, e);
        break;

    case NAME:
        putModule(nameRec(e)->mod);
        unlexVar(nameRec(e)->text);
        break;

    case VARIDCELL:
    case VAROPCELL:
    case DICTVAR:
    case CONIDCELL:
    case CONOPCELL:
        unlexVar(textOf(e));
        break;

    case FREECELL:
        putStr("{free!}");
        break;

    case TUPLE:
        putTuple(tupleOf(e), e);
        break;

    case WILDCARD:
        putChr('_');
        break;

    case ASPAT:
        put(NEVER, fst(snd(e)));
        putChr('@');
        put(ALWAYS, snd(snd(e)));
        break;

    case LAZYPAT:
        putChr('~');
        put(ALWAYS, snd(e));
        break;

    case DOCOMP:
        putStr("do {...}");
        break;

    case COMP:
        putComp(fst(snd(e)), snd(snd(e)));
        break;

    case MONADCOMP:
        putComp(fst(snd(snd(e))), snd(snd(snd(e))));
        break;

    case CHARCELL:
        unlexCharConst(charOf(e));
        break;

    case INTCELL: {
        int i = intOf(e);
        if (i < 0 && d >= UMINUS_PREC)
            putChr('(');
        putInt(i);
        if (i < 0 && d >= UMINUS_PREC)
            putChr(')');
    } break;

    case NEGNUM:
    case ZERONUM:
    case POSNUM:
        xs = bigOut(e, NIL, d >= UMINUS_PREC);
        for (; nonNull(xs); xs = tl(xs))
            putChr(charOf(arg(hd(xs))));
        break;

    case DOUBLECELL: {
        Double f = (Double)doubleOf(e);
        if (f < 0 && d >= UMINUS_PREC)
            putChr('(');
        putStr(doubleToString(f));
        if (f < 0 && d >= UMINUS_PREC)
            putChr(')');
    } break;

    case STRCELL:
        unlexStrConst(textOf(e));
        break;

    case LETREC:
        OPEN(d > WHERE_PREC);
#if DEBUG_CODE
        putStr("let {");
        put(NEVER, fst(snd(e)));
        putStr("} in ");
#else
        putStr("let {...} in ");
#endif
        put(WHERE_PREC + 1, snd(snd(e)));
        CLOSE(d > WHERE_PREC);
        break;

    case COND:
        OPEN(d > COND_PREC);
        putStr("if ");
        put(COND_PREC + 1, fst3(snd(e)));
        putStr(" then ");
        put(COND_PREC + 1, snd3(snd(e)));
        putStr(" else ");
        put(COND_PREC + 1, thd3(snd(e)));
        CLOSE(d > COND_PREC);
        break;

    case LAMBDA:
        xs = fst(snd(e));
        if (whatIs(xs) == BIGLAM)
            xs = snd(snd(xs));
        while (nonNull(xs) && isDictVal(hd(xs)))
            xs = tl(xs);
        if (isNull(xs)) {
            put(d, snd(snd(snd(e))));
            break;
        }
        OPEN(d > LAM_PREC);
        putChr('\\');
        if (nonNull(xs)) {
            put(ALWAYS, hd(xs));
            while (nonNull(xs = tl(xs))) {
                putChr(' ');
                put(ALWAYS, hd(xs));
            }
        }
        putStr(" -> ");
        put(LAM_PREC, snd(snd(snd(e))));
        CLOSE(d > LAM_PREC);
        break;

    case ESIGN:
        OPEN(d > COCO_PREC);
        put(COCO_PREC, fst(snd(e)));
        putStr(" :: ");
        putSigType(snd(snd(e)));
        CLOSE(d > COCO_PREC);
        break;

    case BIGLAM:
        put(d, snd(snd(e)));
        break;

    case CASE:
        putStr("case ");
        put(NEVER, fst(snd(e)));
#if DEBUG_CODE
        putStr(" of {");
        put(NEVER, snd(snd(e)));
        putChr('}');
#else
        putStr(" of {...}");
#endif
        break;

    case CONFLDS:
        putFlds(fst(snd(e)), snd(snd(e)));
        break;

    case UPDFLDS:
        putFlds(fst3(snd(e)), thd3(snd(e)));
        break;

    case INDIRECT:
        putChr('^');
        put(ALWAYS, snd(e));
        break;
    default: /*internal("put");*/
        putChr('$');
        putInt(e);
        break;
    }
    putDepth--;
}

/* Output exp using labelled fields */
static void putFlds(Cell exp, List fs) {
    put(ALWAYS, exp);
    putStr(" {");
    for (; nonNull(fs); fs = tl(fs)) {
        Cell v = hd(fs);
        if (isVar(v))
            put(NEVER, v);
        else {
            Cell f = fst(v);
            Cell e = snd(v);
            Text t = isName(f)  ? nameRec(f)->text
                     : isVar(f) ? textOf(f)
                                : inventText();
            Text s = isName(e)  ? nameRec(e)->text
                     : isVar(e) ? textOf(e)
                                : inventText();

            put(NEVER, f);
            if (s != t) { /* not a punned field            */
                putStr(" = ");
                put(NEVER, e);
            }
        }
        if (nonNull(tl(fs)))
            putStr(", ");
    }
    putChr('}');
}

/* Print comprehension */
static void putComp(Cell e, List qs) {
    putStr("[ ");
    put(NEVER, e);
    if (nonNull(qs)) {
        putStr(" | ");
        putQual(hd(qs));
        while (nonNull(qs = tl(qs))) {
            putStr(", ");
            putQual(hd(qs));
        }
    }
    putStr(" ]");
}

/* Print list comp qualifier */
static void putQual(Cell q) {
    switch (whatIs(q)) {
    case BOOLQUAL:
        put(NEVER, snd(q));
        return;

    case QWHERE:
        putStr("let {...}");
        return;

    case FROMQUAL:
        put(ALWAYS, fst(snd(q)));
        putStr("<-");
        put(NEVER, snd(snd(q)));
        return;
    default:
        break;
    }
}

/* Look for dictionary value */
static bool isDictVal(Cell e) {
#if !DEBUG_CODE
    Cell h = getHead(e);
    switch (whatIs(h)) {
    case DICTVAR:
        return true;
    case NAME:
        return isDfun(h);
    default:
        break;
    }
#endif
    return false;
}

/* Descend function application, ignoring dict aps */
static Cell maySkipDict(Cell e) {
    while (isAp(e) && isDictVal(arg(e)))
        e = fun(e);
    return e;
}

/* Print application (args>=1) */
static void putAp(int d, Cell e) {
    Cell h = NIL;
    Text t = NIL;
    Syntax sy = NIL;
    int args = 0;

    for (h = e; isAp(h); h = fun(h)) /* find head of expression, looking*/
        if (!isDictVal(arg(h)))      /* for dictionary arguments        */
            args++;

    if (args == 0) { /* Special case when *all* args    */
        put(d, h);   /* are dictionary values           */
        return;
    }

    switch (whatIs(h)) {

    case TUPLE:
        OPEN(args > tupleOf(h) && d >= FUN_PREC);
        putTuple(tupleOf(h), e);
        CLOSE(args > tupleOf(h) && d >= FUN_PREC);
        return;

    case NAME:
        if (args == 1 && ((h == nameFromInt && isInt(arg(e))) ||
                          (h == nameFromInteger && isBignum(arg(e))) ||
                          (h == nameFromDouble && isDouble(arg(e))))) {
            put(d, arg(e));
            return;
        }
        t = nameRec(h)->text;
        sy = syntaxOf(h);
        break;

    case VARIDCELL:
    case VAROPCELL:
    case DICTVAR:
    case CONIDCELL:
    case CONOPCELL:
        sy = defaultSyntax(t = textOf(h));
        break;

    default:
        sy = APPLIC;
        break;
    }

    e = maySkipDict(e);

    if (sy == APPLIC) { /* print simple application        */
        OPEN(d >= FUN_PREC);
        putSimpleAp(e, args);
        CLOSE(d >= FUN_PREC);
        return;
    } else if (args == 1) { /* print section of the form (e+)  */
        putChr('(');
        put(FUN_PREC - 1, arg(e));
        putChr(' ');
        unlexOp(t);
        putChr(')');
    } else if (args == 2 && isStrConst(e))
        putStrConst(e);
    else if (args == 2) /* infix expr of the form e1 + e2   */
        putInfix(d, t, sy, arg(maySkipDict(fun(e))), arg(e));
    else { /* o/w (e1 + e2) e3 ... en   (n>=3) */
        OPEN(d >= FUN_PREC);
        putOverInfix(args, t, sy, e);
        CLOSE(d >= FUN_PREC);
    }
}

static void putStrConst(Cell e) {
    putChr('"');
    while (isAp(e) && isAp(fun(e)) && fun(fun(e)) == nameCons) {
        putStr(unlexChar(getCellChar(arg(fun(e))), '"'));
        e = arg(e);
    }
    if (e != nameNil)
        internal("putStrConst");
    putChr('"');
}

static bool isStrConst(Cell e) {
    while (isAp(e) && isAp(fun(e)) && fun(fun(e)) == nameCons) {
        if (!isCharCell(arg(fun(e))))
            return false;
        e = arg(e);
    }
    return e == nameNil;
}

static bool isCharCell(Cell e) {
    while (1)
        switch (whatIs(e)) {
        case CHARCELL:
            return 1;
        case INDIRECT:
            e = snd(e);
            break;
        default:
            return 0;
        }
}

static Char getCellChar(Cell e) {
    while (1)
        switch (whatIs(e)) {
        case CHARCELL:
            return charOf(e);
        case INDIRECT:
            e = snd(e);
            break;
        default:
            internal("error in getCellChar");
        }
}

/* Infix applied to >= 3 arguments */
static void putOverInfix(int args, Text t, Syntax sy, Cell e) {
    if (args > 2) {
        putOverInfix(args - 1, t, sy, maySkipDict(fun(e)));
        putChr(' ');
        put(FUN_PREC, arg(e));
    } else
        putInfix(ALWAYS, t, sy, arg(maySkipDict(fun(e))), arg(e));
}

/* Print infix expression Infix operator symbol with name t, syntax s Left and
 * right operands */
static void putInfix(int d, Text t, Syntax sy, Cell e, Cell f) {
    Syntax a = assocOf(sy);
    int p = precOf(sy);
    OPEN(d > p);
    put((a == LEFT_ASS ? p : 1 + p), e);
    putChr(' ');
    unlexOp(t);
    putChr(' ');
    put((a == RIGHT_ASS ? p : 1 + p), f);
    CLOSE(d > p);
}

/* Print application e0 e1 ... en */
static void putSimpleAp(Cell e, int n) {
    if (n > 0) {
        putSimpleAp(maySkipDict(fun(e)), n - 1);
        putChr(' ');
        put(FUN_PREC, arg(e));
    } else
        put(FUN_PREC, e);
}

/* Print tuple expression, allowing for possibility of either too few or too
 * many args to constr */
static void putTuple(int ts, Cell e) {
    int i;
    putChr('(');
    if ((i = unusedTups(ts, e)) > 0) {
        while (--i > 0)
            putChr(',');
        putChr(')');
    }
}

/* Print first part of tuple expr returning number of constructor args not yet
 * printed ... */
static int unusedTups(int ts, Cell e) {
    if (isAp(e)) {
        if ((ts = unusedTups(ts, fun(e)) - 1) >= 0) {
            put(NEVER, arg(e));
            putChr(ts > 0 ? ',' : ')');
        } else {
            putChr(' ');
            put(FUN_PREC, arg(e));
        }
    }
    return ts;
}

/* Print text as a variable name operator symbols must be enclosed in
 * parentheses... except [] ... */
static void unlexVar(Text t) {
    char *s = textToStr(t);
    if ((isascii(s[0]) && isalpha(s[0])) || s[0] == '_' || s[0] == '[' ||
        s[0] == '(')
        putStr(s);
    else {
        putChr('(');
        putStr(s);
        putChr(')');
    }
}

/* Print text as operator name alpha numeric symbols must be enclosed by
 * backquotes */
static void unlexOp(Text t) {
    char *s = textToStr(t);

    if (isascii(s[0]) && isalpha(s[0])) {
        putChr('`');
        putStr(s);
        putChr('`');
    } else
        putStr(s);
}

static void unlexCharConst(Char c) {
    putChr('\'');
    putStr(unlexChar(c, '\''));
    putChr('\'');
}

static void unlexStrConst(Text t) {
    char *s = textToStr(t);
    static Char SO = 14; /* ASCII code for '\SO'             */
    bool lastWasSO = false;
    bool lastWasDigit = false;
    bool lastWasEsc = false;

    putChr('\"');
    while (*s) {
        char *ch;
        Char c = ' ';
        Char sc = getStrChr(&s);

        ch = unlexChar(sc, '\"');

        if ((lastWasSO && *ch == 'H') ||
            (lastWasEsc && lastWasDigit && isascii(*ch) && isdigit(*ch)))
            putStr("\\&");

        lastWasEsc = (*ch == '\\');
        lastWasSO = (sc == SO);
        for (; *ch; c = *ch++)
            putChr(*ch);
        lastWasDigit = (isascii(c) && isdigit(c));
    }
    putChr('\"');
}

/* --------------------------------------------------------------------------
 * Pretty printer for supercombinator definitions:
 * (i.e., for lambda-lifter output, immediately prior to code generation.)
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Print type expression:
 * ------------------------------------------------------------------------*/

/* Print (possibly) generic type */
static void putSigType(Cell t) {
    int fr = 0;
    if (isPolyType(t)) {
        Kinds ks = polySigOf(t);
        for (; isAp(ks); ks = tl(ks))
            fr++;
        t = monotypeOf(t);
    }

    putType(t, NEVER, fr); /* Finally, print rest of type ... */
}

/* Print context list */
static void putContext(List ps, List qs, int fr) {
    int len = length(ps) + length(qs);
    int c = len;
    bool useParens = len != 1;
    if (useParens)
        putChr('(');
    for (; nonNull(ps); ps = tl(ps)) {
        putPred(hd(ps), fr);
        if (--c > 0) {
            putStr(", ");
        }
    }
    for (; nonNull(qs); qs = tl(qs)) {
        putPred(hd(qs), fr);
        if (--c > 0) {
            putStr(", ");
        }
    }
    if (useParens)
        putChr(')');
}

/* Output predicate */
static void putPred(Cell pi, int fr) {
    if (isAp(pi)) {
        putPred(fun(pi), fr);
        putChr(' ');
        putType(arg(pi), ALWAYS, fr);
    } else if (isClass(pi)) {
        putModule(cclassRec(pi)->mod);
        putStr(textToStr(cclassRec(pi)->text));
    } else if (isCon(pi)) {
        putStr(textToStr(textOf(pi)));
    } else
        putStr("<unknownPredicate>");
}

/* Print nongeneric type expression */
static void putType(Cell t, int prec, int fr) {
    switch (whatIs(t)) {
    case TYCON:
        putModule(tyconRec(t)->mod);
        putStr(textToStr(tyconRec(t)->text));
        break;

    case TUPLE: {
        int n = tupleOf(t);
        putChr('(');
        while (--n > 0)
            putChr(',');
        putChr(')');
    } break;

    case POLYTYPE: {
        Kinds ks = polySigOf(t);
        OPEN(prec >= ARROW_PREC);
        putStr("forall ");
        for (; isAp(ks); ks = tl(ks)) {
            putTyVar(fr++);
            if (isAp(tl(ks)))
                putChr(' ');
        }
        putStr(". ");
        putType(monotypeOf(t), NEVER, fr);
        CLOSE(prec >= ARROW_PREC);
    } break;

    case CDICTS:
    case QUAL:
        OPEN(prec >= ARROW_PREC);
        if (whatIs(snd(snd(t))) == CDICTS) {
            putContext(fst(snd(t)), fst(snd(snd(snd(t)))), fr);
            putStr(" => ");
            putType(snd(snd(snd(snd(t)))), NEVER, fr);
        } else {
            putContext(fst(snd(t)), NIL, fr);
            putStr(" => ");
            putType(snd(snd(t)), NEVER, fr);
        }
        CLOSE(prec >= ARROW_PREC);
        break;

    case EXIST:
    case RANK2:
        putType(snd(snd(t)), prec, fr);
        break;

    case OFFSET:
        putTyVar(offsetOf(t));
        break;

    case VARIDCELL:
    case VAROPCELL:
        putChr('_');
        unlexVar(textOf(t));
        break;

    case INTCELL:
        putChr('_');
        putInt(intOf(t));
        break;

    case AP: {
        Cell typeHead = getHead(t);
        bool brackets = (argCount != 0 && prec >= ALWAYS);
        int args = argCount;

        if (typeHead == typeList) {
            if (argCount == 1) {
                putChr('[');
                putType(arg(t), NEVER, fr);
                putChr(']');
                return;
            }
        } else if (typeHead == typeArrow) {
            if (argCount == 2) {
                OPEN(prec >= ARROW_PREC);
                putType(arg(fun(t)), ARROW_PREC, fr);
                putStr(" -> ");
                putType(arg(t), NEVER, fr);
                CLOSE(prec >= ARROW_PREC);
                return;
            }
            /*
                                else if (argCount==1) {
                                    putChr('(');
                                    putType(arg(t),ARROW_PREC,fr);
                                    putStr("->)");
                                    return;
                                }
*/
        } else if (isTuple(typeHead)) {
            if (argCount == tupleOf(typeHead)) {
                putChr('(');
                putTupleType(t, fr);
                putChr(')');
                return;
            }
        }
        OPEN(brackets);
        putApType(t, args, fr);
        CLOSE(brackets);
    } break;

    default:
        putStr("(bad type)");
    }
}

/* Print type variable */
static void putTyVar(int n) {
    static char *alphabet /* for the benefit of EBCDIC :-)   */
        = "abcdefghijklmnopqrstuvwxyz";
    putChr(alphabet[n % 26]);
    if (n /= 26) /* just in case there are > 26 vars*/
        putInt(n);
}

/* Print tuple of types, returning true if something was printed, false
 * otherwise; used to control printing of intermed. commas */
static bool putTupleType(Cell e, int fr) {
    if (isAp(e)) {
        if (putTupleType(fun(e), fr))
            putChr(',');
        putType(arg(e), NEVER, fr);
        return true;
    }
    return false;
}

/* Print type application */
static void putApType(Cell t, int n, int fr) {
    if (n > 0) {
        putApType(fun(t), n - 1, fr);
        putChr(' ');
        putType(arg(t), ALWAYS, fr);
    } else
        putType(t, ALWAYS, fr);
}

/* --------------------------------------------------------------------------
 * Print kind expression:
 * ------------------------------------------------------------------------*/

/* Print kind expression */
static void putKind(Kind k) {
    switch (whatIs(k)) {
    case AP:
        if (isAp(fst(k))) {
            putChr('(');
            putKind(fst(k));
            putChr(')');
        } else
            putKind(fst(k));
        if (whatIs(snd(k)) != NIL) {
            putStr(" -> ");
            putKind(snd(k));
        }
        break;

    case STAR:
        putChr('*');
        break;

    case OFFSET:
        putTyVar(offsetOf(k));
        break;

    case INTCELL:
        putChr('_');
        putInt(intOf(k));
        break;

    default:
        putStr("(bad kind)");
    }
}

/* Print list of kinds */
static void putKinds(Kinds ks) {
    if (isNull(ks))
        putStr("()");
    else if (nonNull(tl(ks))) {
        putChr('(');
        putKind(hd(ks));
        while (nonNull(ks = tl(ks))) {
            putChr(',');
            putKind(hd(ks));
        }
        putChr(')');
    } else
        putKind(hd(ks));
}

/* --------------------------------------------------------------------------
 * Print observations
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Print qualified module name (if wanted):
 * ------------------------------------------------------------------------*/
bool useQualifiedNames = false;

/* Print module qualifier */
static void putModule(Module m) {
    if (useQualifiedNames &&
        !isPrelude(m)) { /* leave out "Prelude." qualifiers, too noisy. */
        putStr(textToStr(moduleRec(m)->text));
        putChr('.');
    }
}
/* --------------------------------------------------------------------------
 * Main drivers:
 * ------------------------------------------------------------------------*/

/* Print expr on specified stream */
void printExp(FILE *fp, Cell e) {
    outputStream = fp;
    putDepth = 0;
    put(NEVER, e);
}

/* Print type on specified stream */
void printType(FILE *fp, Cell t) {
    outputStream = fp;
    putSigType(t);
}

/* Print context on spec. stream */
void printContext(FILE *fp, List qs) {
    outputStream = fp;
    putContext(qs, NIL, 0);
}

/* Print predicate pi on stream */
void printPred(FILE *fp, Cell pi) {
    outputStream = fp;
    putPred(pi, 0);
}

/* Print kind k on stream */
void printKind(FILE *fp, Kind k) {
    outputStream = fp;
    putKind(k);
}

/* Print list of kinds on stream */
void printKinds(FILE *fp, Kinds ks) {
    outputStream = fp;
    putKinds(ks);
}

/* Print functional dependency */
void printFD(FILE *fp, Pair fd) {
    List us;
    outputStream = fp;
    for (us = fst(fd); nonNull(us); us = tl(us)) {
        putTyVar(offsetOf(hd(us)));
        if (nonNull(tl(us))) {
            putChr(' ');
        }
    }
    putStr(" -> ");
    for (us = snd(fd); nonNull(us); us = tl(us)) {
        putTyVar(offsetOf(hd(us)));
        if (nonNull(tl(us))) {
            putChr(' ');
        }
    }
}

/*-------------------------------------------------------------------------*/
