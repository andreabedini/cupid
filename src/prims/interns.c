/* --------------------------------------------------------------------------
 * Implementation of primitives which provide access to internal Cupid
 * data structures and representations from within Haskell.
 *
 * Despite appearances, the only primitives which break referential
 * transparency are the pointer equality primitives.
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

/* --------------------------------------------------------------------------
 * internalPrims control
 * ------------------------------------------------------------------------*/

static void linkInternals(void);

/* nameLeft and nameRight (the standard Left/Right data constructors) are
 * declared in connect.h and defined in type.c; use those declarations
 * rather than redeclaring them here.
 */

/* Predefine the standard Left/Right data constructors this module
 * links its primitives against. */
static void installInternals(void) {
    setCurrModule(modulePrelude);
    nameLeft = predefName("Left");
    nameRight = predefName("Right");
}

/* Re-resolve the primitives' internal name links for the new script. */
static void resetInternals(void) {
    linkInternals();
}

/* The hooks builtIn's INSTALL registers this module under. */
static const Subsystem internalsSubsystem = {
    "internals", installInternals, resetInternals, NULL, NULL, NULL,
};

PROTO_PRIM(primGetCell);
PROTO_PRIM(primCellPtrEq);
PROTO_PRIM(primCatchError2);
PROTO_PRIM(primClassifyCell);
PROTO_PRIM(primNameInfo);
PROTO_PRIM(primNameEq);
PROTO_PRIM(primNameString);

#if BYTECODE_PRIMS
PROTO_PRIM(primNameCode);
PROTO_PRIM(primIntAt);
PROTO_PRIM(primFloatAt);
PROTO_PRIM(primDoubleAt);
PROTO_PRIM(primCellAt);
PROTO_PRIM(primNameAt);
PROTO_PRIM(primBytecodeAt);
PROTO_PRIM(primTextAt);
PROTO_PRIM(primAddrAt);
#endif /* BYTECODE_PRIMS */

static struct primitive internalPrimTable[] = {
    {"getCell", 1, primGetCell},
    {"cellPtrEq", 2, primCellPtrEq},
    {"catchError2", 1, primCatchError2},
    {"classifyCell", 2 + IOArity, primClassifyCell},
    {"nameString", 1, primNameString},
    {"nameInfo", 1, primNameInfo},
    {"nameEq", 2, primNameEq},

#if BYTECODE_PRIMS
    {"nameCode", 1, primNameCode},
    {"intAt", 1, primIntAt},
    {"floatAt", 1, primFloatAt},
    {"doubleAt", 1, primDoubleAt},
    {"cellAt", 1, primCellAt},
    {"nameAt", 1, primNameAt},
    {"bytecodeAt", 1, primBytecodeAt},
    {"textAt", 1, primTextAt},
    {"addrAt", 1, primAddrAt},
#endif /* BYTECODE_PRIMS */

    {0, 0, 0}};

struct primInfo internalPrims = {&internalsSubsystem, internalPrimTable, 0};

/* --------------------------------------------------------------------------
 * linkInternals
 * ------------------------------------------------------------------------*/

static Module moduleInternals;

static Name nameCupidApply;   /* data CellKind = Apply Cell [Cell] */
static Name nameCupidFun;     /*              | Fun   Name        */
static Name nameCupidCon;     /*              | Con   Name        */
static Name nameCupidTuple;   /*              | Tuple Int         */
static Name nameCupidInt;     /*              | Int   Int         */
static Name nameCupidInteger; /*              | Integer Integer   */
static Name nameCupidFloat;   /*              | Float Float       */
static Name nameCupidDouble;  /*              | Double Double     */
static Name nameCupidChar;    /*              | Char  Char        */
static Name nameCupidPrim;    /*              | Prim  String      */
static Name nameCupidError;   /*              | Error Cell        */

static void linkInternals(void) {
    char *internLib = "Cupid.Internals";
    moduleInternals = findModule(findText(internLib));
    if (nonNull(moduleInternals)) {
        setCurrModule(moduleInternals);
        nameCupidApply = findName(findText("Apply"));
        nameCupidFun = findName(findText("Fun"));
        nameCupidCon = findName(findText("Con"));
        nameCupidTuple = findName(findText("Tuple"));
        nameCupidInt = findName(findText("Int"));
        nameCupidInteger = findName(findText("Integer"));
        nameCupidFloat = findName(findText("Float"));
        nameCupidDouble = findName(findText("Double"));
        nameCupidChar = findName(findText("Char"));
        nameCupidPrim = findName(findText("Prim"));
        nameCupidError = findName(findText("Error"));

        if (isNull(nameCupidApply) || isNull(nameCupidFun) ||
            isNull(nameCupidCon) || isNull(nameCupidTuple) ||
            isNull(nameCupidInt) || isNull(nameCupidInteger) ||
            isNull(nameCupidFloat) || isNull(nameCupidDouble) ||
            isNull(nameCupidChar) || isNull(nameCupidPrim) ||
            isNull(nameCupidError)) {
            fatal("module CupidInternals doesn't define CellKind correctly");
        }
    }
}

/* --------------------------------------------------------------------------
 * Operations on Cells
 * ------------------------------------------------------------------------*/

/* Cells have to be boxed (using the CUPIDOBJECT tag) so that we can        */
/* evaluate a Cell expression such as "fst (cell1, cell2)" without         */
/* evaluating the thunk inside the Cell.                                   */

#define isCupidObject(a) (isPair(a) && fst(a) == CUPIDOBJECT)
#define checkCupidObject(x)                                                    \
    if (!isCupidObject(x))                                                     \
    internal("CupidObject expected")

#define CupidObjectArg(nm, offset)                                             \
    eval(primArg(offset));                                                     \
    checkCupidObject(whnfHead);                                                \
    nm = snd(whnfHead);

/* Trivial coercion */
primFun(primGetCell) {
    updapRoot(CUPIDOBJECT, primArg(1)); /*  :: a -> Cell                    */
}

/* --------------------------------------------------------------------------
 * Pointer equality on Cells
 * ------------------------------------------------------------------------*/

/* Unsafe pointer equality test */
primFun(primCellPtrEq) {
    Cell x = 0;
    Cell y = 0;
    CupidObjectArg(x, 1);
    push(x);
    CupidObjectArg(y, 2);
    x = pop();

    /* We chase indirection chains since that's easy and gives more
     * accurate results.  
     * Don't do this if we use this function during error recovery -
     * it might be possible for an infinite loop to occur.
     */
    while (isPair(x) && INDIRECT == fst(x))
        x = snd(x);
    while (isPair(y) && INDIRECT == fst(y))
        y = snd(y);
    updateRoot((x == y) ? nameTrue : nameFalse);
}

/* --------------------------------------------------------------------------
 * Error catching - evaluate a Cell and return an error redex or its value
 * ------------------------------------------------------------------------*/

/* Error catching primitive */
primFun(primCatchError2) {
    /*  :: a -> Either Cell a           */
    Cell err = NIL;
    err = evalWithNoError(primArg(1));
    if (isNull(err)) {
        updapRoot(nameRight, primArg(1));
    } else {
        updapRoot(nameLeft, ap(CUPIDOBJECT, err));
    }
}

/* --------------------------------------------------------------------------
 * Examine an object either with or without evaluation.
 *
 * This primitive lets Haskell programs examine values of any datatype
 * (probably to print them).  It is wired into the IO monad to (narrowly)
 * avoid breaking referential transparency.
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Note: we can't force the object _outside_ classify for two reasons: 
 *                                                                           
 * 1) Writing "strict classifyCell x" only causes x to be evaluated to 
 *    WHNF.  Since x is a Cell, this doesn't achieve very much.      
 *                                                                           
 * 2) We can overcome this by adding an operation                            
 *                                                                           
 *      "forceCell :: Cell -> Cell"                          
 *                                                                           
 *    and writing "classifyObject (forceObject x)" but, if "x" is a CAF, 
 *    then "x" will be updated but the indirection via "x" will remain  
 *    in place.                                                              
 * ------------------------------------------------------------------------*/

/* ClassifyCell */
primFun(primClassifyCell) {
    bool gc = consGC; /*  :: Bool -> Cell -> IO CellKind    */
    bool strict;
    Cell result = NIL;
    BoolArg(strict, 2 + IOArity);
    if (strict) {
        Cell temp;
        eval(IOArg(1)); /* object */
        checkCupidObject(whnfHead);
        whnfHead = snd(whnfHead);
        if (nonNull(temp = evalWithNoError(whnfHead))) {
            IOReturn(ap(nameCupidError, ap(CUPIDOBJECT, temp)));
        }
    } else {
        eval(IOArg(1)); /* object */
        checkCupidObject(whnfHead);
        whnfHead = snd(whnfHead);
        unwind(whnfHead);
    }
    consGC = true;
    if (whnfArgs > 0) {
        Cell args = nameNil;
        while (whnfArgs-- > 0)
            args = ap(ap(nameCons, ap(CUPIDOBJECT, pushed(whnfArgs))), args);
        result = ap(ap(nameCupidApply, ap(CUPIDOBJECT, whnfHead)), args);
    } else {
        switch (whatIs(whnfHead)) {

        case NAME:
            result = ap((isCfun(whnfHead) ? nameCupidCon : nameCupidFun),
                        mkInt(whnfHead));
            break;

        case TUPLE:
            result = ap(nameCupidTuple, mkInt(tupleOf(whnfHead)));
            break;

        case ZERONUM:
        case POSNUM:
        case NEGNUM:
            result = ap(nameCupidInteger, whnfHead);
            break;

        case INTCELL:
            result = ap(nameCupidInt, whnfHead);
            break;

        case CHARCELL:
            result = ap(nameCupidChar, whnfHead);
            break;

        case FLOATCELL:
            result = ap(nameCupidFloat, whnfHead);
            break;

        case DOUBLECELL:
            result = ap(nameCupidDouble, whnfHead);
            break;

        case HANDCELL:
            result = ap(nameCupidPrim, mkStr(findText("{handle}")));
            break;

        case MUTVAR:
            result = ap(nameCupidPrim, mkStr(findText("{ref}")));
            break;

        case ARRAY:
            result = ap(nameCupidPrim, mkStr(findText("{primitive array}")));
            break;

        case CUPIDOBJECT:
            /* ToDo: fix so that debugger can examine itself */
            result = ap(nameCupidPrim, mkStr(findText("{Cell ...}")));
            break;

        case INSTANCE:
            result = ap(nameCupidPrim, mkStr(findText("{instance}")));
            break;
        default:
            internal("Error in graph2");
            break;
        }
    }
    consGC = gc;
    IOReturn(result);
}

/* --------------------------------------------------------------------------
 * Operations on Names
 * ------------------------------------------------------------------------*/

/* Get string of a name */
primFun(primNameString) {
    Text t; /*  :: Name -> String              */
    Name nm;
    IntArg(nm, 1);
    t = nameRec(nm)->text;
    if (0 <= t && t < NUM_TEXT) {
        /* ToDo: replace updateRoot(mkStr(..)) with updapRoot(STRCELL,..) */
        updateRoot(mkStr(t));
    } else {
        /* If textToStr generates result on the fly, we build the entire
       * string now.
       */
        pushString(textToStr(t));
        updateRoot(pop());
    }
}

/* Get arity and fixity of a Name */
primFun(primNameInfo) {
    Name nm; /*  :: Name -> (Int,Int,Char)       */
    Syntax syn = 0;
    Char assoc = 0;
    IntArg(nm, 1);
    syn = syntaxOf(nm);
    switch (assocOf(syn)) {
    case APPLIC:
        assoc = 'A';
        break;
    case LEFT_ASS:
        assoc = 'L';
        break;
    case RIGHT_ASS:
        assoc = 'R';
        break;
    case NON_ASS:
        assoc = 'N';
        break;
    default:
        break;
    }
    updapRoot(ap(ap(mkTuple(3), mkInt(nameRec(nm)->arity)), mkInt(precOf(syn))),
              mkChar(assoc));
}

/* Compare names */
primFun(primNameEq) {
    Name nm1; /*  :: Name -> Name -> Bool         */
    Name nm2;
    IntArg(nm1, 2);
    IntArg(nm2, 1);
    BoolResult(nm1 == nm2);
}

#if BYTECODE_PRIMS

/* Obtain address of first bytecode */
primFun(primNameCode) {
    Name nm; /*  :: Name -> Addr                 */
    IntArg(nm, 1);
    IntResult(nameRec(nm)->code);
}

#endif /* BYTECODE_PRIMS */

/* --------------------------------------------------------------------------
 * Operations on Addresses/Bytecodes
 * ------------------------------------------------------------------------*/

#if BYTECODE_PRIMS

/* Obtain Int at address */
primFun(primIntAt) {
    Addr m; /*  :: Addr -> Int                   */
    IntArg(m, 1);
    IntResult(IntAt(m));
}

/* Obtain float at address */
primFun(primFloatAt) {
    Addr m; /*  :: Addr -> Cell                  */
    IntArg(m, 1);
    FloatResult(FloatAt(m));
}

/* Obtain float at address */
primFun(primDoubleAt) {
    Addr m; /*  :: Addr -> Cell                  */
    IntArg(m, 1);
    DoubleResult(DoubleAt(m));
}

/* Obtain cell at address */
primFun(primCellAt) {
    Addr m; /*  :: Addr -> Cell                  */
    IntArg(m, 1);
    updapRoot(CUPIDOBJECT, CellAt(m));
}

/* Obtain Name at address */
primFun(primNameAt) {
    Addr m; /*  :: Addr -> Name                  */
    IntArg(m, 1);
    IntResult(CellAt(m));
}

/* Obtain bytecode at address */
primFun(primBytecodeAt) {
    Addr m; /*  :: Addr -> Bytecode              */
    IntArg(m, 1);
    IntResult(InstrAt(m));
}

/* Obtain string at address */
primFun(primTextAt) {
    Addr m; /*  :: Addr -> String                */
    IntArg(m, 1);
    updapRoot(STRCELL, TextAt(m));
}

/* Obtain address at address */
primFun(primAddrAt) {
    Addr m; /*  :: Addr -> Addr                  */
    IntArg(m, 1);
    IntResult(AddrAt(m));
}

#endif /* BYTECODE_PRIMS */

/*-------------------------------------------------------------------------*/
