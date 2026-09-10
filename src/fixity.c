/* --------------------------------------------------------------------------
 * Resolving infix expressions into trees.
 *
 * The parser cannot build the tree for an expression written with infix
 * operators, because the fixities may not be known until the whole script
 * has been read.  It produces a flat INFIX cell instead, and this file
 * turns that into an ordinary application tree once the fixities are in.
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
#include "pattern.h"
#include "depend.h"
#include "fixity.h"

static Pair local attachFixity(Int, Cell);

/* --------------------------------------------------------------------------
 * Dealing with infix operators:
 *
 * Expressions involving infix operators or unary minus are parsed as
 * elements of the following type:
 *
 *     data InfixExp = Only Exp | Neg InfixExp | Infix InfixExp Op Exp
 *
 * (The algorithms here do not assume that negation can be applied only once,
 * i.e., that - - x is a syntax error, as required by the Haskell report.
 * Instead, that restriction is captured by the grammar itself, given above.)
 *
 * There are rules of precedence and grouping, expressed by two functions:
 *
 *     prec :: Op -> Int;   assoc :: Op -> Assoc    (Assoc = {L, N, R})
 *
 * InfixExp values are rearranged accordingly when a complete expression
 * has been read using a simple shift-reduce parser whose result may be taken
 * to be a value of the following type:
 *
 *     data Exp = Atom Int | Negate Exp | Apply Op Exp Exp | Error String
 *
 * The machine on which this parser is based can be defined as follows:
 *
 *     tidy                         :: InfixExp -> [(Op,Exp)] -> Exp
 *     tidy (Only a)      []         = a
 *     tidy (Only a)      ((o,b):ss) = tidy (Only (Apply o a b)) ss
 *     tidy (Infix a o b) []         = tidy a [(o,b)]
 *     tidy (Infix a o b) ((p,c):ss)
 *                      | shift  o p = tidy a ((o,b):(p,c):ss)
 *                      | red    o p = tidy (Infix a o (Apply p b c)) ss
 *                      | ambig  o p = Error "ambiguous use of operators"
 *     tidy (Neg e)       []         = tidy (tidyNeg e) []
 *     tidy (Neg e)       ((o,b):ss)
 *                      | nshift o   = tidy (Neg (underNeg o b e)) ss
 *                      | nred   o   = tidy (tidyNeg e) ((o,b):ss)
 *                      | nambig o   = Error "illegal use of negation"
 *
 * At each stage, the parser can either shift, reduce, accept, or error.
 * The transitions when dealing with juxtaposed operators o and p are
 * determined by the following rules:
 *
 *     shift o p  = (prec o > prec p)
 *               || (prec o == prec p && assoc o == L && assoc p == L)
 *
 *     red o p    = (prec o < prec p)
 *               || (prec o == prec p && assoc o == R && assoc p == R)
 *
 *     ambig o p  = (prec o == prec p)
 *               && (assoc o == N || assoc p == N || assoc o /= assoc p)
 *
 * The transitions when dealing with juxtaposed unary minus and infix
 * operators are as follows.  The precedence of unary minus (infixl 6) is
 * hardwired in to these definitions, as it is to the definitions of the
 * Haskell grammar in the official report.
 *
 *     nshift o   = (prec o > 6)
 *     nred   o   = (prec o < 6) || (prec o == 6 && assoc o == L)
 *     nambig o   = prec o == 6 && (assoc o == R || assoc o == N)
 *
 * An InfixExp of the form (Neg e) means negate the last thing in
 * the InfixExp e; we can force this negation using:
 *
 *     tidyNeg              :: OpExp -> OpExp
 *     tidyNeg (Only e)      = Only (Negate e)
 *     tidyNeg (Infix a o b) = Infix a o (Negate b)
 *     tidyNeg (Neg e)       = tidyNeg (tidyNeg e)
 *
 * On the other hand, if we want to sneak application of an infix operator
 * under a negation, then we use:
 *
 *     underNeg                  :: Op -> Exp -> OpExp -> OpExp
 *     underNeg o b (Only e)      = Only (Apply o e b)
 *     underNeg o b (Neg e)       = Neg (underNeg o b e)
 *     underNeg o b (Infix e p f) = Infix e p (Apply o f b)
 *
 * As a concession to efficiency, we lower the number of calls to syntaxOf
 * by keeping track of the values of sye, sys throughout the process.  The
 * value APPLIC is used to indicate that the syntax value is unknown.
 * ------------------------------------------------------------------------*/

Cell tidyInfix(Int line, Cell e) /* Convert infixExp to Exp     */
{                                /* :: OpExp                    */
    Cell s = NIL;                /* :: [(Op,Exp)]               */
    Syntax sye = APPLIC;         /* Syntax of op in e (init unknown)*/
    Syntax sys = APPLIC;         /* Syntax of op in s (init unknown)*/
    Cell d = e;

    while (fst(d) != ONLY) { /* Attach fixities to operators    */
        if (fst(d) == NEG) {
            d = snd(d);
        } else {
            fun(fun(d)) = attachFixity(line, fun(fun(d)));
            d = arg(fun(d));
        }
    }

    for (;;)
        switch (whatIs(e)) {
        case ONLY:
            e = snd(e);
            while (nonNull(s)) {
                Cell next = arg(fun(s));
                arg(fun(s)) = e;
                fun(fun(s)) = snd(snd(fun(fun(s))));
                e = s;
                s = next;
            }
            return e;

        case NEG:
            if (nonNull(s)) {
                if (sys == APPLIC) { /* calculate sys      */
                    sys = intOf(fst(fun(fun(s))));
                }

                if (precOf(sys) == UMINUS_PREC && /* nambig  */
                    assocOf(sys) != UMINUS_ASSOC) {
                    errHead(line);
                    errPrintf("Ambiguous use of unary minus with \"");
                    errExpr(fst(snd(fun(fun(s)))));
                    errPrintf("\"");
                    errFail();
                }

                if (precOf(sys) > UMINUS_PREC) { /* nshift  */
                    Cell e1 = snd(e);
                    Cell t = s;
                    s = arg(fun(s));
                    while (whatIs(e1) == NEG)
                        e1 = snd(e1);
                    arg(fun(t)) = arg(e1);
                    fun(fun(t)) = snd(snd(fun(fun(t))));
                    arg(e1) = t;
                    sys = APPLIC;
                    continue;
                }
            }

            /* Intentional fall-thru for nreduce and isNull(s) */

            {
                Cell prev = e; /* e := tidyNeg e  */
                Cell temp = arg(prev);
                Int nneg = 1;
                for (; whatIs(temp) == NEG; nneg++) {
                    fun(prev) = nameNegate;
                    prev = temp;
                    temp = arg(prev);
                }
                if (isInt(arg(temp))) { /* special cases   */
                    if (nneg & 1)       /* for literals    */
                        arg(temp) = mkInt(-intOf(arg(temp)));
                } else if (isBignum(arg(temp))) {
                    if (nneg & 1)
                        arg(temp) = bigNeg(arg(temp));
                } else if (isDouble(arg(temp))) {
                    if (nneg & 1)
                        arg(temp) = mkDouble(-doubleOf(arg(temp)));
                } else {
                    fun(prev) = nameNegate;
                    arg(prev) = arg(temp);
                    arg(temp) = e;
                }
                e = temp;
            }
            continue;

        default:
            if (isNull(s)) { /* Move operation onto empty stack */
                Cell next = arg(fun(e));
                s = e;
                arg(fun(s)) = NIL;
                e = next;
                sys = sye;
                sye = APPLIC;
            } else { /* deal with pair of operators        */

                if (sye == APPLIC) { /* calculate sys and sye   */
                    sye = intOf(fst(fun(fun(e))));
                }
                if (sys == APPLIC) {
                    sys = intOf(fst(fun(fun(s))));
                }

                if (precOf(sye) == precOf(sys) && /* ambig   */
                    (assocOf(sye) != assocOf(sys) || assocOf(sye) == NON_ASS)) {
                    errHead(line);
                    errPrintf("Ambiguous use of operator \"");
                    errExpr(fst(snd(fun(fun(e)))));
                    errPrintf("\" with \"");
                    errExpr(fst(snd(fun(fun(s)))));
                    errPrintf("\"");
                    errFail();
                }

                if (precOf(sye) > precOf(sys) || /* shift   */
                    (precOf(sye) == precOf(sys) && assocOf(sye) == LEFT_ASS &&
                     assocOf(sys) == LEFT_ASS)) {
                    Cell next = arg(fun(e));
                    arg(fun(e)) = s;
                    s = e;
                    e = next;
                    sys = sye;
                    sye = APPLIC;
                } else { /* reduce  */
                    Cell next = arg(fun(s));
                    arg(fun(s)) = arg(e);
                    fun(fun(s)) = snd(snd(fun(fun(s))));
                    arg(e) = s;
                    s = next;
                    sys = APPLIC;
                    /* sye unchanged */
                }
            }
            continue;
        }
}

static Pair local attachFixity(Int line,
                               Cell op) /* Attach fixity to operator in an */
                                        /* infix expression                */
{
    Syntax sy = DEF_OPSYNTAX;
    Cell trop = op;

    switch (whatIs(op)) {
    case VAROPCELL:
    case VARIDCELL:
        if ((sy = lookupSyntax(textOf(op))) == NO_SYNTAX) {
            Name n = findName(textOf(op));
            if (isNull(n)) {
                errHead(line);
                errPrintf("Undefined variable \"%s\"", textToStr(textOf(op)));
                errFail();
            }
            sy = syntaxOf(n);
            trop = n;
        }
        break;

    case CONOPCELL:
    case CONIDCELL:
        sy = syntaxOf(trop = conDefined(line, op, FALSE));
        break;

    case QUALIDENT: {
        Name n = findQualName(op);
        if (nonNull(n)) {
            trop = n;
            sy = syntaxOf(n);
        } else {
            errHead(line);
            errPrintf("Undefined qualified variable \"%s\"", identToStr(op));
            errFail();
        }
    } break;
    default:
        break;
    }
    if (sy == APPLIC) {
        sy = DEF_OPSYNTAX;
    }
    return pair(mkInt((Int)sy),
                pair(trop, op)); /* Pair fixity with (possibly) */
                                 /* translated operator       */
}

/*-------------------------------------------------------------------------*/
