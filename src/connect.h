/* --------------------------------------------------------------------------
 * Connections between components of the Cuddles system
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/
#ifndef __CONNECT_H__
#define __CONNECT_H__

#include "prelude.h"
#include "storage.h"

/* --------------------------------------------------------------------------
 * Standard data:
 * ------------------------------------------------------------------------*/

extern Module modulePrelude;
CUDDLES_inline bool isPrelude(Module m) {
    return m == modulePrelude;
}
extern Module moduleUserPrelude;

/* --------------------------------------------------------------------------
 * Primitive constructor functions 
 * ------------------------------------------------------------------------*/

extern Name nameFalse, nameTrue;
extern Name nameNil, nameCons;
extern Name nameJust, nameNothing;
extern Name nameLeft, nameRight;
extern Name nameUnit;

extern Name nameLT, nameEQ;
extern Name nameGT;
extern Name nameFst, nameSnd; /* standard combinators              */
extern Name nameId, nameOtherwise;
extern Name nameNegate, nameFlip; /* primitives reqd for parsing           */
extern Name nameFrom, nameFromThen;
extern Name nameFromTo, nameFromThenTo;
extern Name nameFatbar, nameFail; /* primitives reqd for translation */
extern Name nameIf, nameSel;
extern Name nameCompAux;
extern Name namePmInt, namePmFlt; /* primitives for pattern matching */
extern Name namePmInteger;
extern Name nameBlackHole;   /* For GC-detected black hole     */
extern Name nameInd;         /* For dict indirection                   */
extern Name nameAnd, nameOr; /* For optimisation of && and ||   */
extern Name nameFromInt, nameFromDouble; /*coercion of numerics            */
extern Name nameFromInteger;
extern Name nameEq, nameCompare; /* names used for deriving        */
extern Name nameMinBnd, nameMaxBnd;
extern Name nameIndex, nameInRange;
extern Name nameRange;
extern Name nameLe, nameGt;
extern Name nameShowsPrec, nameReadsPrec;
extern Name nameMult, namePlus;
extern Name nameConCmp, nameEnRange;
extern Name nameEnIndex, nameEnInRng;
extern Name nameEnToEn, nameEnFrEn;
extern Name nameEnFrom, nameEnFrTh;
extern Name nameEnFrTo;
extern Name nameComp, nameApp;    /* composition and append        */
extern Name nameShowField;        /* display single field                  */
extern Name nameShowParen;        /* wrap with parens              */
extern Name nameReadField;        /* read single field             */
extern Name nameReadParen;        /* unwrap from parens              */
extern Name nameLex;              /* lexer                           */
extern Name nameRangeSize;        /* calculate size of index range   */
extern Class classMonad;          /* Monads                        */
extern Name nameReturn, nameBind; /* for translating do / monad comps*/
extern Name nameThen;             /* for translating do / monad comps*/
extern Name nameMFail;
extern Name nameListMonad; /* builder function for List Monad */

extern Name namePrint;  /* printing primitive              */
extern Name nameNPrint; /* internal printer                */

extern Type typeIO;     /* For the IO monad, IO            */
extern Type typeProgIO; /* For the IO monad, IO ()         */
extern Name nameIORun;  /* IO monad executor               */
extern Name nameIOBind; /* IO bind executor                */
extern Name namePutStr; /* Prelude.putStr                  */

extern Name nameIOError, nameUserErr; /* primitives required for IOError */
extern Name namePermDenied;
extern Name nameAlreadyExists, nameAlreadyInUse, nameDoesNotExist, nameIsFull;
extern Name nameIllegal;

#define IOArity 1 /* arity of IO actions             */

extern Name nameEOFErr;
extern Name nameProtocolError;

extern Name namePrimThrow;

extern Name nameArithException;
extern Name nameArrayException;
extern Name nameErrorCall;
extern Name nameIOException;
extern Name nameNoMethodError;
extern Name nameNonTermination;
extern Name namePatternMatchFail;
extern Name nameRecConError;
extern Name nameRecSelError;
extern Name nameRecUpdError;

extern Name nameOverflow;
extern Name nameDivideByZero;
extern Name nameIndexOutOfBounds;
extern Name nameUndefinedElement;

extern Text textCCall; /* ffi tokens                      */
extern Text textSafe;
extern Text textUnsafe;
extern Text textThreadsafe;
extern Text textExport;
#if STDCALL_SUPPORTED
extern Text textStdcall;
#endif

extern Text textPrelude;
extern Text textUserPrelude;
extern Text textNum; /* used to process default decls   */

extern char *versionString; /* String containing version name  */
#if USE_REGISTRY
extern char *cuddlesRegRoot; /* Root in registry for windows    */
#endif
extern char *scriptFile; /* Name of current script (if any) */

extern Type typeArrow; /* Builtin type constructors        */
extern Type typeList;
extern Type typeUnit;

extern Type typeInt;
extern Type typeInt8;
extern Type typeInt16;
extern Type typeInt32;
extern Type typeInt64;
extern Type typeWord;
extern Type typeWord8;
extern Type typeWord16;
extern Type typeWord32;
extern Type typeWord64;
extern Type typeFunPtr;
extern Type typePtr;
extern Type typeAddr;
extern Type typeFloat;
extern Type typeDouble;
extern Type typeChar;
extern Type typeForeignP;
extern Type typeForeign;
extern Type typeStable;
extern Type typeBool;
extern Type typeString;

#define fn(from, to) ap(ap(typeArrow, from), to) /* make type: from -> to   */

extern List stdDefaults; /* List of standard default types  */

extern Class classEq; /* `standard' classes                */
extern Class classOrd;
extern Class classShow;
extern Class classRead;
extern Class classIx;
extern Class classEnum;
extern Class classBounded;

extern Class classReal; /* `numeric' classes               */
extern Class classIntegral;
extern Class classRealFrac;
extern Class classRealFloat;
extern Class classFractional;
extern Class classFloating;
extern Class classNum;

extern Cell *CStackBase; /* pointer to base of C stack     */

extern List tyconDefns;    /* list of type constructor defns  */
extern List typeInDefns;   /* list of synonym restrictions         */
extern List kindSigDefns;  /* kinds declared for tycons and classes */
extern List valDefns;      /* list of value definitions       */
extern List classDefns;    /* list of class definitions       */
extern List instDefns;     /* list of instance definitions    */
extern List selDefns;      /* list of selector lists       */
extern List genDefns;      /* list of generated defns      */
extern List primDefns;     /* list of primitive definitions   */
extern List unqualImports; /* unqualified import list         */
extern List defaultDefns;  /* default definitions (if any)         */
extern int defaultLine;    /* line in which default defs occur*/
extern List evalDefaults;  /* defaults for evaluator       */
extern Cell inputExpr;     /* evaluator input expression      */
extern Cell inputContext;  /* evaluator input expression      */
extern Addr inputCode;     /* Code for compiled input expr    */

extern int whnfArgs;                 /* number of args of term in whnf  */
extern Cell whnfHead;                /* head of term in whnf            */
extern int whnfInt;                  /* integer value of term in whnf   */
extern Float whnfFloat;              /* float value of term in whnf        */
extern Double whnfDouble;            /* double value of term in whnf       */
extern long numReductions;           /* number of reductions used       */
extern long numCells;                /* number of cells allocated       */
extern int numGcs;                   /* number of garbage collections   */
extern volatile sig_atomic_t broken; /* indicates interrupt received;   */
                                     /* set only from a signal handler  */
extern bool preludeLoaded;           /* true => prelude has been loaded */

/* --------------------------------------------------------------------------
 * Function prototypes etc...
 * ------------------------------------------------------------------------*/

/* The hooks a subsystem offers to the interpreter's lifecycle; a null
 * hook means the subsystem has nothing to do in that phase.  The mark
 * hook is named gcMark, not mark: storage.h's `mark(v)` macro expands
 * any token sequence "mark(" it sees, plain struct-member access
 * included, so a field literally called `mark` cannot be called. */
typedef struct Subsystem {
    const char *name;
    void (*install)(void); /* once, before the Prelude is read */
    void (*reset)(void);   /* before each script load or evaluation */
    void (*gcMark)(void);  /* garbage collection: mark structured data */
    void (*atExit)(void);  /* before exit() */
    void (*onBreak)(void); /* after an interrupt */
} Subsystem;

/* cuddles.c exports: */
extern void shutdownCuddles(void);
extern void promptForInput(char *);
/* The next three are required only by winhugs */
extern bool doCommand(void);
extern void runEditor(void);
extern Module findEvalModule(void);

extern bool startsQual(Char);
extern void consoleInput(char *);
extern void stringInput(char *);
extern bool parseScript(char *, long);
extern void parseScriptString(char *);
extern void parseExp(void);
#if EXPLAIN_INSTANCE_RESOLUTION
extern void parseContext(void);
#endif
extern char *readFilename(void);
extern char *readLine(void);
extern bool isModuleId(char *);
extern Syntax defaultSyntax(Text);
extern Syntax syntaxOf(Name);
extern char *unlexChar(Char, Char);
extern void printString(char *);
extern Char getStrChr(char **);

extern void startModule(Cell);
extern void setExportList(List);
extern void setExports(List);
extern void tyconDefn(int, Cell, Cell, Cell);
extern void kindSigDefn(int, Text, Kind); /* type T :: k                 */
extern void setTypeIns(List);
extern void clearTypeIns(void);
extern Type fullExpand(Type);
extern bool isAmbiguous(Type);
extern void ambigError(int, char *, Cell, Type);
extern void classDefn(int, Cell, List, List);
extern void instDefn(int, Cell, Cell);
extern void addTupInst(Class, int);
extern bool hasIOResultType(Type);
extern List oclose(List, List);
extern List zonkTyvarsIn(Type, List);
extern Type zonkTyvar(int);
extern Type zonkType(Type, int);
extern void primDefn(Cell, List, Cell);
extern void defaultDefn(int, List);
extern void checkExp(void);
#if EXPLAIN_INSTANCE_RESOLUTION
extern void checkContext(void);
#endif
extern void checkDefns(void);
extern Cell depExpr(int, Cell);

extern void foreignImport(Cell, Cell, Cell, Cell, Cell, Type);
extern void foreignExport(Cell, Cell, Cell, Cell, Cell, Type);
extern int foreignCount;
extern List foreignImports; /* foreign import declarations     */
extern List foreignExports; /* foreign export declarations     */

extern void foreignHeader(char *);
extern void foreignFooter(char *, Text, List, List);
extern void ffiSetFlags(char *);
extern void ffiAddCppInclude(char *);
extern void implementForeignImport(int, Name, int, Text, Text, bool, Text, List,
                                   bool, Type);
extern void implementForeignImportDynamic(int, int, Text, List, bool, Type);
extern void implementForeignImportWrapper(int, int, Text, List, bool, Type);
extern void implementForeignImportLabel(int, int, Text, Text, Text, Type);
extern void implementForeignExport(int, int, Text, List, bool, Type);
extern bool foreignNeedStubs(List, List);
extern bool generateFFI;  /* running fficuddles? */
extern bool generate_ffi; /* generate FFI for the current module? */

extern void installTypeChecker(void);
extern void resetTypeChecker(void);
extern void markTypeChecker(void);
extern Type typeCheckExp(bool);
extern void typeCheckDefns(void);
extern Cell provePred(Kinds, List, Cell);
extern Cell resolvePred(Kinds, Cell);
extern List simpleContext(List, int);
extern Cell rhsExpr(Cell);
extern int rhsLine(Cell);
extern Cell getProgType(List, Type);
extern Cell superEvid(Cell, Class, Class);
extern void linkPreludeTC(void);
extern void linkPreludeCM(void);
extern void linkPreludeFuns(void);

extern void installCompiler(void);
extern void resetCompiler(void);
extern void compileDefns(void);
extern void compileExp(void);
extern bool failFree(Cell);
extern int discrArity(Cell);

extern void installMachine(void);
extern void resetMachine(void);
extern void exitMachine(void);
extern Addr codeGen(Name, int, Cell);
extern void implementCfun(Name, List);
extern void addCfunTable(Tycon);
extern Name succCfun(Name);
extern Name nextCfun(Name, Name);
extern Name cfunByNum(Name, int);
extern void unwind(Cell);
extern void run(Addr, StackPtr);
extern DynTable *dynMemory;
extern void growMemory(void);

extern void eval(Cell);
extern Cell evalWithNoError(Cell);
extern void evalFails(StackPtr);
extern void throwException(Cell) CUDDLES_noreturn;

#if BYTECODE_PRIMS
extern int IntAt(Addr);
extern Float FloatAt(Addr);
extern Double DoubleAt(Addr);
extern Cell CellAt(Addr);
extern Text TextAt(Addr);
extern Addr AddrAt(Addr);
extern int InstrAt(Addr);
#endif /* BYTECODE_PRIMS */

extern void abandon(char *, Cell);
extern void outputString(FILE *);
extern void dialogue(Cell);
extern Cell consChar(Char);
extern Bignum bigInt(int);
extern Bignum bigInt64(HsInt64);
extern Bignum bigWord(unsigned);
extern Bignum bigDouble(double);
extern Bignum bigNeg(Bignum);
extern Cell bigToInt(Bignum);
extern double bigToDouble(Bignum);
extern Bignum bigStr(char *);
extern Cell bigOut(Bignum, Cell, bool);
extern Bignum bigShift(Bignum, int, int);
extern int bigCmp(Bignum, Bignum);
extern void setCuddlesArgs(int, char *[]);

extern char *findPathname(char *);
extern char *findMPathname(char *);
extern char *findMInDir(char *, char *);
extern bool readable(char *, bool);
#if PROFILING
extern char *timeString(void);
#endif
extern char *cuddlesdir(void);
extern char *mkFFIFilename(char *);
extern char *mkFFIFilename2(char *);
extern void freeDLL(void *);
extern void compileAndLink(char *, char *);

extern List calcFunDepsPreds(List);

extern void pushString(char *);

/*-------------------------------------------------------------------------*/

#endif /* __CONNECT_H__ */
