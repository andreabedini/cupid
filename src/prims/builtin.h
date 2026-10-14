/* --------------------------------------------------------------------------
 * Primitive functions, input output etc...
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/
#ifndef __BUILTIN_H__
#define __BUILTIN_H__

#include "prelude.h"
#include "storage.h"
#include "errors.h" /* IWYU pragma: keep -- internal(), in the CHECK_TAGS macros below */

extern char *evalName(Cell);
extern Cell mkIOError(Cell *, Name, char *, char *, Cell *);
extern Cell followInd(Cell); /* builtin.c: skip indirections    */
extern void throwErrno(char *, bool, int,
                       Cell *);         /* iomonad.c: raise an IOError  */
extern Cell printException(Cell, Cell); /* printer.c: exception -> String*/
extern Name predefPrim(char *, char *); /* builtin.c: name + primitive  */
extern Name predefName(char *);         /* builtin.c: name to be defined*/

extern void installBuiltIn(void); /* register every module, predefine names */
extern void resetBuiltIn(void);   /* free every thunk from the last script  */

/* The primitive tables of the other units in this directory, registered
 * by installBuiltIn() in builtin.c */
extern struct primInfo arrayPrims, bignumPrims, dirPrims, iomonadPrims,
    printerPrims, timePrims;
#if INTERNAL_PRIMS
extern struct primInfo internalPrims;
#endif

/* --------------------------------------------------------------------------
 * Macros used to define primitives:
 * ------------------------------------------------------------------------*/

#define PROTO_PRIM(name) static void name(StackPtr)
#define EXT_PROTO_PRIM(name) extern void name(StackPtr)
#define primFun(name) static void name(StackPtr root)
#define extPrimFun(name) void name(StackPtr root)
#define primCAF(name) static void name(StackPtr root CUPID_unused)
#define primArg(n) stack(root + n)

/* IMPORTANT: the second element of an update must be written first.
 * this is to deal with the case where an INDIRECT tag is written into
 * a Cell before the second value has been set.  If a garbage collection
 * occurs before the second element was set then the INDIRECTion will be
 * (wrongly) elided and result in chaos.  I know.  It happened to me.
 */

#define update(l, r) ((snd(stack(root)) = r), (fst(stack(root)) = l))
#define updateRoot(c) update(INDIRECT, c)
#define updapRoot(l, r) update(l, r)
#define blackHoleRoot() update(nameBlackHole, nameBlackHole)

#if CHECK_TAGS

# define checkChar()                                                           \
     if (!isChar(whnfHead))                                                    \
     internal("Char expected")
# define checkInt()                                                            \
     if (!isInt(whnfHead))                                                     \
     internal("Int expected")
# define checkWord()                                                           \
     if (!isInt(whnfHead))                                                     \
     internal("Word expected")
# define checkPtr()                                                            \
     if (!isPtr(whnfHead))                                                     \
     internal("Ptr expected")
# define checkFloat()                                                          \
     if (!isFloat(whnfHead))                                                   \
     internal("Float expected")
# define checkDouble()                                                         \
     if (!isDouble(whnfHead))                                                  \
     internal("Double expected")

# define checkBool()                                                           \
     if (whnfHead != nameTrue && whnfHead != nameFalse)                        \
         internal("Bool expected");

# define checkCon()                                                            \
     if (!isName(whnfHead) || !isCfun(whnfHead))                               \
         internal("Constructor expected");

#else

# define checkChar() doNothing()
# define checkInt() doNothing()
# define checkWord() doNothing()
# define checkPtr() doNothing()
# define checkFloat() doNothing()
# define checkDouble() doNothing()
# define checkBool() doNothing()
# define checkCon() doNothing()

#endif

/* e is a constant expression                     */
#define CAFPtr(nm, e)                                                          \
    primCAF(nm) {                                                              \
        void *r = e;                                                           \
        push(mkPtr(r));                                                        \
    }

#define PtrArg(nm, offset)                                                     \
    eval(primArg(offset));                                                     \
    checkPtr();                                                                \
    nm = ptrOf(whnfHead)

/* nm should be a variable in which result is stored.
   If you use an expression, reevaluation might occur */
#define PtrResult(nm) updateRoot(mkPtr(nm))

/* e is an expression with free variables x and y */
#define PtrInt2Ptr(nm, e)                                                      \
    primFun(nm) {                                                              \
        void *x, *r;                                                           \
        int y;                                                                 \
        PtrArg(x, 2);                                                          \
        IntArg(y, 1);                                                          \
        r = e;                                                                 \
        PtrResult(r);                                                          \
    }

/* e is an expression with free variables x */
#define Ptr2Int(nm, e)                                                         \
    primFun(nm) {                                                              \
        void *x;                                                               \
        int r;                                                                 \
        PtrArg(x, 1);                                                          \
        r = e;                                                                 \
        IntResult(r);                                                          \
    }

/* e is a constant expression                     */
#define CAFInt(nm, e)                                                          \
    primCAF(nm) {                                                              \
        int r = e;                                                             \
        push(mkInt(r));                                                        \
    }

#define IntArg(nm, offset)                                                     \
    eval(primArg(offset));                                                     \
    checkInt();                                                                \
    nm = whnfInt

/* nm should be a variable in which result is stored.
   If you use an expression, reevaluation might occur */
#define IntResult(nm) updateRoot(mkInt(nm))

/* nm should be a variable in which result is stored.
   If you use an expression, reevaluation might occur */
#define IntIntResult(e1, e2)                                                   \
    do {                                                                       \
        int _arg1 = e1;                                                        \
        int _arg2 = e2;                                                        \
        push(mkInt(_arg1));                                                    \
        topfun(mkTuple(2));                                                    \
        updapRoot(top(), mkInt(_arg2));                                        \
    } while (0)

/* e is a constant expression                     */
#define CAFWord(nm, e)                                                         \
    primCAF(nm) {                                                              \
        unsigned r = e;                                                        \
        push(mkInt(r));                                                        \
    }

#define WordArg(nm, offset)                                                    \
    eval(primArg(offset));                                                     \
    checkWord();                                                               \
    nm = (unsigned)whnfInt

/* nm should be a variable in which result is stored.
   If you use an expression, reevaluation might occur */
#define WordResult(nm) updateRoot(mkInt(nm))

/* nm should be a variable in which result is stored.
   If you use an expression, reevaluation might occur */
#define WordWordResult(e1, e2)                                                 \
    do {                                                                       \
        unsigned _arg1 = e1;                                                   \
        unsigned _arg2 = e2;                                                   \
        push(mkInt(_arg1));                                                    \
        topfun(mkTuple(2));                                                    \
        updapRoot(top(), mkInt(_arg2));                                        \
    } while (0)

#define FloatArg(nm, offset)                                                   \
    eval(primArg(offset));                                                     \
    checkFloat();                                                              \
    nm = whnfFloat

/* nm should be a variable in which result is stored.
   If you use an expression, reevaluation might occur */
#define FloatResult(nm) updateRoot(mkFloat(nm))

#define DoubleArg(nm, offset)                                                  \
    eval(primArg(offset));                                                     \
    checkDouble();                                                             \
    nm = whnfDouble

/* nm should be a variable in which result is stored.
   If you use an expression, reevaluation might occur */
#define DoubleResult(nm) updateRoot(mkDouble(nm))

#define BoolArg(nm, offset)                                                    \
    eval(primArg(offset));                                                     \
    checkBool();                                                               \
    nm = (whnfHead == nameTrue)

/* e can be an expression if you want */
#define BoolResult(e) updateRoot((e) ? nameTrue : nameFalse)

#define ConArg(nm, offset)                                                     \
    eval(primArg(offset));                                                     \
    checkCon();                                                                \
    nm = cfunOf(whnfHead)

/* e is an expression with free variables x and y */
#define IntInt2Int(nm, e)                                                      \
    primFun(nm) {                                                              \
        int x, y, r;                                                           \
        IntArg(x, 2);                                                          \
        IntArg(y, 1);                                                          \
        r = e;                                                                 \
        IntResult(r);                                                          \
    }

/* e is a predicate with free variables x and y   */
#define PtrPtr2Bool(nm, e)                                                     \
    primFun(nm) {                                                              \
        void *x, *y;                                                           \
        PtrArg(x, 2);                                                          \
        PtrArg(y, 1);                                                          \
        BoolResult(e);                                                         \
    }

/* e is an expression with free variables x and y */
#define PtrPtr2Int(nm, e)                                                      \
    primFun(nm) {                                                              \
        void *x, *y;                                                           \
        int r;                                                                 \
        PtrArg(x, 2);                                                          \
        PtrArg(y, 1);                                                          \
        r = e;                                                                 \
        IntResult(r);                                                          \
    }

/* e is an expression with free variables x and y */
/* y must be non-zero                             */
#define IntInt2IntNonZero(nm, e)                                               \
    primFun(nm) {                                                              \
        int x, y, r;                                                           \
        IntArg(x, 2);                                                          \
        IntArg(y, 1);                                                          \
        if (y == 0)                                                            \
            throwException(ap(nameArithException, nameDivideByZero));          \
        r = e;                                                                 \
        IntResult(r);                                                          \
    }

/* e is an expression with free variable x        */
#define Int2Int(nm, e)                                                         \
    primFun(nm) {                                                              \
        int x, r;                                                              \
        IntArg(x, 1);                                                          \
        r = e;                                                                 \
        IntResult(r);                                                          \
    }

/* e is a predicate with free variables x and y   */
#define IntInt2Bool(nm, e)                                                     \
    primFun(nm) {                                                              \
        int x, y;                                                              \
        IntArg(x, 2);                                                          \
        IntArg(y, 1);                                                          \
        BoolResult(e);                                                         \
    }

/* e is a predicate with free variable x          */
#define Int2Bool(nm, e)                                                        \
    primFun(nm) {                                                              \
        int x;                                                                 \
        IntArg(x, 1);                                                          \
        BoolResult(e);                                                         \
    }

/* e is an expression with free variables x and y */
#define WordWord2Word(nm, e)                                                   \
    primFun(nm) {                                                              \
        unsigned x, y, r;                                                      \
        WordArg(x, 2);                                                         \
        WordArg(y, 1);                                                         \
        r = e;                                                                 \
        WordResult(r);                                                         \
    }

/* e is an expression with free variables x and y */
/* y must be non-zero                             */
#define WordWord2WordNonZero(nm, e)                                            \
    primFun(nm) {                                                              \
        unsigned x, y, r;                                                      \
        WordArg(x, 2);                                                         \
        WordArg(y, 1);                                                         \
        if (y == 0)                                                            \
            throwException(ap(nameArithException, nameDivideByZero));          \
        r = e;                                                                 \
        WordResult(r);                                                         \
    }

/* e is an expression with free variable x        */
#define Word2Word(nm, e)                                                       \
    primFun(nm) {                                                              \
        unsigned x, r;                                                         \
        WordArg(x, 1);                                                         \
        r = e;                                                                 \
        WordResult(r);                                                         \
    }

/* e is a predicate with free variables x and y   */
#define WordWord2Bool(nm, e)                                                   \
    primFun(nm) {                                                              \
        unsigned x, y;                                                         \
        WordArg(x, 2);                                                         \
        WordArg(y, 1);                                                         \
        BoolResult(e);                                                         \
    }

/* e is a predicate with free variables x and y   */
#define WordInt2Bool(nm, e)                                                    \
    primFun(nm) {                                                              \
        unsigned x;                                                            \
        int y;                                                                 \
        WordArg(x, 2);                                                         \
        IntArg(y, 1);                                                          \
        BoolResult(e);                                                         \
    }

/* e is a predicate with free variables x and y   */
#define WordInt2Word(nm, e)                                                    \
    primFun(nm) {                                                              \
        unsigned x;                                                            \
        int y;                                                                 \
        unsigned r;                                                            \
        WordArg(x, 2);                                                         \
        IntArg(y, 1);                                                          \
        r = e;                                                                 \
        WordResult(r);                                                         \
    }

/* e is a predicate with free variable x          */
#define Int2Word(nm, e)                                                        \
    primFun(nm) {                                                              \
        int x;                                                                 \
        unsigned r;                                                            \
        IntArg(x, 1);                                                          \
        r = e;                                                                 \
        WordResult(r);                                                         \
    }

/* e is a predicate with free variable x          */
#define Word2Bool(nm, e)                                                       \
    primFun(nm) {                                                              \
        unsigned x;                                                            \
        WordArg(x, 1);                                                         \
        BoolResult(e);                                                         \
    }

/* e is an expression with free variables x and y */
#define FloatFloat2Float(nm, e)                                                \
    primFun(nm) {                                                              \
        Float x, y, r;                                                         \
        FloatArg(x, 2);                                                        \
        FloatArg(y, 1);                                                        \
        r = e;                                                                 \
        FloatResult(r);                                                        \
    }

/* e is an expression with free variables x and y */
/* y must be non-zero                             */
#define FloatFloat2FloatNonZero(nm, e)                                         \
    primFun(nm) {                                                              \
        Float x, y, r;                                                         \
        FloatArg(x, 2);                                                        \
        FloatArg(y, 1);                                                        \
        if (y == 0)                                                            \
            throwException(ap(nameArithException, nameDivideByZero));          \
        r = e;                                                                 \
        FloatResult(r);                                                        \
    }

/* e is an expression with free variable x        */
#define Float2Float(nm, e)                                                     \
    primFun(nm) {                                                              \
        Float x, r;                                                            \
        FloatArg(x, 1);                                                        \
        r = (Float)e;                                                          \
        FloatResult(r);                                                        \
    }

/* e is an expression with free variable x        */
#define Float2Int(nm, e)                                                       \
    primFun(nm) {                                                              \
        Float x;                                                               \
        int r;                                                                 \
        FloatArg(x, 1);                                                        \
        r = e;                                                                 \
        IntResult(r);                                                          \
    }

/* e is an expression with free variable x        */
#define Float2Bool(nm, e)                                                      \
    primFun(nm) {                                                              \
        Float x;                                                               \
        bool r;                                                                \
        FloatArg(x, 1);                                                        \
        r = e;                                                                 \
        BoolResult(r);                                                         \
    }

/* e is an expression with free variable x        */
/* pre is a precondition (fv x) to test           */
#define Float2FloatPre(nm, e, pre)                                             \
    primFun(nm) {                                                              \
        Float x, r;                                                            \
        FloatArg(x, 1);                                                        \
        if (!(pre))                                                            \
            throwException(                                                    \
                ap(nameErrorCall, mkStr(findText("argument out of range"))));  \
        r = (Float)e;                                                          \
        FloatResult(r);                                                        \
    }

/* e is an expression with free variables x and y */
#define DoubleDouble2Double(nm, e)                                             \
    primFun(nm) {                                                              \
        Double x, y, r;                                                        \
        DoubleArg(x, 2);                                                       \
        DoubleArg(y, 1);                                                       \
        r = e;                                                                 \
        DoubleResult(r);                                                       \
    }

/* e is an expression with free variables x and y */
/* y must be non-zero                             */
#define DoubleDouble2DoubleNonZero(nm, e)                                      \
    primFun(nm) {                                                              \
        Double x, y, r;                                                        \
        DoubleArg(x, 2);                                                       \
        DoubleArg(y, 1);                                                       \
        if (y == 0)                                                            \
            throwException(ap(nameArithException, nameDivideByZero));          \
        r = e;                                                                 \
        DoubleResult(r);                                                       \
    }

/* e is an expression with free variable x        */
#define Double2Double(nm, e)                                                   \
    primFun(nm) {                                                              \
        Double x, r;                                                           \
        DoubleArg(x, 1);                                                       \
        r = (Double)e;                                                         \
        DoubleResult(r);                                                       \
    }

/* e is an expression with free variable x        */
#define Double2Int(nm, e)                                                      \
    primFun(nm) {                                                              \
        Double x;                                                              \
        int r;                                                                 \
        DoubleArg(x, 1);                                                       \
        r = e;                                                                 \
        IntResult(r);                                                          \
    }

/* e is an expression with free variable x        */
#define Double2Bool(nm, e)                                                     \
    primFun(nm) {                                                              \
        Double x;                                                              \
        bool r;                                                                \
        DoubleArg(x, 1);                                                       \
        r = e;                                                                 \
        BoolResult(r);                                                         \
    }

/* e is an expression with free variable x        */
/* pre is a precondition (fv x) to test           */
#define Double2DoublePre(nm, e, pre)                                           \
    primFun(nm) {                                                              \
        Double x, r;                                                           \
        DoubleArg(x, 1);                                                       \
        if (!(pre))                                                            \
            throwException(                                                    \
                ap(nameErrorCall, mkStr(findText("argument out of range"))));  \
        r = (Double)e;                                                         \
        DoubleResult(r);                                                       \
    }

/* e is a constant expression                     */
#define CAFChar(nm, e)                                                         \
    primCAF(nm) {                                                              \
        Char r = e;                                                            \
        push(mkChar(r));                                                       \
    }

#define CharArg(nm, offset)                                                    \
    eval(primArg(offset));                                                     \
    checkChar();                                                               \
    nm = charOf(whnfHead)

/* nm should be a variable in which result is stored.
   If you use an expression, reevaluation might occur */
#define CharResult(nm) updateRoot(mkChar(nm))

/* e is a predicate with free variables x and y   */
#define CharChar2Bool(nm, e)                                                   \
    primFun(nm) {                                                              \
        Char x, y;                                                             \
        CharArg(x, 2);                                                         \
        CharArg(y, 1);                                                         \
        BoolResult(e);                                                         \
    }

/* e is a predicate with free variable x          */
#define Char2Bool(nm, e)                                                       \
    primFun(nm) {                                                              \
        Char x;                                                                \
        CharArg(x, 1);                                                         \
        BoolResult(e);                                                         \
    }

/* e is an expression with free variable x        */
#define Char2Char(nm, e)                                                       \
    primFun(nm) {                                                              \
        Char x;                                                                \
        CharArg(x, 1);                                                         \
        CharResult(e);                                                         \
    }

/* e is an integer expression with free variable x */
#define Char2Int(nm, e)                                                        \
    primFun(nm) {                                                              \
        Char x;                                                                \
        CharArg(x, 1);                                                         \
        IntResult(e);                                                          \
    }

/* e is a predicate with free variables x and y   */
#define FloatFloat2Bool(nm, e)                                                 \
    primFun(nm) {                                                              \
        Float x, y;                                                            \
        FloatArg(x, 2);                                                        \
        FloatArg(y, 1);                                                        \
        BoolResult(e);                                                         \
    }

/* e is a predicate with free variables x and y   */
#define DoubleDouble2Bool(nm, e)                                               \
    primFun(nm) {                                                              \
        Double x, y;                                                           \
        DoubleArg(x, 2);                                                       \
        DoubleArg(y, 1);                                                       \
        BoolResult(e);                                                         \
    }

/* --------------------------------------------------------------------------
 * IO monad macros:
 *
 * Note: the IOReturn and IOFail macros do not use the standard "do while"
 * trick to create a single statement because some C compilers (eg sun)
 * report warning messages "end-of-loop code not reached".
 * This may lead to syntax errors if used where a statement is required - such
 * errors can be fixed by adding braces round the call.  Blech!
 * ------------------------------------------------------------------------*/
#define IOArg(n) primArg((n) + IOArity)
#define IOReturn(r)                                                            \
    {                                                                          \
        updapRoot(primArg(1), r);                                              \
        return;                                                                \
    }
#define IOFail(r) throwException(ap(nameIOException, r))
#define IOBoolResult(e) IOReturn((e) ? nameTrue : nameFalse)
#define HandleArg(nm, offset)                                                  \
    eval(primArg(offset));                                                     \
    nm = intValOf(whnfHead)

#endif /* __BUILTIN_H__ */
