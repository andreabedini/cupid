/* --------------------------------------------------------------------------
 * Primitive functions, input output etc...
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
#include "char.h"
#include <math.h>
#include <stdint.h> /* intptr_t, for the StablePtr conversions */

#if HAVE_IO_H
# include <io.h>
#endif

/* Header files needed to compile the IO primitives */

#include <sys/types.h>
#include <sys/stat.h>

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_SYS_TIMES_H && !mingw32_HOST_OS
# include <sys/times.h>
#endif

#if HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#if HAVE_SYS_RESOURCE_H && !mingw32_HOST_OS
# include <sys/resource.h>
#endif

#if HAVE_SYS_TIMEB_H
# include <sys/timeb.h>
#endif

#if HAVE_WINDOWS_H
# include <windows.h>
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
#endif

#if HAVE_DIRECT_H
# include <direct.h>
#endif

#include <fcntl.h>

#include "builtin.h"

Name nameNegate, nameFlip; /* primitives reqd for parsing     */
Name nameFrom, nameFromThen;
Name nameFromTo, nameFromThenTo;
Name nameFatbar, nameFail; /* primitives reqd for translation */
Name nameIf, nameSel;
Name nameId, nameOtherwise;
Name nameConCmp, nameEnRange; /* primitives used for deriv inst  */
Name nameEnIndex, nameEnInRng;
Name nameEnToEn, nameEnFrEn;
Name nameEnFrom, nameEnFrTh;
Name nameEnFrTo;
Name nameBlackHole;         /* for GC-detected black hole      */
Name nameInd;               /* for dict indirection                */
Name namePrint, nameNPrint; /* primitives for printing         */

Name nameFst, nameSnd;     /* 2-tuple selector functions      */
Name nameAnd, nameOr;      /* built-in logical connectives    */
Name namePrimThrow;        /* throw primitive function        */
Name nameComp;             /* function composition            */
Name nameApp;              /* list append                     */
Name nameShowField;        /* display single field            */
Name nameShowParen;        /* wrap with parens                */
Name nameReadField;        /* read single field               */
Name nameReadParen;        /* unwrap from parens              */
Name nameLex;              /* lexer                           */
Name nameRangeSize;        /* calculate size of index range   */
Name nameCompAux;          /* auxiliary function for compares */
Name namePmInt, namePmFlt; /* primitives for pattern matching */
Name nameReturnIO;
Name namePmInteger;

Name nameRationalToFloat;
Name nameRationalToDouble;
Name nameFloatToRational;
Name nameDoubleToRational;
Name nameDoubleToRatio;
Name nameIntToRatio;
Name nameIntToFloat;
Name nameIntToDouble;
Name nameDoubleToFloat;
Name nameFloatToDouble;

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

PROTO_PRIM(primFatbar);
PROTO_PRIM(primFail);
PROTO_PRIM(primCatchError);
PROTO_PRIM(primThrowException);
PROTO_PRIM(primCatchException);
PROTO_PRIM(primBlackHole);
PROTO_PRIM(primIndirect);
PROTO_PRIM(primSel);
PROTO_PRIM(primIf);
PROTO_PRIM(primStrict);
PROTO_PRIM(primSeq);
PROTO_PRIM(primConCmp);
PROTO_PRIM(primEnRange);
PROTO_PRIM(primEnIndex);
PROTO_PRIM(primEnInRng);
PROTO_PRIM(primEnFrEn);
PROTO_PRIM(primEnToEn);
PROTO_PRIM(primEnFrom);
PROTO_PRIM(primEnFrTh);
PROTO_PRIM(primEnFrTo);

PROTO_PRIM(primMinInt);
PROTO_PRIM(primMaxInt);
PROTO_PRIM(primPlusInt);
PROTO_PRIM(primMinusInt);
PROTO_PRIM(primMulInt);
PROTO_PRIM(primDivInt);
PROTO_PRIM(primQuotInt);
PROTO_PRIM(primModInt);
PROTO_PRIM(primRemInt);
PROTO_PRIM(primQrmInt);
PROTO_PRIM(primNegInt);

PROTO_PRIM(primAndInt);
PROTO_PRIM(primOrInt);
PROTO_PRIM(primXorInt);
PROTO_PRIM(primComplementInt);
PROTO_PRIM(primShiftInt);
PROTO_PRIM(primBitInt);
PROTO_PRIM(primTestInt);

PROTO_PRIM(primCharToInt);
PROTO_PRIM(primIntToChar);
PROTO_PRIM(primWordToInt);
PROTO_PRIM(primIntToWord);
PROTO_PRIM(primIntToFloat);
PROTO_PRIM(primIntToDouble);
PROTO_PRIM(primDummyCvt);

PROTO_PRIM(primRationalToFloat);
PROTO_PRIM(primRationalToDouble);

PROTO_PRIM(primMaxWord);
PROTO_PRIM(primPlusWord);
PROTO_PRIM(primMinusWord);
PROTO_PRIM(primNegateWord);
PROTO_PRIM(primMulWord);
PROTO_PRIM(primQuotWord); /* also used, unsigned division being     */
/* truncating either way, for primDivWord */
PROTO_PRIM(primRemWord); /* also used, for the same reason, for    */
/* primModWord                            */
PROTO_PRIM(primQrmWord);

PROTO_PRIM(primAndWord);
PROTO_PRIM(primOrWord);
PROTO_PRIM(primXorWord);
PROTO_PRIM(primComplementWord);
PROTO_PRIM(primShiftWord);
PROTO_PRIM(primRotateWord);
PROTO_PRIM(primBitWord);
PROTO_PRIM(primTestWord);

PROTO_PRIM(primItoI8);
PROTO_PRIM(primItoI16);
PROTO_PRIM(primItoI32);

PROTO_PRIM(primI8toI);
PROTO_PRIM(primI16toI);
PROTO_PRIM(primI32toI);

PROTO_PRIM(primI32toI64);
PROTO_PRIM(primI64toI32); /* also used, the two representations     */
/* being identical, for primW64toW32      */

PROTO_PRIM(primWtoW8);
PROTO_PRIM(primWtoW16);
PROTO_PRIM(primWtoW32);

PROTO_PRIM(primW8toW);
PROTO_PRIM(primW16toW);
PROTO_PRIM(primW32toW);

PROTO_PRIM(primW32toW64);

PROTO_PRIM(primFreeHFunPtr);

PROTO_PRIM(primDoubleToFloat);
PROTO_PRIM(primFloatToDouble);

PROTO_PRIM(primPlusFloat);
PROTO_PRIM(primMinusFloat);
PROTO_PRIM(primMulFloat);
PROTO_PRIM(primDivFloat);
PROTO_PRIM(primNegFloat);

PROTO_PRIM(primPlusDouble);
PROTO_PRIM(primMinusDouble);
PROTO_PRIM(primMulDouble);
PROTO_PRIM(primDivDouble);
PROTO_PRIM(primNegDouble);

PROTO_PRIM(primSinFloat);
PROTO_PRIM(primCosFloat);
PROTO_PRIM(primTanFloat);
PROTO_PRIM(primAsinFloat);
PROTO_PRIM(primAcosFloat);
PROTO_PRIM(primAtanFloat);
PROTO_PRIM(primExpFloat);
PROTO_PRIM(primLogFloat);
PROTO_PRIM(primSqrtFloat);
PROTO_PRIM(primFloatToInt);
PROTO_PRIM(primFloatDigits);
PROTO_PRIM(primFloatMinExp);
PROTO_PRIM(primFloatMaxExp);
PROTO_PRIM(primFloatDecode);
PROTO_PRIM(primFloatEncode);

PROTO_PRIM(primSinDouble);
PROTO_PRIM(primCosDouble);
PROTO_PRIM(primTanDouble);
PROTO_PRIM(primAsinDouble);
PROTO_PRIM(primAcosDouble);
PROTO_PRIM(primAtanDouble);
PROTO_PRIM(primExpDouble);
PROTO_PRIM(primLogDouble);
PROTO_PRIM(primSqrtDouble);
PROTO_PRIM(primDoubleToInt);
PROTO_PRIM(primDoubleDigits);
PROTO_PRIM(primDoubleMinExp);
PROTO_PRIM(primDoubleMaxExp);
PROTO_PRIM(primDoubleDecode);
PROTO_PRIM(primDoubleEncode);

PROTO_PRIM(primNullPtr);
PROTO_PRIM(primPlusPtr);
PROTO_PRIM(primAlignPtr);
PROTO_PRIM(primMinusPtr);
PROTO_PRIM(primEqPtr);
PROTO_PRIM(primCmpPtr);

PROTO_PRIM(primEqInt);
PROTO_PRIM(primCmpInt);

PROTO_PRIM(primEqWord);
PROTO_PRIM(primCmpWord);

PROTO_PRIM(primEqChar);
PROTO_PRIM(primCmpChar);

PROTO_PRIM(primEqFloat);
PROTO_PRIM(primCmpFloat);

PROTO_PRIM(primEqDouble);
PROTO_PRIM(primCmpDouble);

PROTO_PRIM(primMaxChar);
PROTO_PRIM(primIsUpper);
PROTO_PRIM(primIsLower);
PROTO_PRIM(primIsAlpha);
PROTO_PRIM(primIsAlphaNum);
PROTO_PRIM(primIsPrint);
PROTO_PRIM(primToUpper);
PROTO_PRIM(primToLower);
PROTO_PRIM(primToTitle);
PROTO_PRIM(primUniGenCat);

PROTO_PRIM(primUnsafePtrEq);
PROTO_PRIM(primUnsafePtrToInt);

/* --------------------------------------------------------------------------
 * Table of primitive/built-in values:
 * ------------------------------------------------------------------------*/

static struct primitive builtinPrimTable[] = {
    {"fatbar", 2, primFatbar},
    {"fail", 0, primFail},
    {"catchError", 1, primCatchError},
    {"primThrowException", 1, primThrowException},
    {"primCatchException", 1, primCatchException},
    {"gcBhole", 0, primBlackHole},
    {"dictIndirect", 1, primIndirect},
    {"sel", 3, primSel},
    {"if", 3, primIf},
    {"conCmp", 2, primConCmp},
    {"enRange", 1, primEnRange},
    {"enIndex", 2, primEnIndex},
    {"enInRng", 2, primEnInRng},
    {"enToEn", 2, primEnToEn},
    {"enFrEn", 1, primEnFrEn},
    {"enFrom", 1, primEnFrom},
    {"enFrTh", 2, primEnFrTh},
    {"enFrTo", 2, primEnFrTo},

    {"primMinInt", 0, primMinInt},
    {"primMaxInt", 0, primMaxInt},
    {"primPlusInt", 2, primPlusInt},
    {"primMinusInt", 2, primMinusInt},
    {"primMulInt", 2, primMulInt},
    {"primDivInt", 2, primDivInt},
    {"primQuotInt", 2, primQuotInt},
    {"primModInt", 2, primModInt},
    {"primRemInt", 2, primRemInt},
    {"primNegInt", 1, primNegInt},
    {"primQrmInt", 2, primQrmInt},

    {"primAndInt", 2, primAndInt},
    {"primOrInt", 2, primOrInt},
    {"primXorInt", 2, primXorInt},
    {"primComplementInt", 1, primComplementInt},
    {"primShiftInt", 2, primShiftInt},
    {"primBitInt", 1, primBitInt},
    {"primTestInt", 2, primTestInt},

    {"primMaxWord", 0, primMaxWord},
    {"primPlusWord", 2, primPlusWord},
    {"primMinusWord", 2, primMinusWord},
    {"primNegateWord", 1, primNegateWord},
    {"primMulWord", 2, primMulWord},
    {"primDivWord", 2, primQuotWord}, /* unsigned x/y truncates either way */
    {"primQuotWord", 2, primQuotWord},
    {"primModWord", 2, primRemWord}, /* unsigned x%y is the same either way */
    {"primRemWord", 2, primRemWord},
    {"primQrmWord", 2, primQrmWord},

    {"primAndWord", 2, primAndWord},
    {"primOrWord", 2, primOrWord},
    {"primXorWord", 2, primXorWord},
    {"primComplementWord", 1, primComplementWord},
    {"primShiftWord", 2, primShiftWord},
    {"primRotateWord", 3, primRotateWord},
    {"primBitWord", 1, primBitWord},
    {"primTestWord", 2, primTestWord},

    {"primIntToInt8", 1, primItoI8},
    {"primIntToInt16", 1, primItoI16},
    {"primIntToInt32", 1, primItoI32},

    {"primInt8ToInt", 1, primI8toI},
    {"primInt16ToInt", 1, primI16toI},
    {"primInt32ToInt", 1, primI32toI},

    {"primInt32ToInt64", 2, primI32toI64},
    {"primInt64ToInt32", 1, primI64toI32},

    {"primWordToWord8", 1, primWtoW8},
    {"primWordToWord16", 1, primWtoW16},
    {"primWordToWord32", 1, primWtoW32},

    {"primWord8ToWord", 1, primW8toW},
    {"primWord16ToWord", 1, primW16toW},
    {"primWord32ToWord", 1, primW32toW},

    {"primWord32ToWord64", 2, primW32toW64},
    {"primWord64ToWord32", 1, primI64toI32}, /* splitting a 64-bit pair cell */
                                             /* into 32-bit halves doesn't    */
                                             /* depend on signedness          */

    {"freeHaskellFunPtr", 1 + IOArity, primFreeHFunPtr},

    {"primPlusFloat", 2, primPlusFloat},
    {"primMinusFloat", 2, primMinusFloat},
    {"primMulFloat", 2, primMulFloat},
    {"primDivFloat", 2, primDivFloat},
    {"primNegFloat", 1, primNegFloat},

    {"primPlusDouble", 2, primPlusDouble},
    {"primMinusDouble", 2, primMinusDouble},
    {"primMulDouble", 2, primMulDouble},
    {"primDivDouble", 2, primDivDouble},
    {"primNegDouble", 1, primNegDouble},

    {"primSinFloat", 1, primSinFloat},
    {"primCosFloat", 1, primCosFloat},
    {"primTanFloat", 1, primTanFloat},
    {"primAsinFloat", 1, primAsinFloat},
    {"primAcosFloat", 1, primAcosFloat},
    {"primAtanFloat", 1, primAtanFloat},
    {"primExpFloat", 1, primExpFloat},
    {"primLogFloat", 1, primLogFloat},
    {"primSqrtFloat", 1, primSqrtFloat},
    {"primFloatToInt", 1, primFloatToInt},
    {"primFloatDigits", 0, primFloatDigits},
    {"primFloatMinExp", 0, primFloatMinExp},
    {"primFloatMaxExp", 0, primFloatMaxExp},
    {"primFloatDecode", 1, primFloatDecode},
    {"primFloatEncode", 2, primFloatEncode},

    {"primSinDouble", 1, primSinDouble},
    {"primCosDouble", 1, primCosDouble},
    {"primTanDouble", 1, primTanDouble},
    {"primAsinDouble", 1, primAsinDouble},
    {"primAcosDouble", 1, primAcosDouble},
    {"primAtanDouble", 1, primAtanDouble},
    {"primExpDouble", 1, primExpDouble},
    {"primLogDouble", 1, primLogDouble},
    {"primSqrtDouble", 1, primSqrtDouble},
    {"primDoubleToInt", 1, primDoubleToInt},
    {"primDoubleDigits", 0, primDoubleDigits},
    {"primDoubleMinExp", 0, primDoubleMinExp},
    {"primDoubleMaxExp", 0, primDoubleMaxExp},
    {"primDoubleDecode", 1, primDoubleDecode},
    {"primDoubleEncode", 2, primDoubleEncode},

    {"primIntToChar", 1, primIntToChar},
    {"primCharToInt", 1, primCharToInt},
    {"primIntToWord", 1, primIntToWord},
    {"primWordToInt", 1, primWordToInt},
    {"primIntToFloat", 1, primIntToFloat},
    {"primIntToDouble", 1, primIntToDouble},
    {"primDoubleToFloat", 1, primDoubleToFloat},
    {"primFloatToDouble", 1, primFloatToDouble},

    {"primRationalToFloat", 1, primRationalToFloat},
    {"primRationalToDouble", 1, primRationalToDouble},

    {"nullPtr", 0, primNullPtr},
    {"plusPtr", 2, primPlusPtr},
    {"alignPtr", 2, primAlignPtr},
    {"minusPtr", 2, primMinusPtr},
    {"primEqPtr", 2, primEqPtr},
    {"primCmpPtr", 2, primCmpPtr},

    {"primEqInt", 2, primEqInt},
    {"primCmpInt", 2, primCmpInt},
    {"primEqWord", 2, primEqWord},
    {"primCmpWord", 2, primCmpWord},
    {"primEqChar", 2, primEqChar},
    {"primCmpChar", 2, primCmpChar},
    {"primEqFloat", 2, primEqFloat},
    {"primCmpFloat", 2, primCmpFloat},
    {"primEqDouble", 2, primEqDouble},
    {"primCmpDouble", 2, primCmpDouble},

    {"primUnsafeCoerce", 1, primDummyCvt}, /* breaks the type system */

    {"strict", 2, primStrict},
    {"seq", 2, primSeq},

    {"primMaxChar", 0, primMaxChar},
    {"isUpper", 1, primIsUpper},
    {"isLower", 1, primIsLower},
    {"isAlpha", 1, primIsAlpha},
    {"isAlphaNum", 1, primIsAlphaNum},
    {"isPrint", 1, primIsPrint},
    {"toUpper", 1, primToUpper},
    {"toLower", 1, primToLower},
    {"toTitle", 1, primToTitle},
    {"primUniGenCat", 1, primUniGenCat},

    {"unsafePtrEq", 2, primUnsafePtrEq},       /* breaks the semantics  */
    {"unsafePtrToInt", 1, primUnsafePtrToInt}, /* breaks the semantics  */

    {0, 0, 0}};

/* --------------------------------------------------------------------------
 * Primitive functions:
 *
 * IMPORTANT NOTICE: the primitive function definitions in this file
 * should be written in a style that permits correct execution *without*
 * conservative garbage collection (i.e., without marking from the C stack).
 * Adding primitive definitions that do not meet this requirement may
 * corrupt the heap and lead to failed execution; do not modify this code
 * unless you are really confident about what you are doing.
 *
 * Some general guidelines follow, using c, e to denote expressions that
 * involve either at most 1 allocation, or the possibility/certainty of
 * multiple allocations, resp.
 *
 * push(c);             Ok.
 * push(e);             Bad -- intermediate result may be lost if GC occurs
 *                      in the middle of building e; break e into steps, and
 *                      use toparg(), topfun(), etc.
 *
 * Cell x = ...;        Safe if value assigned to x will never be an
 * <any code with a     indirection.  (Otherwise, cell assigned to x may
 * possible alloc>      be returned to freeList *before* the value is used.)
 * ... x ...            Probably best avoided in other circumstances.
 *
 * updateRoot(e);       All ok.
 * updapRoot(e,e);
 * updateRoot(mkInt(n));
 * eval(pop());
 *
 * eval(ap(c,pop()));   Bad -- a GC call may corrupt value pop'd off stack.
 *
 * It is also worth a reminder that the fst and snd values passed in any call
 * to the allocator are automatically marked and preserved if a GC is needed.
 * As a result, code like the following is guaranteed to be safe:
 *  return ap(ap(mkTuple(2),ZERONUM),ZERONUM);    (ZERONUM is a constant)
 *  for ( ... )                                   (PROVIDED that ds is the
 *     ds = cons(consChar(c),ds);                  only var that needs GC).
 *
 * If these restrictions are judged to be too onerous in particular cases,
 * temporarily enable conservative GC (and reset it to the original state,
 * either on or off at the beginning of the operation).  See bignums.c
 * for an example.
 *
 * There are also certain conventions that must always be obeyed, regardless
 * of whether conservative GC is in use.  For example:
 *
 * lhs = expr;          If lhs involves an address calculation that may be
 *                      invalidated by a gc, and expr could trigger an alloc,
 *                      then this expression is bad, or at least not portable:
 *                      it will only do the right thing under some evaluation
 *                      orders.  For example:  hd(top()) = ap(..,..) is bad,
 *                      unless you know that top() will never be modified
 *                      during a GC.
 *
 *                      This is no different from the problems that occur
 *                      with non-portable combinations of stack operators
 *                      like push(top());  The solution is also the same:
 *                      use an intermediate variable to make the order
 *                      of evaluation explicit.
 *
 * If this version of Cuddles has been modified to allow different or
 * additional run-time representations for certain values, then the
 * examples and principles illustrated here may need to be reconsidered,
 * and possibly reclassified.  The same will also be true if the execution
 * mechanisms etc., are changed in any way.  (And all this is assuming
 * that the original implementations are correct...)
 * ------------------------------------------------------------------------*/

/* Fatbar primitive */
primFun(primFatbar) {
    Cell temp = evalWithNoError(primArg(2));
    if (nonNull(temp))
        if (temp == nameFail) /* _FAIL [] r = r                  */
            updateRoot(primArg(1));
        else
            throwException(temp);
    else
        updateRoot(primArg(2)); /* l     [] r = l  -- otherwise    */
}

/* Failure primitive */
primFun(primFail) {
    throwException(nameFail);
}

primFun(primBlackHole) {
    throwException(nameNonTermination);
}

primFun(primIndirect) {
    throwException(nameNonTermination);
}

/* Error catching primitive */
primFun(primCatchError) {
    Cell err = NIL;
    err = evalWithNoError(primArg(1)); /*  :: a -> Maybe a                */
    if (isNull(err)) {
        updapRoot(nameJust, primArg(1));
    } else {
        updateRoot(nameNothing);
    }
}

/* Failure primitive */
primFun(primThrowException) {
    throwException(primArg(1)); /*  :: Exception -> a              */
}

/* This function ought to be in the IO monad to preserve referential       */
/* transparency but it has tricky interactions with the concurrency parts  */
/* of the IO monad so we provide it in unsafe form here and make it safe   */
/* in the Prelude.                                                         */

/* Error catching primitive */
primFun(primCatchException) {
    Cell err = NIL; /*  :: a -> Either Exception a      */
    err = evalWithNoError(primArg(1));
    if (isNull(err)) {
        updapRoot(nameRight, primArg(1));
    } else {
        updapRoot(nameLeft, err);
    }
}

/* Component selection */
primFun(primSel) {
    eval(primArg(2));           /* _sel c e n  return nth component*/
    if (whnfHead == primArg(3)) /* in expr e, built with cfun c    */
        updateRoot(pushed(intOf(primArg(1)) - 1));
    else
        internal("primSel");
}

/* Conditional primitive */
primFun(primIf) {
    eval(primArg(3));
    checkBool();
    if (whnfHead == nameTrue)
        updateRoot(primArg(2));
    else
        updateRoot(primArg(1));
}

/* Strict application primitive */
primFun(primStrict) {
    eval(primArg(1));                  /* evaluate 2nd argument */
    updapRoot(primArg(2), primArg(1)); /* and apply 1st argument to result */
}

/* Strict sequencing primitive */
primFun(primSeq) {
    eval(primArg(2));       /* evaluate 1st argument */
    updateRoot(primArg(1)); /* and return the first */
}

/* Compare constructors */
primFun(primConCmp) {
    int l, r; /*  :: a -> a -> Ordering          */
    ConArg(l, 2);
    ConArg(r, 1);
    updateRoot(l < r ? nameLT : (l > r ? nameGT : nameEQ));
}

/* Derived range for enum type */
primFun(primEnRange) {
    eval(primArg(1)); /* :: (a,a) -> [a]                 */
    updapRoot(ap(nameEnFrTo, primArg(3)), primArg(2));
}

/* Derived index for enum type */
primFun(primEnIndex) {
    int l, h, ix; /*  :: (a,a) -> a -> Int           */
    eval(primArg(2));
    ConArg(l, 4);  /* evaluate lower bound            */
    ConArg(h, 3);  /* evaluate upper bound            */
    ConArg(ix, 1); /* evaluate index                  */
    if (l <= ix && ix <= h) {
        IntResult(ix - l);
    } else {
        throwException(ap(nameErrorCall,
                          mkStr(findText("Ix.index: Index out of range."))));
    }
}

/* Derived inRange for enum type */
primFun(primEnInRng) {
    int l, h, ix; /*  :: (a,a) -> a -> Bool          */
    eval(primArg(2));
    ConArg(l, 4);  /* evaluate lower bound            */
    ConArg(h, 3);  /* evaluate upper bound            */
    ConArg(ix, 1); /* evaluate index                  */
    BoolResult(l <= ix && ix <= h);
}

/* Derived toEnum for enum type */
primFun(primEnToEn) {
    Name n; /* :: a -> Int -> a                */
    int i;
    eval(primArg(2));
    checkCon();
    n = whnfHead;
    IntArg(i, 1);
    if (nonNull(n = cfunByNum(n, i)))
        updateRoot(n);
    else
        throwException(
            ap(nameErrorCall, mkStr(findText("toEnum: out of range"))));
}

/* Derived fromEnum for enum type */
primFun(primEnFrEn) {
    int i; /* :: a -> Int                     */
    ConArg(i, 1);
    IntResult(i == 0 ? 0 : (i - 1));
}

/* Derived enumFrom for enum type */
primFun(primEnFrom) {
    Name cfs; /* :: a -> [a]                     */
    eval(primArg(1));
    checkCon();
    cfs = succCfun(whnfHead);
    push(isNull(cfs) ? nameNil : ap(nameEnFrom, cfs));
    updapRoot(ap(nameCons, whnfHead), top());
}

/* Derived enumFromTo for enum type */
primFun(primEnFrTo) {
    Name l, r; /* :: a -> a -> [a]                */
    eval(primArg(2));
    checkCon();
    l = whnfHead;
    eval(primArg(1));
    checkCon();
    r = whnfHead;
    if (cfunOf(l) < cfunOf(r)) {
        push(ap(nameEnFrTo, succCfun(l)));
        updapRoot(ap(nameCons, l), ap(top(), whnfHead));
    } else if (l == r) {
        updapRoot(ap(nameCons, l), nameNil);
    } else {
        updateRoot(nameNil);
    }
}

/* Derived enumFromThen for enum ty */
primFun(primEnFrTh) {
    Name f, n; /* :: a -> a -> [a]                */
    eval(primArg(2));
    checkCon();
    f = whnfHead;
    eval(primArg(1));
    checkCon();
    n = nextCfun(f, whnfHead);
    if (isNull(n)) {
        push(ap(nameCons, whnfHead));
        toparg(nameNil);
    } else {
        push(ap(nameEnFrTh, whnfHead));
        toparg(n);
    }
    updapRoot(ap(nameCons, f), top());
}

/* --------------------------------------------------------------------------
 * Integer arithmetic primitives:
 * ------------------------------------------------------------------------*/

/* Signed integer overflow is undefined behaviour in C, but Haskell's Int
 * is defined to wrap silently (Int is a fixed-width type).  Do the
 * arithmetic in Unsigned, which wraps modulo 2^bitsize by definition, and
 * convert the bit pattern back, instead of letting +, -, * and negate
 * overflow signed int directly.                                          */
static int wrapAdd(int x, int y) {
    return (int)((unsigned)x + (unsigned)y);
}
static int wrapSub(int x, int y) {
    return (int)((unsigned)x - (unsigned)y);
}
static int wrapMul(int x, int y) {
    return (int)((unsigned)x * (unsigned)y);
}
static int wrapNeg(int x) {
    return (int)(0u - (unsigned)x);
}

CAFInt(primMinInt, MINNEGINT)     /* minimum integer CAF              */
    CAFInt(primMaxInt, MAXPOSINT) /* maximum integer CAF              */
    IntInt2Int(primPlusInt,
               wrapAdd(x, y)) /* Integer addition primitive       */
    IntInt2Int(primMinusInt,
               wrapSub(x, y))             /* Integer subtraction primitive    */
    IntInt2Int(primMulInt, wrapMul(x, y)) /* Integer multiplication primitive */
    Int2Int(primNegInt, wrapNeg(x))       /* Integer negation primitive       */

    /* quot and rem satisfy:                                                   */
    /*     (x `quot` y)*y + (x `rem` y) == x                                   */
    /* which is exactly the property described in K&R 2:                       */
    /*     (a/b)*b + a%b == a                                                  */
    /*                                                                          */
    /* minBound `quot` (-1) and minBound `quotRem` (-1) overflow (the true     */
    /* quotient, maxBound+1, isn't representable), so raise the same           */
    /* "arithmetic overflow" error used for Integer.  minBound `rem` (-1) is   */
    /* always 0 and doesn't overflow.                                          */

    primFun(primQuotInt) { /* Integer division primitive       */
    int x, y;              /* truncated towards zero           */
    IntArg(x, 2);
    IntArg(y, 1);
    if (y == 0)
        throwException(ap(nameArithException, nameDivideByZero));
    if (x == MINNEGINT && y == -1)
        throwException(ap(nameArithException, nameOverflow));
    IntResult(x / y);
}

/* Integer remainder primitive */
primFun(primRemInt) {
    int x, y;
    IntArg(x, 2);
    IntArg(y, 1);
    if (y == 0)
        throwException(ap(nameArithException, nameDivideByZero));
    IntResult((x == MINNEGINT && y == -1) ? 0 : x % y);
}

/* Integer quotient and remainder truncated towards zero */
primFun(primQrmInt) {
    int x, y;
    IntArg(x, 2);
    IntArg(y, 1);
    if (y == 0)
        throwException(ap(nameArithException, nameDivideByZero));
    if (x == MINNEGINT && y == -1)
        throwException(ap(nameArithException, nameOverflow));
    IntIntResult(x / y, x % y);
}

/* Integer division primitive truncated towards -ve infinity */
primFun(primDivInt) {
    int x, y, r;
    IntArg(x, 2);
    IntArg(y, 1);
    if (y == 0)
        throwException(ap(nameArithException, nameDivideByZero));
    if (x == MINNEGINT && y == -1)
        throwException(ap(nameArithException, nameOverflow));
    r = x % y;
    x = x / y;
    if ((y < 0 && r > 0) || (y > 0 && r < 0))
        x--;
    IntResult(x);
}

/* Integer modulo primitive */
primFun(primModInt) {
    int x, y, r;
    IntArg(x, 2);
    IntArg(y, 1);
    if (y == 0)
        throwException(ap(nameArithException, nameDivideByZero));
    if (x == MINNEGINT && y == -1) {
        IntResult(0);
    } else {
        r = x % y;              /* "... the modulo having the sign  */
        if ((r < 0 && y > 0) || /*              of the divisor ..." */
            (r > 0 && y < 0)) { /* See definition on p.91 of Haskell*/
            IntResult(r + y);   /* report... (Haskell 1.1?)         */
        } else {
            IntResult(r);
        }
    }
}

IntInt2Int(primAndInt, x &y) IntInt2Int(primOrInt, x | y)
    IntInt2Int(primXorInt, (x & ~y) | (~x & y)) Int2Int(primComplementInt, ~x)

        primFun(primBitInt) { /* Data.Bits.bit :: Int -> Int      */
    int x;
    IntArg(x, 1);
    /* 1<<x is undefined for x<0 or x>=bitsize(Int); Haskell's bit gives 0 */
    if (x < 0 || x >= (int)(sizeof(int) * 8)) {
        IntResult(0);
    } else {
        IntResult((int)((unsigned)1 << x));
    }
}

IntInt2Bool(primTestInt, (x >> y) & 1)

    primFun(primShiftInt) {
    int x, y;
    IntArg(x, 2);
    IntArg(y, 1);
    if (y >= 0) {
        /* << isn't defined for y larger than the Int size, and left-      */
        /* shifting a negative Int is undefined behaviour in C; do it in   */
        /* Unsigned instead, which wraps the way Haskell's shiftL does.    */
        IntResult(y >= (int)(sizeof(x) * 8) ? 0 : (int)((unsigned)x << y));
    } else {
        y = -y;
        /* arithmetic right shift of a negative Int by >= the Int size     */
        /* gives -1, not 0 (unlike the unsigned case in primShiftWord)     */
        IntResult(y >= (int)(sizeof(x) * 8) ? (x < 0 ? -1 : 0) : x >> y);
    }
}

/* --------------------------------------------------------------------------
 * Unsigned arithmetic primitives:
 * ------------------------------------------------------------------------*/

CAFWord(primMaxWord, MAXCUDDLESWORD)    /* maximum integer CAF              */
    WordWord2Word(primPlusWord, x + y)  /* Word addition primitive          */
    WordWord2Word(primMinusWord, x - y) /* Word subtraction primitive       */
    Word2Word(primNegateWord, 0u - x)   /* Word negation (modulo MAXWORD)   */
    WordWord2Word(primMulWord, x *y)    /* Word multiplication primitive    */
    WordWord2WordNonZero(primQuotWord,
                         x / y) /* Word division primitive; also    */
                                /* primDivWord, since unsigned x/y  */
                                /* truncates the same way either way*/
    WordWord2WordNonZero(primRemWord,
                         x % y) /* Word remainder primitive; also   */
                                /* primModWord, for the same reason */

    /* quot and rem satisfy:                                                   */
    /*     (x `quot` y)*y + (x `rem` y) == x                                   */
    /* which is exactly the property described in K&R 2:                       */
    /*     (a/b)*b + a%b == a                                                  */

    primFun(primQrmWord) { /* Integer quotient and remainder   */
    unsigned x, y;         /* truncated towards zero           */
    WordArg(x, 2);
    WordArg(y, 1);
    if (y == 0)
        throwException(ap(nameArithException, nameDivideByZero));
    WordWordResult(x / y, x % y);
}

/* Data.Bits.bit :: Int -> Word */
WordWord2Word(primAndWord, x &y) WordWord2Word(primOrWord, x | y)
    WordWord2Word(primXorWord, (x & ~y) | (~x & y))
        Word2Word(primComplementWord, ~x) primFun(primBitWord) {
    int x;
    IntArg(x, 1);
    /* 1<<x is undefined for x<0 or x>=bitsize(Word); Haskell's bit gives 0 */
    if (x < 0 || x >= (int)(sizeof(unsigned) * 8)) {
        WordResult(0);
    } else {
        WordResult((unsigned)1 << x);
    }
}
WordInt2Bool(primTestWord, (x >> y) & 1)

    primFun(primShiftWord) {
    unsigned x;
    int y;
    WordArg(x, 2);
    IntArg(y, 1);
    if (y >= 0) {
        /* << isn't defined for y larger than word size */
        WordResult(y >= (int)(sizeof(x) * 8) ? 0 : x << y);
    } else {
        y = -y;
        WordResult(y >= (int)(sizeof(x) * 8) ? 0 : x >> y);
    }
}

primFun(primRotateWord) {
    unsigned x;
    int y, z;
    WordArg(x, 2);
    IntArg(y, 1);
    IntArg(z, 3);
    y = y % z;
    if (y >= 0) {
        WordResult((x << y) | (x >> (z - y)));
    } else {
        WordResult((x >> (-y)) | (x << (z + y)));
    }
}

Int2Int(primItoI8, x & 0xff) Int2Int(primItoI16, x & 0xffff)
    Int2Int(primItoI32, x & 0xffffffff)

        Int2Int(primI8toI, (int8_t)x) /* casts used to cause sign extension */
    Int2Int(primI16toI, (int16_t)x)   /* casts used to cause sign extension */
    Int2Int(primI32toI, x)

        Word2Word(primWtoW8, x & 0xff) Word2Word(
            primWtoW16, x & 0xffff) Word2Word(primWtoW32, x & 0xffffffff)

            Word2Word(primW8toW, x) Word2Word(primW16toW,
                                              x) Word2Word(primW32toW, x)

                primFun(
                    primI64toI32) { /* also used for primW64toW32: splitting a 64-bit */
    Cell x, y;        /* pair cell into 32-bit halves doesn't depend   */
    eval(primArg(1)); /* on signedness                                 */
    x = fst(snd(whnfHead));
    y = snd(snd(whnfHead));
    updapRoot(ap(mkTuple(2), x), y);
}

primFun(primI32toI64) {
    int x, y;
    IntArg(x, 2);
    IntArg(y, 1);
    updateRoot(pair(I64CELL, pair(mkInt(x), mkInt(y))));
}

primFun(primW32toW64) {
    unsigned x, y;
    WordArg(x, 2);
    WordArg(y, 1);
    updateRoot(pair(I64CELL, pair(mkInt(x), mkInt(y))));
}

/* --------------------------------------------------------------------------
 * Coercion primitives:
 * ------------------------------------------------------------------------*/

Char2Int(primCharToInt, x) /* Character to integer primitive   */

    primFun(primIntToChar) { /* Integer to character primitive   */
    int i;
    IntArg(i, 1);
    if (i < 0 || i > MAXCHARVAL)
        throwException(ap(nameErrorCall, mkStr(findText("chr: out of range"))));
    CharResult(i);
}

/* Word to integer primitive */
primFun(primWordToInt) {
    unsigned x;
    WordArg(x, 1);
    IntResult(x);
}

/* Integer to word primitive */
primFun(primIntToWord) {
    int i;
    IntArg(i, 1);
    WordResult(i);
}

/* Integer to Float primitive */
primFun(primIntToFloat) {
    int i;
    IntArg(i, 1);
    FloatResult((Float)i);
}

/* Integer to Double primitive */
primFun(primIntToDouble) {
    int i;
    IntArg(i, 1);
    DoubleResult((Double)i);
}

/* Dummy (identity) conversion */
primFun(primDummyCvt) {
    updateRoot(primArg(1));
}

primFun(primRationalToFloat) {
    /* Optimisation: we try to short-circuit trivial conversions */
    Cell x = followInd(primArg(1));
    if (isAp(x)) {
        Cell f = followInd(fun(x));
        Cell a = arg(x);
        if (f == nameFloatToRational) {
            updateRoot(a);
            return;
        } else if (f == nameDoubleToRational) {
            updapRoot(nameDoubleToFloat, a);
            return;
        } else if (isAp(f)) {
            Cell g = followInd(fun(f));
            if (g == nameDoubleToRatio) {
                /* ignore the dict - it must be right */
                updapRoot(nameDoubleToFloat, a);
                return;
            } else if (g == nameIntToRatio) {
                updapRoot(nameIntToFloat, a);
                return;
            }
        }
    }
    updapRoot(nameRationalToFloat, primArg(1));
}

primFun(primRationalToDouble) {
    /* Optimisation: we try to short-circuit trivial conversions */
    Cell x = followInd(primArg(1));
    if (isAp(x)) {
        Cell f = followInd(fun(x));
        Cell a = arg(x);
        if (f == nameFloatToRational) {
            updapRoot(nameFloatToDouble, a);
            return;
        } else if (f == nameDoubleToRational) {
            updateRoot(a);
            return;
        } else if (isAp(f)) {
            Cell g = followInd(fun(f));
            if (g == nameDoubleToRatio) {
                updateRoot(a); /* ignore the dict - it must be right */
                return;
            } else if (g == nameIntToRatio) {
                updapRoot(nameIntToDouble, a);
                return;
            }
        }
    }
    updapRoot(nameRationalToDouble, primArg(1));
}

/* --------------------------------------------------------------------------
 * Float arithmetic primitives:
 * ------------------------------------------------------------------------*/

primFun(primFloatToDouble) {
    Float f;
    FloatArg(f, 1);
    DoubleResult((Double)f);
}

primFun(primDoubleToFloat) {
    Double f;
    DoubleArg(f, 1);
    FloatResult((Float)f);
}

FloatFloat2Float(primPlusFloat, x + y) /* Float addition primitive         */
    FloatFloat2Float(primMinusFloat,
                     x - y)               /* Float subtraction primitive      */
    FloatFloat2Float(primMulFloat, x *y)  /* Float multiplication primitive   */
    Float2Float(primNegFloat, -x)         /* Float negation primitive         */
    FloatFloat2Float(primDivFloat, x / y) /* Float division primitive       */

    DoubleDouble2Double(primPlusDouble,
                        x + y) /* Double addition primitive        */
    DoubleDouble2Double(primMinusDouble,
                        x - y) /* Double subtraction primitive    */
    DoubleDouble2Double(primMulDouble,
                        x *y)        /* Double multiplication primitive  */
    Double2Double(primNegDouble, -x) /* Double negation primitive        */
    DoubleDouble2Double(primDivDouble, x / y) /* Double division primitive  */

    Float2Float(primSinFloat, sin(x))   /* Float sin (trig) primitive       */
    Float2Float(primCosFloat, cos(x))   /* Float cos (trig) primitive       */
    Float2Float(primTanFloat, tan(x))   /* Float tan (trig) primitive       */
    Float2Float(primAsinFloat, asin(x)) /* Float arc sin (trig) primitive   */
    Float2Float(primAcosFloat, acos(x)) /* Float arc cos (trig) primitive   */
    Float2Float(primAtanFloat, atan(x)) /* Float arc tan (trig) primitive   */
                                        /*               (trig) primitive   */
    Float2Float(primExpFloat, exp(x))   /* Float exponential primitive      */
    Float2FloatPre(primLogFloat, log(x),
                   x > 0) /* Float logarithm primitive        */
    Float2FloatPre(primSqrtFloat, sqrt(x),
                   x >= 0) /* Float square root primitive  */

    Double2Double(primSinDouble, sin(x)) /* Double sin (trig) primitive      */
    Double2Double(primCosDouble, cos(x)) /* Double cos (trig) primitive      */
    Double2Double(primTanDouble, tan(x)) /* Double tan (trig) primitive      */
    Double2Double(primAsinDouble,
                  asin(x)) /* Double arc sin (trig) primitive  */
    Double2Double(primAcosDouble,
                  acos(x)) /* Double arc cos (trig) primitive  */
    Double2Double(primAtanDouble,
                  atan(x))               /* Double arc tan (trig) primitive  */
    Double2Double(primExpDouble, exp(x)) /* Double exponential primitive     */
    Double2DoublePre(primLogDouble, log(x),
                     x > 0) /* Double logarithm primitive    */
    Double2DoublePre(primSqrtDouble, sqrt(x),
                     x >= 0) /* Double square root primitive */

    /* Not used in Cuddles prelude, rounds towards zero */
    Float2Int(primFloatToInt, (int)x)   /* Adhoc Float --> Int conversion   */
    Double2Int(primDoubleToInt, (int)x) /* Adhoc Double --> Int conversion  */

    CAFInt(primFloatDigits,
           CUDDLES_FLT_MANT_DIG) /* Float sig. digits primitive    */
                                 /* again, courtesy K&R2             */

    CAFInt(primFloatMinExp,
           CUDDLES_FLT_MIN_EXP) /* Float min exponent primitive    */
    CAFInt(primFloatMaxExp,
           CUDDLES_FLT_MAX_EXP) /* Float max exponent primitive    */

    CAFInt(primDoubleDigits,
           CUDDLES_DBL_MANT_DIG) /* Double sig. digits primitive  */
                                 /* again, courtesy K&R2             */

    CAFInt(primDoubleMinExp,
           CUDDLES_DBL_MIN_EXP) /* Double min exponent primitive  */
    CAFInt(primDoubleMaxExp,
           CUDDLES_DBL_MAX_EXP) /* Double max exponent primitive  */

    /* ToDo: GHC stole its decode code from Lennart - maybe we should too?     */
    primFun(primFloatDecode) { /* Float decode primitive           */
    double f;                  /*  :: Float -> (Integer,Int)       */
    int n;                     /* another gruesome hack            */
    FloatArg(f, 1);
    f = frexp((double)(f), &n); /* 0.5   <= f < 1                   */
    f = ldexp(
        f, CUDDLES_FLT_MANT_DIG); /* 2^m-1 <= f < 2^m, m=CUDDLES_FLT_MANT_DIG*/
    n -= CUDDLES_FLT_MANT_DIG;
    push(bigDouble(f));
    updapRoot(ap(mkTuple(2), top()), mkInt(n));
}

/* Float encode primitive */
primFun(primFloatEncode) {
    int n;                            /*  :: Integer -> Int -> Float      */
    Float f;                          /* Ugly hack, don't use Cuddles for    */
    IntArg(n, 1);                     /* numerical work                   */
    eval(primArg(2));                 /* get integer                      */
    f = (Float)bigToDouble(whnfHead); /* and turn it into a float         */
    updateRoot(mkFloat(ldexp(f, n)));
}

/* ToDo: GHC stole its decode code from Lennart - maybe we should too?     */
/* Double decode primitive */
primFun(primDoubleDecode) {
    double f; /*  :: Double -> (Integer,Int)       */
    int n;    /* another gruesome hack            */
    DoubleArg(f, 1);
    f = frexp((double)(f), &n); /* 0.5   <= f < 1                   */
    f = ldexp(
        f, CUDDLES_DBL_MANT_DIG); /* 2^m-1 <= f < 2^m, m=CUDDLES_DBL_MANT_DIG*/
    n -= CUDDLES_DBL_MANT_DIG;
    push(bigDouble(f));
    updapRoot(ap(mkTuple(2), top()), mkInt(n));
}

/* Double encode primitive */
primFun(primDoubleEncode) {
    int n;                             /*  :: Integer -> Int -> Double     */
    Double f;                          /* Ugly hack, don't use Cuddles for    */
    IntArg(n, 1);                      /* numerical work                   */
    eval(primArg(2));                  /* get integer                      */
    f = (Double)bigToDouble(whnfHead); /* and turn it into a double        */
    DoubleResult(ldexp(f, n));
}

/* --------------------------------------------------------------------------
 * Ptr primitives:
 * ------------------------------------------------------------------------*/

/* Null pointer Pointer arithmetic Aligning the pointer */
CAFPtr(primNullPtr, 0) PtrInt2Ptr(primPlusPtr, (char *)x + y)
    primFun(primAlignPtr) {
    void *x;
    int y;
    PtrArg(x, 2);
    IntArg(y, 1);
    if (y <= 0)
        throwException(ap(nameArithException, nameDivideByZero));
    PtrResult((char *)x + (int)((y - (long)x % y) % y));
}
PtrPtr2Int(primMinusPtr,
           (char *)x - (char *)y)  /* Pointer arithmetic           */
    PtrPtr2Bool(primEqPtr, x == y) /* Pointer equality primitive       */

    primFun(primCmpPtr) { /* Pointer compare primitive        */
    void *x, *y;
    PtrArg(x, 2);
    PtrArg(y, 1);
    updateRoot(x < y ? nameLT : (x > y ? nameGT : nameEQ));
}

/* --------------------------------------------------------------------------
 * Comparison primitives:
 * ------------------------------------------------------------------------*/

IntInt2Bool(primEqInt, x == y)           /* Integer equality primitive       */
    WordWord2Bool(primEqWord, x == y)    /* Natural equality primitive       */
    CharChar2Bool(primEqChar, x == y)    /* Character equality primitive     */
    FloatFloat2Bool(primEqFloat, x == y) /* Float equality primitive         */
    DoubleDouble2Bool(primEqDouble,
                      x == y) /* Double equality primitive        */

    primFun(primCmpInt) { /* Integer compare primitive        */
    int x, y;
    IntArg(x, 2);
    IntArg(y, 1);
    updateRoot(x < y ? nameLT : (x > y ? nameGT : nameEQ));
}

/* Natural compare primitive */
primFun(primCmpWord) {
    unsigned x, y;
    WordArg(x, 2);
    WordArg(y, 1);
    updateRoot(x < y ? nameLT : (x > y ? nameGT : nameEQ));
}

/* Character compare primitive */
primFun(primCmpChar) {
    Char x, y;
    CharArg(x, 2);
    CharArg(y, 1);
    updateRoot(x < y ? nameLT : (x > y ? nameGT : nameEQ));
}

/* Float compare primitive */
primFun(primCmpFloat) {
    Float x, y;
    FloatArg(x, 2);
    FloatArg(y, 1);
    updateRoot(x < y ? nameLT : (x > y ? nameGT : nameEQ));
}

/* Double compare primitive */
primFun(primCmpDouble) {
    Double x, y;
    DoubleArg(x, 2);
    DoubleArg(y, 1);
    updateRoot(x < y ? nameLT : (x > y ? nameGT : nameEQ));
}

/* --------------------------------------------------------------------------
 * Evaluate name, obtaining a C string from a Cuddles string:
 * ------------------------------------------------------------------------*/

#if FILENAME_MAX < 1024
# define MAX_STRING 1024
#else
# define MAX_STRING FILENAME_MAX
#endif

/* Evaluate es :: [Char] and save in char array... return ptr to string or 0,
 * if error occurs */
char *evalName(Cell es) {
    static char buffer[MAX_STRING + 1];
    char *bp = buffer;
    StackPtr saveSp = sp;

    eval(es);
    while (whnfHead == nameCons &&
           bp <= buffer + MAX_STRING - MAX_CHAR_ENCODING) {
        eval(pop());
        AddChar(charOf(whnfHead), bp);
        eval(pop());
    }
    if (whnfHead == nameNil) {
        *bp = '\0';
        return buffer;
    }
    sp = saveSp; /* stack pointer must be the same  */
    return 0;    /* as it was on entry              */
}

/* --------------------------------------------------------------------------
 * Top-level printing mechanism:
 * ------------------------------------------------------------------------*/

/* Evaluate string on top of stack and print it on fp */
void outputString(FILE *fp) {
    StackPtr origSp = sp;
    for (;;) {
        Cell temp = evalWithNoError(pop());
        if (nonNull(temp)) {
            sp = origSp;
            top() = printException((top() = temp), nameNil);
        } else if (whnfHead == nameCons) {
            if (nonNull(temp = evalWithNoError(pop()))) {
                sp = origSp;
                onto(temp);
                pushed(1) = printException(pushed(0), pushed(1));
                drop();
            } else {
                FPutChar(charOf(whnfHead), fp);
                fflush(fp);
            }
        } else
            return;
    }
}

/* --------------------------------------------------------------------------
 * Char primitives:
 * ------------------------------------------------------------------------*/

CAFChar(primMaxChar, MAXCHARVAL)

    Char2Bool(primIsUpper, isUpper(x)) Char2Bool(primIsLower, isLower(x))
        Char2Bool(primIsAlpha, isAlpha(x))
            Char2Bool(primIsAlphaNum, isAlphaNum(x))
                Char2Bool(primIsPrint, isPrint(x))

                    Char2Char(primToLower, toLower(x))
                        Char2Char(primToUpper, toUpper(x))
                            Char2Char(primToTitle, toTitle(x))

                                Char2Int(primUniGenCat, uni_gencat(x))

    /* --------------------------------------------------------------------------
 * Extensible records: (Gaster and Jones, 1996)
 * ------------------------------------------------------------------------*/

    /* --------------------------------------------------------------------------
 * Auxilliary functions
 * ------------------------------------------------------------------------*/

    /* The two ways the INSTALL cases in this directory make a Prelude name
 * the interpreter refers to by hand.  Both were a `pFun' and a `predef'
 * macro repeated in six files, each assigning to the caller's global from
 * inside the expansion; a function has its effect in its signature. */

    /* A Prelude name implemented by a primitive table entry */
    Name predefPrim(char *s, char *impl) {
    Name n = newName(findText(s), NIL);
    addPrim(0, n, impl, modulePrelude, NIL);
    return n;
}

/* A Prelude name the Prelude itself defines; recorded now so the compiler can
 * name it */
Name predefName(char *s) {
    Name n = newName(findText(s), NIL);
    nameRec(n)->defn = PREDEFINED;
    return n;
}

/* Follow chain of indirections and CAFs */
Cell followInd(Cell c) {
    do {
        switch (whatIs(c)) {
        case INDIRECT:
            c = snd(c);
            break;
        case NAME:
            if (isCfun(c) || nameRec(c)->arity != 0 ||
                isNull(nameRec(c)->defn)) {
                return c;
            }
            c = nameRec(c)->defn;
            break;
        default:
            return c;
        }
        allowBreak();
    } while (1);
}

/* --------------------------------------------------------------------------
 * Pointer equality
 * ------------------------------------------------------------------------*/

/* Pointer equality tests break referential transparency.
 * However, they can be useful in implementing referentially transparent
 * functions such as lazy memo-tables.
 *
 *   foo = cache sin
 *
 *   cache :: (a -> b) -> (a -> b)
 *   cache f = \x -> unsafePerformIO (check x)
 *    where
 *     ref = unsafePerformIO (newRef (error "cache", error "cache"))
 *     check x = derefRef ref >>= \ (x',a) ->
 *               if x `unsafePtrEq` x' then
 *                 return a
 *               else
 *                 let a = f x in
 *                 assignRef ref (x, a) >>
 *                 return a
 */

/* Unsafe pointer equality test */
primFun(primUnsafePtrEq) {
    Cell x = followInd(primArg(2));
    Cell y = followInd(primArg(1));
    updateRoot((x == y) ? nameTrue : nameFalse);
}

/* Companion function for use when debugging uses of unsafePtrEq.
 * Converts a heap pointer to an Int so you can look at it.
 * I don't think there's any way of using this function that 
 * doesn't break the semantics - debugging use only.
 */
primFun(primUnsafePtrToInt) {
    updateRoot(mkInt(followInd(primArg(1))));
}

/*---------------------------------------------------------------------------
 * GreenCard entry points
 *
 * GreenCard generated code accesses Cuddles data structures and functions 
 * (only) via these functions (which are stored in the virtual function
 * table cuddlesAPI4).
 *-------------------------------------------------------------------------*/

static void getUnit(void);
static HsInt getInt(void);
static HsWord getWord(void);
static HsAddr getAddr(void);
static char getChar4(void);
static HsChar getChar(void);
static CuddlesForeign getForeign(void);
static HsBool getBool(void);
static HsInt8 getInt8(void);
static HsInt16 getInt16(void);
static HsInt32 getInt32(void);
static HsInt64 getInt64(void);
static HsWord8 getWord8(void);
static HsWord16 getWord16(void);
static HsWord32 getWord32(void);
static HsWord64 getWord64(void);
static HsPtr getPtr(void);
static HsFunPtr getFunPtr(void);
static HsForeignPtr getForeignPtr(void);
static HsStablePtr getStablePtr4(void);
static HsFloat getFloat(void);
static HsDouble getDouble(void);

static CuddlesStablePtr getStablePtr(void);

static void putInt(HsInt);
static void putWord(HsWord);
static void putAddr(HsAddr);
static void putChar4(char);
static void putChar(HsChar);
static void putForeign(CuddlesForeign, void (*)(void *));
static void putStablePtr4(HsStablePtr);
static void putBool(HsBool);
static void putInt8(HsInt8);
static void putInt16(HsInt16);
static void putInt32(HsInt32);
static void putInt64(HsInt64);
static void putWord8(HsWord8);
static void putWord16(HsWord16);
static void putWord32(HsWord32);
static void putWord64(HsWord64);
static void putPtr(HsPtr);
static void putFunPtr(HsFunPtr);
static void putForeignPtr(HsForeignPtr);
static void putFloat(HsFloat);
static void putDouble(HsDouble);

static void freeStablePtr4(HsStablePtr);

static void returnIO(CuddlesStackPtr, int);
static void returnId(CuddlesStackPtr, int);
static int runIO(int);
static void apMany(int);

static void getUnit(void) {
    eval(pop());
}
static HsInt getInt(void) {
    eval(pop());
    checkInt();
    return whnfInt;
}
static HsWord getWord(void) {
    eval(pop());
    checkWord();
    return (unsigned int)whnfInt;
}
static HsAddr getAddr(void) {
    eval(pop());
    checkPtr();
    return ptrOf(whnfHead);
}
static char getChar4(void) {
    eval(pop());
    checkChar();
    return (char)charOf(whnfHead);
}
static HsChar getChar(void) {
    eval(pop());
    checkChar();
    return charOf(whnfHead);
}
static CuddlesForeign getForeign(void) {
    eval(pop());
    return derefMP(whnfHead);
}
static HsBool getBool(void) {
    eval(pop());
    checkBool();
    return (whnfHead == nameTrue);
}
static HsInt8 getInt8(void) {
    eval(pop());
    checkInt();
    return (HsInt8)whnfInt;
}
static HsInt16 getInt16(void) {
    eval(pop());
    checkInt();
    return (HsInt16)whnfInt;
}
static HsInt32 getInt32(void) {
    eval(pop());
    checkInt();
    return whnfInt;
}
static HsInt64 getInt64(void) {
    eval(pop());
    return int64FromParts(intOf(fst(snd(whnfHead))), intOf(snd(snd(whnfHead))));
}
static HsWord8 getWord8(void) {
    eval(pop());
    checkWord();
    return (unsigned int)whnfInt;
}
static HsWord16 getWord16(void) {
    eval(pop());
    checkWord();
    return (unsigned int)whnfInt;
}
static HsWord32 getWord32(void) {
    eval(pop());
    checkWord();
    return (unsigned int)whnfInt;
}
static HsWord64 getWord64(void) {
    eval(pop());
    return int64FromParts(intOf(fst(snd(whnfHead))), intOf(snd(snd(whnfHead))));
}
static HsPtr getPtr(void) {
    eval(pop());
    checkPtr();
    return ptrOf(whnfHead);
}
static HsFunPtr getFunPtr(void) {
    eval(pop());
    checkPtr();
    return (HsFunPtr)ptrOf(whnfHead);
}

static HsForeignPtr getForeignPtr(void) {
    errHead(0);
    errPrintf("getForeignPtr: not implemented in Cuddles");
    errFail();
    return 0;
}

static CuddlesStablePtr getStablePtr(void) {
    Cell c = mkStablePtr(pop());
    if (isNull(c)) {
        errHead(0);
        errPrintf("Stable pointer table full");
        errFail();
    }
    return c;
}

static CuddlesStablePtr lookupName(char *, char *);
static CuddlesStablePtr lookupName(char *q, char *n) {
    Name nm = findQualFun(findText(q), findText(n));
    Cell c;

    if (isNull(nm)) {
        errHead(0);
        errPrintf("Can't find qualified name '%s.%s'", q, n);
        errFail();
    }
    c = mkStablePtr(nm);
    if (isNull(c)) {
        errHead(0);
        errPrintf("Stable pointer table full");
        errFail();
    }
    return c;
}

static void putInt(HsInt x) {
    push(mkInt(x));
}
static void putWord(HsWord x) {
    push(mkInt((int)x));
}
static void putAddr(HsAddr x) {
    push(mkPtr(x));
}
static void putChar4(char x) {
    push(mkChar(x));
}
static void putChar(HsChar x) {
    push(mkChar(x));
}
static void putForeign(CuddlesForeign x, void (*f)(CuddlesForeign)) {
    push(mkMallocPtr(x, f));
}
static void putStablePtr(CuddlesStablePtr x) {
    push(derefStablePtr(x));
}
static void putBool(HsBool x) {
    push(x ? nameTrue : nameFalse);
}

static void putInt8(HsInt8 x) {
    push(mkInt(x));
}
static void putInt16(HsInt16 x) {
    push(mkInt(x));
}
static void putInt32(HsInt32 x) {
    push(mkInt(x));
}
static void putInt64(HsInt64 x) {
    push(pair(I64CELL, pair(mkInt(part1Int64(x)), mkInt(part2Int64(x)))));
}
static void putWord8(HsWord8 x) {
    push(mkInt((int)x));
}
static void putWord16(HsWord16 x) {
    push(mkInt((int)x));
}
static void putWord32(HsWord32 x) {
    push(mkInt((int)x));
}
static void putWord64(HsWord64 x) {
    push(pair(I64CELL, pair(mkInt(part1Int64((HsInt64)x)),
                            mkInt(part2Int64((HsInt64)x)))));
}
static void putPtr(HsPtr x) {
    push(mkPtr(x));
}
static void putFunPtr(HsFunPtr x) {
    push(mkPtr((void *)x));
}

/* A StablePtr is represented in Haskell as mkInt(sp), the stable-pointer
 * table index returned by mkStablePtr/newStablePtr (see primMakeSP and the
 * SPArg/SPResult macros in iomonad.c) -- not as a raw Cell number, so it
 * must be boxed with mkInt() on the way out, and evaluated (it may still
 * be an unevaluated thunk) and unboxed with checkInt()/whnfInt on the way
 * in, exactly like every other get()/put() pair here.                    */
static void putStablePtr4(HsStablePtr x) {
    push(mkInt((CuddlesStablePtr)(intptr_t)x));
}

static HsStablePtr getStablePtr4(void) {
    eval(pop());
    checkInt();
    return (HsStablePtr)(intptr_t)(CuddlesStablePtr)whnfInt;
}

static void freeStablePtr4(HsStablePtr x) {
    if (x)
        freeStablePtr((CuddlesStablePtr)(intptr_t)x);
}

static HsFloat getFloat(void) {
    eval(pop());
    checkFloat();
    return whnfFloat;
}
static HsDouble getDouble(void) {
    eval(pop());
    checkDouble();
    return whnfDouble;
}

static void putFloat(HsFloat x) {
    push(mkFloat(x));
}

static void putDouble(HsDouble x) {
    push(mkDouble(x));
}

static void putForeignPtr(HsForeignPtr x) {
    errHead(0);
    errPrintf("putForeignPtr: not implemented in Cuddles");
    errFail();
}

/* Return in IO monad */
static void returnIO(CuddlesStackPtr root, int n) {
    /* There should be n return values on the top of the stack */
    if (n == 0) {
        push(nameUnit);
    } else if (n == 1) {
        /* do nothing */
    } else {
        int i;
        push(mkTuple(n));
        for (i = 0; i < n; ++i) {
            pushed(1) = ap(pushed(0), pushed(1));
            drop();
        }
    }
    IOReturn(top());
}

/* Return in identity monad */
static void returnId(CuddlesStackPtr root, int n) {
    /* There should be n return values on the top of the stack */
    if (n == 0) {
        push(nameUnit);
    } else if (n == 1) {
        /* do nothing, already there. */
    } else {
        int i;
        push(mkTuple(n));
        for (i = 0; i < n; ++i) {
            pushed(1) = ap(pushed(0), pushed(1));
            drop();
        }
    }
    /*
     * Note: have to be a bit careful when returning, since we could
     * be returning from evaluating a CAF. In the non-CAF case,
     * 'root' points to the object we've entered, so we can just
     * go ahead and update it. No problem there.
     *
     * For CAFs, the evaluator leaves 'root' pointing at the Name
     * of the CAF, so updating it directly is not going to work.
     * Instead, we just leave the result at the top of the stack & 
     * let the evaluator do what it has always done; clean up & 
     * update the 'defn' field inside the CAF Name.
     * 
     *  [6/01 --sof]
     */
    if (isPair(stack(root))) {
        updateRoot(top());
    }
}

static int runIO(int n) {
    /* stack = argn : ... : arg1 : fun : rest */
    StackPtr old_sp = sp - n - 1;
    Cell temp = NIL;
    int i;
    /* build application node */
    for (i = n - 1; i >= 0; --i) {
        pushed(n) = ap(pushed(n), pushed(i));
    }
    sp -= n;

    /* evaluate it - should have type IO a */
    temp = evalWithNoError(ap(nameIORun, pop()));
    if (nonNull(temp)) {
        errHead(0);
        errPrintf("runIO: uncaught error");
        errFail();
    }
    if (sp != old_sp + 1) {
        errHead(0);
        errPrintf("runIO: unbalanced stack (%d)", sp - old_sp);
        errFail();
    }
    if (whnfHead == nameRight) {
        return 0;
    } else if (whnfHead != nameLeft) { /* Called "exit" */
        errHead(0);
        errPrintf("runIO: bad return value");
        errFail();
    }
    return 1;
}

static void apMany(int n) {
    /* stack = argn : ... : arg1 : fun : rest */
    int i;
    /* build application node */
    for (i = n - 1; i >= 0; --i) {
        pushed(n) = ap(pushed(n), pushed(i));
    }
    sp -= n;
    /* stack = ap(...(ap(fun,arg1),...),argn) : rest */
}

static int runId(int);

static int runId(int n) {
    apMany(n);
    top() = ap(nameReturnIO, top());
    return runIO(0);
}

/* This allocates a small object and writes some machine code into it.  
 *
 * The code generated by the generated code is equivalent to:
 *  
 *   rty f(ty1 a1, ... tym am) {
 *     return (*app)(s,a1, ... am);
 *   }
 *
 * Where s is a stable pointer (an int).
 *
 * But, because this is machine code, we can do it without knowing
 * anything about the argument types.  This works because the C
 * calling convention on most machines has the stack looking something
 * like this (where leftmost = top of stack)
 *
 *   ret_addr : a1 : ... am : rest of stack
 *
 * If this is not the case for some architecture/calling convention,
 * the easiest thing to do might be to make the stable pointer be
 * the last argument (if that is easily found) or to put it in a callee
 * saves register - and then adapt the apply function generated by 
 * implementForeignExport.  
 *
 * thunk->{next,prev}: a doubly linked list (makes deletion easy)
 * thunk->stable:      the stable pointer (makes deletion easy)
 * 
 * At the end of execution, we run down the list freeing every thunk
 * that the user did not explicitly deallocate.
 */

struct thunk_data {
    struct thunk_data *next;
    struct thunk_data *prev;
    CuddlesStablePtr stable;
    /* Machine-code generation for the adjustor thunk itself (the i386,
     * PowerPC and SPARC cases GHC's Adjustor.c has) is gone: none of
     * those architectures are supported, and mkThunk
     * below now refuses "foreign import \"wrapper\"" outright, before
     * a thunk_data is ever allocated. This placeholder field just
     * keeps freeHaskellFunctionPtr's "&thunk->code" pointer arithmetic
     * well defined for the caller-supplied pointers it is handed.
     */
    char code[1];
};

static void *mkThunk(void (*)(void), CuddlesStablePtr);
static void freeThunkAux(struct thunk_data *);
static void freeAllThunks(void);

static struct thunk_data *foreignThunks = 0;

static void *mkThunk(void (*app)(void), CuddlesStablePtr s) {
    /* Every architecture we still support needs machine code written
     * into the thunk to trampoline into Haskell (see the comment above
     * struct thunk_data), and none of the ones that had it (i386,
     * PowerPC, SPARC) are supported any more, so refuse right away -
     * in particular before allocating a thunk_data and linking it into
     * foreignThunks, which the earlier code did first and
     * left there, half-initialised, once this error longjmp'd out. */
    errHead(0);
    errPrintf("Foreign import wrapper is not supported on this architecture");
    errFail();
    return 0; /*NOTREACHED*/
}

static void freeThunkAux(struct thunk_data *thunk) {
    freeStablePtr(thunk->stable);
    if (thunk->prev) {
        assert(foreignThunks != thunk);
        thunk->prev->next = thunk->next;
    } else {
        assert(foreignThunks == thunk);
        foreignThunks = thunk->next;
    }
    if (thunk->next) {
        thunk->next->prev = thunk->prev;
    }
    free(thunk);
}

static void freeAllThunks(void) {
    while (foreignThunks) {
        freeThunkAux(foreignThunks);
    }
}

/* This frees the object allocated by mkThunk.
 * [A useful debugging mode would not deallocate the thunk but would
 *  overwrite the thunk with code which prints an error message.]
 */
static void freeHaskellFunctionPtr(void *t) {
    struct thunk_data *thunk =
        (struct thunk_data *)((char *)t -
                              (char *)&((struct thunk_data *)0)->code);
    freeThunkAux(thunk);
}

/* :: FunPtr a -> IO () */
primFun(primFreeHFunPtr) {
    void *x;
    PtrArg(x, 1 + IOArity);
    freeHaskellFunctionPtr(x);
    IOReturn(nameUnit);
}

/* Build virtual function table */
CuddlesAPI4 *cuddlesAPI4(void) {
    static CuddlesAPI4 api;
    static bool initialised = false;

    if (!initialised) {

        /* evaluate next argument */
        api.getInt = getInt;
        api.getWord = getWord;
        api.getAddr = getAddr;
        api.getFloat = getFloat;
        api.getDouble = getDouble;
        api.getChar = getChar4;
        api.getForeign = getForeign;
        api.getStablePtr = getStablePtr;

        /* push part of result   */
        api.putInt = putInt;
        api.putWord = putWord;
        api.putAddr = putAddr;
        api.putFloat = putFloat;
        api.putDouble = putDouble;
        api.putChar = putChar4;
        api.putForeign = putForeign;
        api.putStablePtr = putStablePtr;

        /* return n values in IO monad or Id monad */
        api.returnIO = returnIO;
        api.returnId = returnId;
        api.runIO = runIO;

        /* free a stable pointer */
        api.freeStablePtr = freeStablePtr;

        /* register the prim table */
        api.registerPrims = registerPrims;

        /* garbage collect */
        api.garbageCollect = garbageCollect;

        /* API3 additions follow */
        api.lookupName = lookupName;
        api.ap = apMany;
        api.getUnit = getUnit;
        api.mkThunk = mkThunk;
        api.freeThunk = freeHaskellFunctionPtr;
        api.getBool = getBool;
        api.putBool = putBool;

        /* API4 additions follow */
        api.getInt8 = getInt8;
        api.getInt16 = getInt16;
        api.getInt32 = getInt32;
        api.getInt64 = getInt64;
        api.getWord8 = getWord8;
        api.getWord16 = getWord16;
        api.getWord32 = getWord32;
        api.getWord64 = getWord64;
        api.getPtr = getPtr;
        api.getFunPtr = getFunPtr;
        api.getForeignPtr = getForeignPtr;

        api.putInt8 = putInt8;
        api.putInt16 = putInt16;
        api.putInt32 = putInt32;
        api.putInt64 = putInt64;
        api.putWord8 = putWord8;
        api.putWord16 = putWord16;
        api.putWord32 = putWord32;
        api.putWord64 = putWord64;
        api.putPtr = putPtr;
        api.putFunPtr = putFunPtr;
        api.putForeignPtr = putForeignPtr;

        api.makeStablePtr4 = getStablePtr;
        api.derefStablePtr4 = putStablePtr;

        api.putStablePtr4 = putStablePtr4;
        api.getStablePtr4 = getStablePtr4;
        api.freeStablePtr4 = freeStablePtr4;

        api.runId = runId;
    }
    return &api;
}

/* Build virtual function table */
CuddlesAPI5 *cuddlesAPI5(void) {
    static CuddlesAPI5 api;
    static bool initialised = false;
    if (!initialised) {
        api.getBool = getBool;
        api.getInt = getInt;
        api.getWord = getWord;
        api.getAddr = getAddr;
        api.getFloat = getFloat;
        api.getDouble = getDouble;
        api.getChar = getChar;
        api.getForeign = getForeign;
        api.getInt8 = getInt8;
        api.getInt16 = getInt16;
        api.getInt32 = getInt32;
        api.getInt64 = getInt64;
        api.getWord8 = getWord8;
        api.getWord16 = getWord16;
        api.getWord32 = getWord32;
        api.getWord64 = getWord64;
        api.getPtr = getPtr;
        api.getFunPtr = getFunPtr;
        api.getForeignPtr = getForeignPtr;
        api.getStablePtr4 = getStablePtr4;

        api.putBool = putBool;
        api.putInt = putInt;
        api.putWord = putWord;
        api.putAddr = putAddr;
        api.putFloat = putFloat;
        api.putDouble = putDouble;
        api.putChar = putChar;
        api.putForeign = putForeign;
        api.putInt8 = putInt8;
        api.putInt16 = putInt16;
        api.putInt32 = putInt32;
        api.putInt64 = putInt64;
        api.putWord8 = putWord8;
        api.putWord16 = putWord16;
        api.putWord32 = putWord32;
        api.putWord64 = putWord64;
        api.putPtr = putPtr;
        api.putFunPtr = putFunPtr;
        api.putForeignPtr = putForeignPtr;
        api.putStablePtr4 = putStablePtr4;

        api.returnIO = returnIO;
        api.runIO = runIO;
        api.returnId = returnId;
        api.runId = runId;

        api.registerPrims = registerPrims;

        api.lookupName = lookupName;
        api.ap = apMany;
        api.getUnit = getUnit;
        api.mkThunk = mkThunk;
        api.freeThunk = freeHaskellFunctionPtr;

        api.makeStablePtr4 = getStablePtr;
        api.derefStablePtr4 = putStablePtr;
        api.freeStablePtr4 = freeStablePtr4;
    }
    return &api;
}

void hs_perform_gc(void) {
    garbageCollect();
}

void hs_free_stable_ptr(HsStablePtr sp) {
    freeStablePtr4(sp);
}

void hs_free_fun_ptr(HsFunPtr fp) {
    freeHaskellFunctionPtr((void *)fp);
}

/* --------------------------------------------------------------------------
 * Built-in control:
 * ------------------------------------------------------------------------*/

/* Dummy entry */
static void builtinControl(int);
static void builtinControl(int what) {
}
static struct primInfo builtinPrims = {builtinControl, builtinPrimTable, 0};

/* Register every primitive module, and predefine the primitive and
 * derived names the rest of the interpreter refers to by global; done
 * once, before the Prelude is read. */
static void installBuiltIn(void) {
    registerPrims(&builtinPrims);
    registerPrims(&printerPrims);
    registerPrims(&arrayPrims);
    registerPrims(&bignumPrims);
    registerPrims(&iomonadPrims);
    registerPrims(&timePrims);
    registerPrims(&dirPrims);
#if INTERNAL_PRIMS
    registerPrims(&internalPrims);
#endif
    setCurrModule(modulePrelude);
    nameFatbar = predefPrim("_FATBAR", "fatbar");
    nameFail = predefPrim("_FAIL", "fail");
    nameIf = predefPrim("_IF", "if");
    nameSel = predefPrim("_SEL", "sel");

    nameConCmp = predefPrim("_concmp", "conCmp");
    nameEnRange = predefPrim("_range", "enRange");
    nameEnIndex = predefPrim("_index", "enIndex");
    nameEnInRng = predefPrim("_inRange", "enInRng");
    nameEnToEn = predefPrim("_toEnum", "enToEn");
    nameEnFrEn = predefPrim("_frEnum", "enFrEn");
    nameEnFrom = predefPrim("_from", "enFrom");
    nameEnFrTo = predefPrim("_fromTo", "enFrTo");
    nameEnFrTh = predefPrim("_fromThen", "enFrTh");

    namePrimThrow = predefPrim("_throw", "primThrowException");
    nameBlackHole = predefPrim("_Gc Black Hole", "gcBhole");
    nameInd = predefPrim("_indirect", "dictIndirect");
    nameRec(nameInd)->number = DFUNNAME;
    nameNegate = predefName("negate");
    nameFlip = predefName("flip");
    nameFrom = predefName("enumFrom");
    nameFromThen = predefName("enumFromThen");
    nameFromTo = predefName("enumFromTo");
    nameFromThenTo = predefName("enumFromThenTo");
    nameFst = predefName("fst");
    nameSnd = predefName("snd");
    nameAnd = predefName("&&");
    nameOr = predefName("||");
    nameId = predefName("id");
    nameOtherwise = predefName("otherwise");
    nameComp = predefName(".");
    nameApp = predefName("++");
    nameShowField = predefName("showField");
    nameShowParen = predefName("showParen");
    nameReadField = predefName("readField");
    nameReadParen = predefName("readParen");
    nameLex = predefName("lex");
    nameRangeSize = predefName("rangeSize");
    nameCompAux = predefName("primCompAux");
    namePmInt = predefName("primPmInt");
    namePmInteger = predefName("primPmInteger");
    namePmFlt = predefName("primPmFlt");
    nameReturnIO = predefName("primretIO");
    nameRationalToDouble = predefName("rationalToDouble");
    nameRationalToFloat = predefName("rationalToFloat");
    nameFloatToRational = predefName("floatToRational");
    nameDoubleToRational = predefName("doubleToRational");
    nameDoubleToRatio = predefName("doubleToRatio");
    nameIntToRatio = predefName("intToRatio");
    nameIntToFloat = predefName("primIntToFloat");
    nameIntToDouble = predefName("primIntToDouble");
    nameDoubleToFloat = predefName("primDoubleToFloat");
    nameFloatToDouble = predefName("primFloatToDouble");
}

/* Free every thunk built while evaluating the previous script or
 * expression. */
static void resetBuiltIn(void) {
    freeAllThunks();
}

void builtIn(int what) {
    switch (what) {
    case INSTALL:
        installBuiltIn();
        break;
    case RESET:
        resetBuiltIn();
        break;
    default:
        break;
    }
}

/*-------------------------------------------------------------------------*/
