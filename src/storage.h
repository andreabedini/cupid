/* --------------------------------------------------------------------------
 * Defines storage datatypes: Text, Name, Module, Tycon, Cell, List, Pair,
 * Triple, ...
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/
#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "prelude.h"
#include <limits.h> /* UCHAR_MAX, for the assertions at the end */

/* --------------------------------------------------------------------------
 * Typedefs for main data types:
 * Many of these type names are used to indicate the intended us of a data
 * item, rather than for type checking purposes.  Sadly (although sometimes,
 * fortunately), the C compiler cannot distinguish between the use of two
 * different names defined to be synonyms for the same types.
 * ------------------------------------------------------------------------*/

typedef int Text;               /* text string     */
typedef unsigned Syntax;        /* syntax (assoc,preced)  */
typedef int Addr;               /* address of code         */
typedef int Cell;               /* general cell value      */
typedef Cell *Heap;             /* storage of heap         */
typedef Cell Pair;              /* pair cell               */
typedef int StackPtr;           /* stack pointer           */
typedef Cell Offset;            /* offset/generic variable*/
typedef int Script;             /* script file number      */
typedef int Module;             /* module                  */
typedef Cell Tycon;             /* type constructor        */
typedef Cell Type;              /* type expression         */
typedef Cell Kind;              /* kind expression         */
typedef Cell Kinds;             /* list of kinds           */
typedef Cell Constr;            /* constructor expression */
typedef Cell Name;              /* named value     */
typedef void (*Prim)(StackPtr); /* primitive function      */
typedef Cell Class;             /* type class              */
typedef Cell Inst;              /* instance of type class */
typedef Cell Triple;            /* triple of cell values  */
typedef Cell List;              /* list of cells           */
typedef Cell Bignum;            /* bignum integer          */
typedef FloatImpType Float;     /* implementation of Float*/
typedef DoubleImpType Double;   /* implementation of Double*/

/* --------------------------------------------------------------------------
 * Text storage:
 * provides storage for the characters making up identifier and symbol
 * names, string literals, character constants etc...
 * ------------------------------------------------------------------------*/

extern char *textToStr(Text);
extern Text findText(char *);
extern Text inventText(void);
extern Text inventDictText(void);
extern bool inventedText(Text);
extern char *identToStr(Cell);
extern Text fixLitText(Text);
extern Text concatText(char *, char *);
extern Text subText(char *, int);

/* --------------------------------------------------------------------------
 * Specification of syntax (i.e. default written form of application)
 * ------------------------------------------------------------------------*/

#define MIN_PREC 0              /* weakest binding operator        */
#define MAX_PREC 9              /* strongest binding operator      */
#define FUN_PREC (MAX_PREC + 2) /* binding of function symbols     */
#define DEF_PREC MAX_PREC
#define APPLIC 0    /* written applicatively       */
#define LEFT_ASS 1  /* left associative infix      */
#define RIGHT_ASS 2 /* right associative infix     */
#define NON_ASS 3   /* non associative infix       */
#define DEF_ASS LEFT_ASS

#define UMINUS_PREC 6         /* Change these settings at your   */
#define UMINUS_ASSOC LEFT_ASS /* own risk; they may not work!      */

CUPID_inline Syntax assocOf(Syntax x) {
    return x & NON_ASS;
}
CUPID_inline int precOf(Syntax x) {
    return (int)(x >> 2);
}
CUPID_inline Syntax mkSyntax(Syntax a, int p) {
    return a | ((Syntax)p << 2);
}
#define DEF_OPSYNTAX mkSyntax(DEF_ASS, DEF_PREC)
#define NO_SYNTAX ((Syntax)(-1))

#define FFI_NOSAFETY 0
#define FFI_SAFE 1
#define FFI_UNSAFE 2
#define FFI_THREADSAFE 4

/* Note: cconv flags are combined with the above safety flags,
 * so need to be disjoint.
 */
#define FFI_CCONV_UNKNOWN 0
#define FFI_CCONV_CCALL 8
#define FFI_CCONV_STDCALL 16

/* --------------------------------------------------------------------------
 * Primitive functions:
 * ------------------------------------------------------------------------*/

struct primitive { /* entry in table of primitives         */
    char *ref;     /* primitive reference string           */
    int arity;     /* primitive function arity     */
    Prim imp;      /* primitive implementation     */
};

/* Modules that define new primitive functions may register a Subsystem
 * of lifecycle hooks (connect.h; a null pointer here means it has none)
 * and a (null-terminated) table of primitive functions.
 *
 * They are stored as a linked list - so there's no wired in limits.
 * A phase's hooks are called in the order the modules were registered,
 * after every other subsystem's hook for that phase has been called.
 * (At the moment) there's no way of unregistering a module.
 */
struct Subsystem; /* connect.h; storage.h only needs a pointer to it */

struct primInfo {
    const struct Subsystem *hooks;
    struct primitive *primFuns;
    struct primInfo *nextPrimInfo;
};

extern void registerPrims(struct primInfo *);
extern void installControlFuns(void); /* call every registered module's */
extern void resetControlFuns(void);   /* hook for the phase named, in   */
extern void markControlFuns(void);    /* registration order, skipping   */
extern void exitControlFuns(void);    /* modules with no hook for it    */
extern void breakControlFuns(void);
extern struct primInfoDef *setPrimInfoDll(void *);

/* --------------------------------------------------------------------------
 * Program code storage: for holding compiled function defns etc...
 * ------------------------------------------------------------------------*/

extern Addr getMem(int);
extern void nextInstr(Addr);

/* --------------------------------------------------------------------------
 * Heap storage:
 * Provides a garbage collectable heap for storage of expressions etc.
 * ------------------------------------------------------------------------*/

#define heapAlloc(s) (Heap)(farCalloc(s, sizeof(Cell)))
extern int heapSize;
extern Heap heapFst, heapSnd;
CUPID_inline bool heapBuilt(void) {
    return heapFst != 0;
}
extern Heap heapTopFst;
extern Heap heapTopSnd;
extern bool consGC;        /* Set to false to turn off gc from*/
                           /* C stack; use with extreme care! */
extern int cellsRecovered; /* cells recovered by last gc           */

#define fst(c) heapTopFst[c]
#define snd(c) heapTopSnd[c]
#if PROFILING
extern Heap heapThd, heapTopThd;
# define thd(c) heapTopThd[c]
extern Name producer;
extern bool profiling;
extern int profInterval;
extern void profilerLog(char *);
#endif

extern Pair pair(Cell, Cell);
extern void garbageCollect(void);

extern void installStorage(void); /* allocate the heaps and handle table */
extern void resetStorage(void);   /* reset the heap pointers and handles */
extern void markStorage(void);    /* mark the symbol tables and stack    */
extern void exitStorage(void);    /* free the heaps and handle table     */

extern void overwrite(Pair, Pair);
extern Cell markExpr(Cell);
extern void markWithoutMove(Cell);

#define mark(v) v = markExpr(v)

/* A static that holds a cell is a root.  A module lists the addresses of
 * its roots in a table and registers it once, at INSTALL; the collector
 * then marks them on every collection.  MARK cases remain only for data
 * a flat table cannot describe: the symbol tables, the substitution, the
 * type checker's scope stack. */
extern void gcRegisterRoots(Cell *const *, int);
#define registerRoots(t) gcRegisterRoots((t), (int)(sizeof(t) / sizeof((t)[0])))

CUPID_inline bool isPair(Cell c) {
    return c < 0;
}
CUPID_inline bool isGenPair(Cell c) {
    return c < 0 && -heapSize <= c;
}

extern unsigned char whatCode[]; /* whatIs code of each atom; see   */
                                 /* whatIs below the tag definitions*/

/* --------------------------------------------------------------------------
 * Box cell tags are used as the fst element of a pair to indicate that
 * the snd element of the pair is to be treated in some special way, other
 * than as a Cell.  Examples include holding integer values, variable name
 * and string text etc.
 * ------------------------------------------------------------------------*/

#define TAGMIN 1         /* Box and constructor cell tag values   */
#define BCSTAG 30        /* Box=TAGMIN..BCSTAG-1                   */
#define TAG_MASK (~0x7f) /* This masks 0..SPECMIN-1                */

/* Every tag and special value, so that whatIs() has a type the compiler can
 * check switches against.  The numeric values are load-bearing: isTag,
 * isBoxTag, isConTag and the whatCode table are defined by these ranges. */
typedef enum CellTag {
    INDIRECT = 1,   /* Indirection node:              snd :: Cell  */
    INDIRECT1 = 2,  /* Temporary indirection:   snd :: Cell  */
    FREECELL = 3,   /* Free list cell:        snd :: Cell  */
    VARIDCELL = 4,  /* Identifier variable:     snd :: Text  */
    VAROPCELL = 5,  /* Operator variable:       snd :: Text  */
    DICTVAR = 6,    /* Dictionary variable:     snd :: Text  */
    CONIDCELL = 7,  /* Identifier constructor:  snd :: Text  */
    CONOPCELL = 8,  /* Operator constructor:    snd :: Text  */
    STRCELL = 9,    /* String literal:        snd :: Text  */
    INTCELL = 10,   /* Int literal:           snd :: Int   */
    HANDCELL = 12,  /* IO monad Handle:       snd :: Int   */
    WEAKCELL = 13,  /* WeakPtr Cell:          snd::(Cell,List)*/
    WEAKFIN = 14,   /* Weak pointer with fin:   snd::(k,(v,f))*/
    FLOATCELL = 15, /* Floating pt number:      snd :: Float */
    MPCELL = 17,    /* Malloc Ptr Cell:         snd :: Int   */
#if SIZEOF_INTP == SIZEOF_INT
    PTRCELL = 18, /* C Heap Pointer:          snd ::Pointer*/
#endif
    /* CHARCELL is the whatIs code for chars; if UNICODE_CHARS is set, it
 * is also used for box cells representing character values greater than
 * or equal to NUM_SMALL_CHARS, in the same way as INTCELL.
 */
    CHARCELL = 22, /* Char literal:           snd :: Char  */

    /* Constructor cell tags (BCSTAG..SPECMIN-1): the fst of a pair that
     * describes a syntactic construct; see the note above isAp. */

    LETREC = 30,  /* LETREC     snd :: ([Decl],Exp)        */
    COND = 31,    /* COND       snd :: (Exp,Exp,Exp)       */
    LAMBDA = 32,  /* LAMBDA     snd :: Alt                 */
    FINLIST = 33, /* FINLIST    snd :: [Exp]               */
    DOCOMP = 34,  /* DOCOMP     snd :: (Exp,[Qual])        */

    BANG = 35,    /* BANG       snd :: Type                */
    COMP = 36,    /* COMP       snd :: (Exp,[Qual])        */
    ASPAT = 37,   /* ASPAT      snd :: (Var,Exp)           */
    ESIGN = 38,   /* ESIGN      snd :: (Exp,Type)          */
    RSIGN = 39,   /* RSIGN      snd :: (Rhs,Type)          */
    CASE = 40,    /* CASE       snd :: (Exp,[Alt])         */
    NUMCASE = 41, /* NUMCASE    snd :: (Exp,Disc,Rhs)      */
    FATBAR = 42,  /* FATBAR     snd :: (Exp,Exp)           */
    LAZYPAT = 43, /* LAZYPAT    snd :: Exp                 */
    DERIVE = 45,  /* DERIVE     snd :: Cell                */

    POSNUM = 47, /* POSNUM      snd :: [Int]               */
    NEGNUM = 48, /* NEGNUM      snd :: [Int]               */

    BOOLQUAL = 49,  /* BOOLQUAL snd :: Exp                 */
    QWHERE = 50,    /* QWHERE   snd :: [Decl]              */
    FROMQUAL = 51,  /* FROMQUAL snd :: (Exp,Exp)           */
    DOQUAL = 52,    /* DOQUAL   snd :: Exp                 */
    MONADCOMP = 53, /* MONADCOMP        snd :: ((m,m0),(Exp,[Qual])*/

    GUARDED = 54, /* GUARDED    snd :: [guarded exprs]     */

    ARRAY = 55,       /* Array snd :: (Bounds,[Values])   */
    MUTVAR = 56,      /* Mutvar        snd :: Cell                */
    CUPIDOBJECT = 57, /* CUPIDOBJECT  snd :: Cell                */

    STABLENAME = 58, /* Stable Nm  snd :: Cell             */

    POLYTYPE = 60, /* POLYTYPE  snd :: (Kind,Type)         */
    QUAL = 61,     /* QUAL      snd :: ([Classes],Type)    */
    RANK2 = 62,    /* RANK2     snd :: (Int,Type)          */
    EXIST = 63,    /* EXIST     snd :: (Int,Type)          */
    POLYREC = 64,  /* POLYREC   snd :: (Int,Type)          */
    BIGLAM = 65,   /* BIGLAM    snd :: (vars,patterns)     */
    CDICTS = 66,   /* CDICTS    snd :: ([Pred],Type)       */
    KINDED = 67,   /* KINDED    snd :: (Var,Kind): a binder with a kind */

    LABC = 70,    /* LABC       snd :: (con,[(Vars,Type)]) */
    CONFLDS = 71, /* CONFLDS    snd :: (con,[Field])       */
    UPDFLDS = 72, /* UPDFLDS    snd :: (Exp,[con],[Field]) */
    IMPDEPS = 78, /* IMPDEPS    snd :: [Binding]           */

    QUALIDENT = 80, /* Qualified identifier  snd :: (Id,Id)  */
    HIDDEN = 81,    /* hiding import list    snd :: [Entity] */
    MODULEENT = 82, /* module in export list snd :: con      */

    INFIX = 90, /* INFIX        snd :: (see tidyInfix)     */
    ONLY = 91,  /* ONLY snd :: Exp                 */
    NEG = 92,   /* NEG  snd :: Exp                 */

    DOUBLECELL = 97, /* DOUBLECELL snd :: (Int,Int)        */
    I64CELL = 98,    /* Int/Word64 snd :: (Int,Int)           */

#if SIZEOF_INTP != SIZEOF_INT
    PTRCELL = 99, /* C Heap Pointer snd :: (Int,Int)       */
#endif

    /* Special cell values (SPECMIN..TUPMIN-1): atoms that are also their own
     * whatIs codes. */

    NONE = 129,       /* Dummy stub                                  */
    STAR = 130,       /* Representing the kind of types      */
    CONSTRAINT = 131, /* The kind of a class, in a kind signature */
    WILDCARD = 132,   /* Wildcard pattern                    */

    SKOLEM = 133, /* Skolem constant                       */

    DOTDOT = 134,   /* ".." in import/export list            */
    EQUALITY = 135, /* The ~ of an equality predicate t1 ~ t2, as its head */

    ZERONUM = 136, /* The zero bignum (see POSNUM, NEGNUM)  */

    NAME = 138,     /* whatIs code for isName              */
    TYCON = 139,    /* whatIs code for isTycon             */
    CLASS = 140,    /* whatIs code for isClass             */
    MODULE = 141,   /* whatIs code for isModule              */
    INSTANCE = 142, /* whatIs code for isInst                */
    TUPLE = 143,    /* whatIs code for tuple constructor           */
    OFFSET = 144,   /* whatis code for offset              */
    AP = 145,       /* whatIs code for application node    */

    SIGDECL = 148, /* Signature declaration                */
    FIXDECL = 149, /* Fixity declaration                           */
    FUNBIND = 150, /* Function binding                     */
    PATBIND = 151, /* Pattern binding                      */

    DATATYPE = 158,    /* Datatype type constructor                */
    NEWTYPE = 159,     /* Newtype type constructor                 */
    SYNONYM = 160,     /* Synonym type constructor                 */
    RESTRICTSYN = 161, /* Synonym with restricted scope    */

    NODEPENDS = 163,  /* Stop calculation of deps in type check*/
    PREDEFINED = 164, /* Predefined name, not yet filled       */
} CellTag;

extern bool isVar(Cell);
extern bool isCon(Cell);
extern bool isQVar(Cell);
extern bool isQCon(Cell);
extern bool isQualIdent(Cell);
extern bool isIdent(Cell);

extern Cell mkFloat(FloatPro);
extern FloatPro floatOf(Cell);
extern char *floatToString(FloatPro);

extern Cell mkDouble(DoublePro);
extern DoublePro doubleOf(Cell);
extern char *doubleToString(DoublePro);
extern DoublePro stringToDouble(char *);
extern Cell part1Double(DoublePro);
extern Cell part2Double(DoublePro);
extern DoublePro doubleFromParts(Cell, Cell);

extern Cell mkPtr(void *);
extern void *ptrOf(Cell);

/* --------------------------------------------------------------------------
 * Resizeable table struct type.
 * ------------------------------------------------------------------------*/
typedef struct strDynTable {
    /* "data" holds "maxIdx*eltSize" bytes.
   * "maxIdx" may not grow beyond "hWater", unless "hWater" is 0,
   * in which case "maxIdx" does not have an upper bound.
   */
    unsigned long maxIdx;
    unsigned long hWater;
    unsigned long eltSize;
    const char *tabName;
    void *data;
} DynTable;

#define IdxDynTable(eltTy, tab, idx) (((eltTy *)(tab->data))[idx])
extern DynTable *allocDynTable(unsigned long, unsigned long, unsigned long,
                               const char *);
extern void freeDynTable(DynTable *);
extern void growDynTable(DynTable *);

/* --------------------------------------------------------------------------
 * Constructor cell tags are used as the fst element of a pair to indicate
 * a particular syntactic construct described by the snd element of the
 * pair.
 * Note that a cell c will not be treated as an application (AP/isAp) node
 * if its first element is a constructor cell tag, whereas a cell whose fst
 * element is a special cell will be treated as an application node.
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Special cell values:
 * ------------------------------------------------------------------------*/

#define SPECMIN 129 /* must be out of TAG_MASK range         */

/* --------------------------------------------------------------------------
 * Tuple data/type constructors:
 * ------------------------------------------------------------------------*/

#define TUPMIN 201
/* mkTuple(n) is called with an unchecked component count wherever a tuple
 * literal or type is built (including the parser's tuple grammar rule, one
 * comma at a time); an n at or beyond NUM_TUPLES would alias the OFFSET
 * range below and confuse anything that later looks at the cell's tag, so
 * tooManyTupleComponents (machine.c) reports a clean error instead.
 */
extern void tooManyTupleComponents(void);
static inline Cell mkTuple(int n) {
    if (n >= NUM_TUPLES)
        tooManyTupleComponents();
    return TUPMIN + n;
}

/* --------------------------------------------------------------------------
 * Offsets: (generic types/stack offsets)
 * ------------------------------------------------------------------------*/

#define OFFMIN (TUPMIN + NUM_TUPLES)
/* mkOffset(o), similarly, is called with an unchecked offset wherever the
 * compiler hands out a stack/generic-type offset; an o at or beyond
 * NUM_OFFSETS would alias the MODMIN range, so tooManyOffsets (machine.c)
 * reports a clean error instead of letting the aliased cell corrupt later
 * processing (as "INTERNAL ERROR: lift").
 */
extern void tooManyOffsets(void);
static inline Cell mkOffset(int o) {
    if (o >= NUM_OFFSETS)
        tooManyOffsets();
    return OFFMIN + o;
}

/* --------------------------------------------------------------------------
 * Observations
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Modules:
 * ------------------------------------------------------------------------*/

#define MODMIN (OFFMIN + NUM_OFFSETS)

/* Under Haskell 1.3, the list of qualified imports is always a subset
 * of the list of unqualified imports.  For simplicity and flexibility,
 * we do not attempt to exploit this fact - when a module is imported
 * unqualified, it is added to both the qualified and unqualified
 * import lists.
 * Similarily, Haskell 1.3 does not allow a constructor to be imported
 * or exported without exporting the type it belongs to but the export
 * list is just a flat list of Texts (before static analysis) or
 * Tycons, Names and Classes (after static analysis).
 */
struct strModule {
    Text text;
    /* Lists of top level objects (local defns + imports)                       */
    /* Invariant: a name/tycons only appears once (i.e., no dups) in this list. */
    List tycons;
    List names;
    List classes;
    List exports; /* [ Entity | (Entity, NIL|DOTDOT|[Entity]) ] */
    /* (alias,modName) list of local aliases. Used both during compilation and when
     * evaluating an expression in the context of the current module.
     */
    List modAliases;
    /* List of qualified imports.  Used both during compilation and when
     * evaluating an expression in the context of the current module.
     */
    List qualImports; /* [ (modName{-Text-},[Entity]) ] */
    /* NOTE: static.c:checkQualImport() modifies 'modName' to a Module */

    /* For each module imported, record the names that was
     * effectively imported, taking into consideration
     * import lists and 'hiding's. Used to handle re-exportation
     * of modules only -- that is, if a subset S of module A is
     * imported in module B, then the presence of 'module A' in
     * the export list of B, means that S should be re-exported
     * from B, not all of A.
     *
     * The bool in the second pair is used to optimise the handling
     * of a module that's only imported once, but constrained by
     * a 'hiding'.
     */
    List modImports; /* :: [ (modName{-Module-},(Bool,[Entity])) ] */
};

extern Module currentModule; /* Module currently being processed */
extern struct strModule DECTABLE(tabModule);

extern bool isValidModule(Module);
extern Module newModule(Text);
extern Module findModule(Text);
extern Module findModid(Cell);
extern void setCurrModule(Module);
extern List getModuleImports(Module);

/* --------------------------------------------------------------------------
 * Type constructor names:
 * ------------------------------------------------------------------------*/

#define TYCMIN (MODMIN + NUM_MODULE)

struct strTycon {
    Text text;
    int line;
    Module mod; /* module that defines it          */
    int arity;
    Kind kind; /* kind (includes arity) of Tycon  */
    Cell what; /* DATATYPE/SYNONYM/RESTRICTSYN... */
    Cell defn;
    Tycon nextTyconHash;
    List clashes;
};

extern struct strTycon DECTABLE(tabTycon);

extern Tycon newTycon(Text);
extern Tycon findTycon(Text);
extern Tycon addTycon(Tycon);
extern void removeTycon(Tycon);
extern Tycon findQualTycon(Cell);
extern Tycon addPrimTycon(Text, Kind, int, Cell, Cell);

#define polySigOf(t) fst(snd(t))
#define monotypeOf(t) snd(snd(t))

/* --------------------------------------------------------------------------
 * Globally defined name values:
 * ------------------------------------------------------------------------*/

#define NAMEMIN (TYCMIN + NUM_TYCON)

struct strName {
    Text text;
    int line;
    Module mod; /* module that defines it          */
    Syntax syntax;
    Cell parent; /* enclosing object                */
                 /* :: Class | Instance | Name | NIL*/
    int arity;
    int number;
    Cell type;
    Cell defn;
    Addr code;
    Text extFun;      /* for foreign import/export       */
    int foreignId;    /* per module identifier           */
    int foreignFlags; /* encoding safety + calling conv  */
    Prim primDef;
    Name nextNameHash;
#if PROFILING
    int count;
#endif
    List clashes;
};

extern struct strName DECTABLE(tabName);

/* The number field in a name is used to distinguish various kinds of name:
 *   mfunNo(i) = code for member function, offset i
 *               members that are sole elements of dict use mfunNo(0)
 *               members of dicts with more than one elem use mfunNo(n), n>=1
 *   EXECNAME  = code for executable name (bytecodes or primitive)
 *   SELNAME   = code for selector function
 *   DFUNNAME  = code for dictionary builder or selector
 *   cfunNo(i) = code for data constructor
 *               datatypes with only one constructor uses cfunNo(0)
 *               datatypes with multiple constructors use cfunNo(n), n>=1
 */

#define EXECNAME 0
#define SELNAME 1
#define DFUNNAME 2
#define CFUNNAME 3

extern Name newName(Text, Cell);
extern Name findName(Text);
extern Name addName(Name);
extern void removeName(Name);
extern Name findQualName(Cell);
extern List findQualNames(Cell);
extern Name findQualFun(Text, Text);
extern void addPrim(int, Name, char *, Module, Type);
extern Name addPrimCfun(Text, int, int, Cell);
extern int sfunPos(Name, Name);
extern bool setOldDLLFlag(bool);

/* --------------------------------------------------------------------------
 * Type class values:
 * ------------------------------------------------------------------------*/

#define INSTMIN (NAMEMIN + NUM_NAME) /* instances                          */

struct strInst {
    Class c; /* class C                    */
    int line;
    Kinds kinds;      /* Kinds of variables in head        */
    Cell head;        /* :: Pred                           */
    List specifics;   /* :: [Pred]                         */
    int numSpecifics; /* length(specifics)                 */
    List implements;
    Name builder; /* Dictionary constructor function */
};

/* a predicate (an element :: Pred) is an application of a Class to one or
 * more type expressions
 */

#define CLASSMIN (INSTMIN + NUM_INSTS)

struct strClass {
    Text text;      /* Name of class               */
    int line;       /* Line where declaration begins   */
    Module mod;     /* module that declares it         */
    int level;      /* Level in class hierarchy    */
    int arity;      /* Number of arguments                 */
    List tyvars;    /* Names of tyvars             */
    Kinds kinds;    /* Kinds of constructors in class  */
    Cell head;      /* Head of class               */
    List fds;       /* Functional Dependencies     */
    List xfds;      /* Xpanded Functional Dependencies */
    Name dcon;      /* Dictionary constructor function */
    List supers;    /* :: [Pred]                           */
    int numSupers;  /* length(supers)              */
    List dsels;     /* Superclass dictionary selectors */
    List members;   /* :: [Name]                           */
    int numMembers; /* length(members)             */
    List defaults;  /* :: [Name]                           */
    List instances; /* :: [Inst]                           */
    List clashes;   /* Conflicting classes/tycons      */
};

extern struct strClass DECTABLE(tabClass);
extern struct strInst DECTABLE(tabInst);

extern Class newClass(Text);
extern Class classMax(void);
extern Class findClass(Text);
extern Class addClass(Class);
extern Class findQualClass(Cell);
extern Inst newInst(void);
extern Inst findFirstInst(Tycon);
extern Inst findNextInst(Tycon, Inst);
extern Module findQualifier(Text);
extern List findQualifiers(Text);
extern Text findModAlias(Text);

/* --------------------------------------------------------------------------
 * Character values:
 * ------------------------------------------------------------------------*/

#define CHARMIN (CLASSMIN + NUM_CLASSES)

/* --------------------------------------------------------------------------
 * Unicode Characters: NUM_SHORT_CHARS is the number of one-cell characters.
 * Such cells are recognized by the new predicate isShortChar().
 * Larger character values are stored as boxed values, in the same way
 * as integers, with predicate, selector and constructor functions.
 * ------------------------------------------------------------------------*/

#define NUM_SHORT_CHARS 1280 /* number of one-cell characters           */
/* (Latin, Greek and Cyrillic)             */

extern bool isChar(Cell);
extern int charOf(Cell);
extern Cell mkChar(int);

/* --------------------------------------------------------------------------
 * Small Integer values:
 * ------------------------------------------------------------------------*/

#define INTMIN (CHARMIN + NUM_SHORT_CHARS)

/* All ints out of the range defined below are boxed, so we want the biggest
 * INTMAX possible. But the bigger INTMAX is, the bigger the whatIs array.
 * We strike a balance by choosing an integer range of 4096.
 */

#define INTMAX (INTMIN + 4096)
#define N_WHATCODE (INTMAX + 1) /* size of whatCode array */

#define INTZERO (INTMIN / 2 + INTMAX / 2)
#define MINSMALLINT (INTMIN - INTZERO)
#define MAXSMALLINT (INTMAX - INTZERO)

extern bool isInt(Cell);
extern int intOf(Cell);
extern Cell mkInt(int);
extern bool isBignum(Cell);

/* --------------------------------------------------------------------------
 * Implementation of triples:
 * ------------------------------------------------------------------------*/

#define fst3(c) fst(c)
#define snd3(c) fst(snd(c))
#define thd3(c) snd(snd(c))

/* --------------------------------------------------------------------------
 * Implementation of lists:
 * ------------------------------------------------------------------------*/

#define NIL 0
#define hd(c) fst(c)
#define tl(c) snd(c)

extern int length(List);
extern List appendOnto(List, List); /* destructive         */
extern List dupOnto(List, List);
extern List dupList(List);
extern List dupUpto(List, Cell);
extern List revOnto(List, List); /* destructive    */
CUPID_inline List rev(List xs) {
    return revOnto(xs, NIL);
} /* non-destructive */

/* --------------------------------------------------------------------------
 * Predicates, projections and constructors on cells.  These were macros;
 * as functions they have types, evaluate their argument once, and can be
 * stepped into.  fst, snd, hd, tl and the table accessors stay macros
 * because they are assigned through.
 * ------------------------------------------------------------------------*/

CUPID_inline bool isTag(Cell c) {
    return TAGMIN <= c && c < SPECMIN;
}
CUPID_inline bool isBoxTag(Cell c) {
    return TAGMIN <= c && c < BCSTAG;
}
CUPID_inline bool isConTag(Cell c) {
    return BCSTAG <= c && c < SPECMIN;
}
CUPID_inline bool isTag1(Cell c) {
    return (c & TAG_MASK) == 0;
} /* NIL too */

CUPID_inline CellTag whatIs(Cell c) {
    return (CellTag)(isPair(c) ? (isTag(fst(c)) ? fst(c) : AP) : whatCode[c]);
}
/* Faster; NIL if fst(c) is NIL */
CUPID_inline CellTag whatIs1(Cell c) {
    return (CellTag)(isPair(c) ? (isTag1(fst(c)) ? fst(c) : AP) : whatCode[c]);
}
CUPID_inline bool isAp(Cell c) {
    return isPair(c) && !isTag(fst(c));
}
CUPID_inline bool isAp1(Cell c) {
    return isPair(c) && !isTag1(fst(c));
}

CUPID_inline Cell ap(Cell f, Cell x) {
    return pair(f, x);
}

CUPID_inline bool isNull(Cell c) {
    return c == NIL;
}
CUPID_inline bool nonNull(Cell c) {
    return c != NIL;
}

CUPID_inline bool isTuple(Cell c) {
    return TUPMIN <= c && c < OFFMIN;
}
CUPID_inline int tupleOf(Cell c) {
    return (int)(c - TUPMIN);
}
CUPID_inline bool isOffset(Cell c) {
    return OFFMIN <= c && c < MODMIN;
}
CUPID_inline int offsetOf(Cell c) {
    return c - OFFMIN;
}
CUPID_inline bool isModule(Cell c) {
    return MODMIN <= c && c < TYCMIN;
}
CUPID_inline bool isTycon(Cell c) {
    return TYCMIN <= c && c < NAMEMIN;
}
CUPID_inline bool isName(Cell c) {
    return NAMEMIN <= c && c < INSTMIN;
}
CUPID_inline bool isInst(Cell c) {
    return INSTMIN <= c && c < CLASSMIN;
}
CUPID_inline bool isClass(Cell c) {
    return CLASSMIN <= c && c < CHARMIN;
}
CUPID_inline bool isShortChar(Cell c) {
    return CHARMIN <= c && c < INTMIN;
}
CUPID_inline bool isSmall(Cell c) {
    return INTMIN <= c;
}

/* --------------------------------------------------------------------------
 * The record behind a Module, Tycon, Name, Inst or Class.
 *
 * A handle stays a Cell in the range its table occupies.  These turn one
 * into a pointer to its record, so that a field of it is an lvalue, which
 * is what kept the accessors macros until now.  Under -Dchecks each
 * one first checks that the cell really is a handle into that table; the
 * macros indexed whatever they were given.  The tables are fixed arrays,
 * so a record does not move and the pointer stays good.
 * ------------------------------------------------------------------------*/

/* Reports a cell that is not the handle the accessor expected.  Defined in
 * tagcheck.c and also used by the assert* macros in tagcheck.h; declared
 * here rather than there because tagcheck.h needs Cell from this header,
 * and only one of the two may depend on the other. */
extern void tagError(const char *, const char *, Cell) CUPID_noreturn;

CUPID_inline struct strModule *moduleRec(Module m) {
#if CHECK_TAGS
    if (!isModule(m))
        tagError("module", __func__, m);
#endif
    return tabModule + (m - MODMIN);
}

CUPID_inline struct strTycon *tyconRec(Tycon t) {
#if CHECK_TAGS
    if (!isTycon(t))
        tagError("tycon", __func__, t);
#endif
    return tabTycon + (t - TYCMIN);
}

CUPID_inline struct strName *nameRec(Name n) {
#if CHECK_TAGS
    if (!isName(n))
        tagError("name", __func__, n);
#endif
    return tabName + (n - NAMEMIN);
}

CUPID_inline struct strInst *instRec(Inst in) {
#if CHECK_TAGS
    if (!isInst(in))
        tagError("instance", __func__, in);
#endif
    return tabInst + (in - INSTMIN);
}

CUPID_inline struct strClass *cclassRec(Class c) {
#if CHECK_TAGS
    if (!isClass(c))
        tagError("class", __func__, c);
#endif
    return tabClass + (c - CLASSMIN);
}
CUPID_inline Cell mkDigit(int c) {
    return (Cell)(c + INTMIN);
}
CUPID_inline int digitOf(Cell c) {
    return (int)(c - INTMIN);
}

CUPID_inline bool isSynonym(Cell h) {
    return isTycon(h) && tyconRec(h)->what == SYNONYM;
}
CUPID_inline bool isNewtype(Cell h) {
    return isTycon(h) && tyconRec(h)->what == NEWTYPE;
}

CUPID_inline bool isQualType(Cell t) {
    return isPair(t) && fst(t) == QUAL;
}
CUPID_inline bool isPolyType(Cell t) {
    return isPair(t) && fst(t) == POLYTYPE;
}
CUPID_inline bool isPolyOrQualType(Cell t) {
    return isPair(t) && (fst(t) == POLYTYPE || fst(t) == QUAL);
}
CUPID_inline Type mkPolyType(Kinds ks, Type t) {
    return pair(POLYTYPE, pair(ks, t));
}

CUPID_inline bool isSfun(Name n) {
    return nameRec(n)->number == SELNAME;
}
CUPID_inline bool isDfun(Name n) {
    return nameRec(n)->number == DFUNNAME;
}
CUPID_inline bool isCfun(Name n) {
    return nameRec(n)->number >= CFUNNAME;
}
CUPID_inline int cfunOf(Name n) {
    return nameRec(n)->number - CFUNNAME;
}
CUPID_inline int cfunNo(int i) {
    return i + CFUNNAME;
}
CUPID_inline bool isMfun(Name n) {
    return nameRec(n)->number < 0;
}
CUPID_inline int mfunOf(Name n) {
    return (-1) - nameRec(n)->number;
}
CUPID_inline int mfunNo(int i) {
    return (-1) - i;
}
CUPID_inline bool hasCfun(List cs) {
    return nonNull(cs) && isCfun(hd(cs));
}

CUPID_inline Text textOf(Cell c) {
    return (Text)snd(c);
} /* c :: (VAR|CON)(ID|OP) */
CUPID_inline Text qmodOf(Cell c) {
    return textOf(fst(snd(c)));
} /* c :: QUALIDENT */
CUPID_inline Text qtextOf(Cell c) {
    return textOf(snd(snd(c)));
}
CUPID_inline int intValOf(Cell c) {
    return (int)snd(c);
}
CUPID_inline Cell mkVar(Text t) {
    return ap(VARIDCELL, t);
}
CUPID_inline Cell mkVarop(Text t) {
    return ap(VAROPCELL, t);
}
CUPID_inline Cell mkCon(Text t) {
    return ap(CONIDCELL, t);
}
CUPID_inline Cell mkConop(Text t) {
    return ap(CONOPCELL, t);
}
CUPID_inline Cell mkQId(Text m, Cell c) {
    return ap(QUALIDENT, pair(mkCon(m), c));
}
CUPID_inline Cell mkQVar(Text m, Text t) {
    return mkQId(m, mkVar(t));
}
CUPID_inline Cell mkQCon(Text m, Text t) {
    return mkQId(m, mkCon(t));
}
CUPID_inline Cell mkQVarOp(Text m, Text t) {
    return mkQId(m, mkVarop(t));
}
CUPID_inline Cell mkQConOp(Text m, Text t) {
    return mkQId(m, mkConop(t));
}
CUPID_inline Text mkNestedQual(Cell c) {
    return findText(identToStr(c));
} /* fragile... */
CUPID_inline Cell inventVar(void) {
    return mkVar(inventText());
}
CUPID_inline Cell mkDictVar(Text t) {
    return ap(DICTVAR, t);
}
CUPID_inline Cell inventDictVar(void) {
    return mkDictVar(inventDictText());
}
CUPID_inline Cell mkStr(Text t) {
    return ap(STRCELL, t);
}
CUPID_inline bool isFloat(Cell c) {
    return isPair(c) && fst(c) == FLOATCELL;
}
CUPID_inline bool isDouble(Cell c) {
    return isPair(c) && fst(c) == DOUBLECELL;
}
CUPID_inline bool isPtr(Cell c) {
    return isPair(c) && fst(c) == PTRCELL;
}

CUPID_inline Cell triple(Cell x, Cell y, Cell z) {
    return pair(x, pair(y, z));
}
CUPID_inline List cons(Cell x, List xs) {
    return pair(x, xs);
}
CUPID_inline List singleton(Cell x) {
    return cons(x, NIL);
}
CUPID_inline Cell ap2(Cell f, Cell x, Cell y) {
    return ap(ap(f, x), y);
}
CUPID_inline Cell ap3(Cell f, Cell x, Cell y, Cell z) {
    return ap(ap(ap(f, x), y), z);
}

CUPID_inline bool IS_STANDARD_HANDLE(int h) {
    return h <= 2;
}
extern Cell cellIsMember(Cell, List);
extern Cell cellAssoc(Cell, List);
extern Cell cellRevAssoc(Cell, List);
extern List concat(List);
extern List intersect(List, List);
extern Cell varIsMember(Text, List);
extern Name nameIsMember(Text, List);
extern Name nameInIEList(Name, List);
extern Tycon tyconInIEList(Text, List);
extern Class classInIEList(Text, List);
extern Cell intIsMember(int, List);
extern List replicate(int, Cell);
extern List diffList(List, List);   /* destructive         */
extern List deleteCell(List, Cell); /* non-destructive */
extern List take(int, List);        /* destructive         */
extern List splitAt(int, List);     /* non-destructive */
extern Cell nth(int, List);
extern List removeCell(Cell, List); /* destructive         */
extern List nubList(List);          /* non-destructive */

/* The following macros provide `inline expansion' of some common ways of
 * traversing, using and modifying lists:
 *
 * N.B. We use the names _f, _a, _xs, Zs, in an attempt to avoid clashes
 *      with identifiers used elsewhere.
 */

#define mapBasic(_init, _step)                                                 \
    {                                                                          \
        List Zs = (_init);                                                     \
        for (; nonNull(Zs); Zs = tl(Zs))                                       \
            _step;                                                             \
    }
#define mapModify(_init, _step) mapBasic(_init, hd(Zs) = _step)

#define mapProc(_f, _xs) mapBasic(_xs, _f(hd(Zs)))
#define map1Proc(_f, _a, _xs) mapBasic(_xs, _f(_a, hd(Zs)))
#define map2Proc(_f, _a, _b, _xs) mapBasic(_xs, _f(_a, _b, hd(Zs)))
#define map3Proc(_f, _a, _b, _c, _xs) mapBasic(_xs, _f(_a, _b, _c, hd(Zs)))
#define map4Proc(_f, _a, _b, _c, _d, _xs)                                      \
    mapBasic(_xs, _f(_a, _b, _c, _d, hd(Zs)))

#define mapOver(_f, _xs) mapModify(_xs, _f(hd(Zs)))
#define map1Over(_f, _a, _xs) mapModify(_xs, _f(_a, hd(Zs)))
#define map2Over(_f, _a, _b, _xs) mapModify(_xs, _f(_a, _b, hd(Zs)))
#define map3Over(_f, _a, _b, _c, _xs) mapModify(_xs, _f(_a, _b, _c, hd(Zs)))
#define map4Over(_f, _a, _b, _c, _d, _xs)                                      \
    mapModify(_xs, _f(_a, _b, _c, _d, hd(Zs)))

/* This is just what you want for functions with accumulating parameters */
#define mapAccum(_f, _acc, _xs) mapBasic(_xs, _acc = _f(_acc, hd(Zs)))
#define map1Accum(_f, _acc, _a, _xs) mapBasic(_xs, _acc = _f(_acc, _a, hd(Zs)))
#define map2Accum(_f, _acc, _a, _b, _xs)                                       \
    mapBasic(_xs, _acc = _f(_acc, _a, _b, hd(Zs)))
#define map3Accum(_f, _acc, _a, _b, _c, _xs)                                   \
    mapBasic(_xs, _acc = _f(_acc, _a, _b, _c, hd(Zs)))

/* --------------------------------------------------------------------------
 * Implementation of function application nodes:
 * ------------------------------------------------------------------------*/

#define fun(c) fst(c)
#define arg(c) snd(c)

/* A predicate is a class applied to types, or EQUALITY applied to two
 * types, the equality t1 ~ t2 */
CUPID_inline bool isEquality(Cell pi) {
    return isAp(pi) && isAp(fun(pi)) && fun(fun(pi)) == EQUALITY;
}
extern Cell getHead(Cell);
extern List getArgs(Cell);
extern int argCount;
extern Cell nthArg(int, Cell);
extern int numArgs(Cell);
extern Cell applyToArgs(Cell, List);

/* --------------------------------------------------------------------------
 * Stack implementation:
 *
 * NB: Use of macros makes order of evaluation hard to predict.
 *     For example, "push(1+pop());" doesn't increment TOS.
 * ------------------------------------------------------------------------*/

extern Cell DECTABLE(cellStack);
extern StackPtr sp;

#define stack(p) cellStack[p] /* lvalues: assigned through         */
#define top() stack(sp)
#define pushed(n) stack(sp - (n))
extern void cupidStackOverflow(void);

CUPID_inline void clearStack(void) {
    sp = -1;
}
CUPID_inline bool stackEmpty(void) {
    return sp == -1;
}
CUPID_inline void chkStack(int n) {
    if (sp >= NUM_STACK - n)
        cupidStackOverflow();
}
CUPID_inline void onto(Cell c) {
    stack(++sp) = c;
} /* push, unchecked */
CUPID_inline void push(Cell c) {
    chkStack(1);
    onto(c);
}
CUPID_inline Cell pop(void) {
    return stack(sp--);
}
CUPID_inline void drop(void) {
    sp--;
}
CUPID_inline void topfun(Cell f) {
    top() = ap(f, top());
}
CUPID_inline void toparg(Cell x) {
    top() = ap(top(), x);
}

/* STACK_CHECK compares the address of a local variable against a limit
 * derived from CStackBase (machdep.c) and the process's stack rlimit, so
 * that every point in the interpreter that recurses on the C stack --
 * eval, the static checker, the type checker, and the compiler passes
 * that walk the AST -- can detect an approaching C stack overflow and
 * report it through the ordinary error path instead of crashing.  The
 * limit starts out as the highest address, so the first check always
 * takes the slow path, cStackCheck (machine.c), which computes it.
 */
extern char *cStackLimit;
extern void cStackCheck(void);
/* Call at the top of a recursive function; inlined, so the probe is in the
 * caller's frame */
CUPID_inline void stackCheck(void) {
    char cStackProbe;
    if (&cStackProbe <= cStackLimit)
        cStackCheck();
}

/* --------------------------------------------------------------------------
 * Script file control:
 *
 * There is one record per script, numbered from zero, and script 0 is the
 * Prelude implementation.  A record has two halves.  The name half says
 * which file the script was read from; it is filled when the script is
 * queued for loading and script.c owns it.  The marks half holds the state
 * of the tables as it was immediately before the script was read, so that
 * dropScriptsFrom(s) restores the system to that moment; storage.c owns it.
 * Both halves, and every Script value in the system, use this numbering.
 * ------------------------------------------------------------------------*/

#include "machdep.h" /* for Time */

struct primInfoDef; /* a script's primitives; defined in storage.c */

struct strScriptName {
    char *fileName;  /* Script file name                */
    char *realName;  /* Full path to canonical name     */
    char *directory; /* Directory module was found in   */
    Time lastChange; /* Time of last change to script   */
    bool postponed;  /* Indicates postponed load        */
    bool chased;     /* Added by import chasing?        */
};

struct strScriptMarks {
    Text file; /* Script file name, as a text     */
    Text textHw;
    Text nextNewText;
    Text nextNewDText;
    Addr addrHw;
    Module moduleHw;
    Tycon tyconHw;
    Name nameHw;
    Class classHw;
    Inst instHw;
    /*
     * handle to the script's primInfoDef containing primops associated
     * with this script (if any.)
     */
    struct primInfoDef *prims;
};

struct strScript {
    struct strScriptName name;
    struct strScriptMarks marks;
};

extern struct strScript DECTABLE(tabScript);

CUPID_inline struct strScript *scriptRec(Script s) {
#if CHECK_TAGS
    if (s < 0 || s >= NUM_SCRIPTS)
        tagError("script", __func__, s);
#endif
    return tabScript + s;
}

extern Script startNewScript(char *);
extern bool moduleThisScript(Module);
extern Module moduleOfScript(Script);
extern bool isPreludeScript(void);
extern Module lastModule(void);
extern Script scriptThisFile(Text);
extern Script scriptThisName(Name);
extern Script scriptThisTycon(Tycon);
extern Script scriptThisInst(Inst);
extern Script scriptThisClass(Class);
extern char *fileOfModule(Module);
extern void dropAScript(Script);
extern void dropScriptsFrom(Script);
extern void setScriptPrims(void *);

/* --------------------------------------------------------------------------
 * I/O Handles:
 * ------------------------------------------------------------------------*/

#define HSTDIN 0 /* Numbers for standard handles                   */
#define HSTDOUT 1
#define HSTDERR 2

struct strHandle { /* Handle description and status flags          */
    Cell hcell;    /* Heap representation of handle (or NIL)  */
    FILE *hfp;     /* Corresponding file pointer                   */
    int hmode;     /* Current mode: see below              */
    int hbufMode;  /* Buffering mode.                         */
    int hbufSize;  /* < 0 => not yet known.                   */
    int hRWState;  /* State of a READWRITE handle (see below) */
#if CHAR_ENCODING
    bool hBinaryMode; /* true => Handle opened in binary mode    */
    Char hLookAhead;  /* Char read by hLookAhead (or <0 if none) */
                      /* This is only used in text mode.         */
#endif
};

#define HCLOSED 0000     /* no I/O permitted                       */
#define HSEMICLOSED 0001 /* semiclosed reads only                  */
#define HREAD 0002       /* set to enable reads from handle        */
#define HWRITE 0004      /* set to enable writes to handle         */
#define HAPPEND 0010     /* opened in append mode                  */
#define HREADWRITE 0020  /* set to enable both reading and writing  */

#define HUNKNOWN_BUFFERING                                                     \
    (-1) /* the buffering mode of a handle is lazily
                                   determined. */

#define HANDLE_NOTBUFFERED 1
#define HANDLE_LINEBUFFERED 2
#define HANDLE_BLOCKBUFFERED 3

#define RW_NEUTRAL 0 /* r/w stream was just opened/at EOF/seeked */
#define RW_READING 1 /* last operation on r/w stream was reading */
#define RW_WRITING 2 /* last operation on r/w stream was writing */

#define NO_HANDLE (-1)

extern unsigned long num_handles;
extern DynTable *dynTabHandles;
extern struct strHandle *handles;

/* --------------------------------------------------------------------------
 * Malloc Pointers
 * ------------------------------------------------------------------------*/

struct strMallocPtr { /* Malloc Ptr description                  */
    Cell mpcell;      /* Back pointer to MPCELL                  */
    void *ptr;        /* Pointer into C world                    */
    int refCount;     /* Reference count                         */
    Cell finalizers;  /* List of finalizer functions             */
                      /* or (finalizer, envptr) pairs              */
};

typedef void (*CFinalizer)(void *);
typedef void (*CFinalizerEnv)(void *, void *);

extern struct strMallocPtr mallocPtrs[];
extern Cell newMallocPtr(void *);
extern Cell mkMallocPtr(void *, CFinalizer);
extern void freeMallocPtr(Cell);
extern void incMallocPtrRefCnt(int, int);

extern int mallocPtr_hw;

#define mpOf(c) snd(c)
#define derefMP(c) (mallocPtrs[(int)mpOf(c)].ptr)

/* --------------------------------------------------------------------------
 * Weak Pointers
 * ------------------------------------------------------------------------*/

#define derefWeakPtr(c) fst(snd(c))
#define nextWeakPtr(c) snd(snd(c))

extern List finalizers;
extern List liveWeakPtrs;

/* --------------------------------------------------------------------------
 * Foreign Function Interface
 * ------------------------------------------------------------------------*/

#include "HsFFI.h"

extern int part1Int64(HsInt64);
extern int part2Int64(HsInt64);
extern HsInt64 int64FromParts(int, int);

/* --------------------------------------------------------------------------
 * Stable pointers
 * ------------------------------------------------------------------------*/

extern int mkStablePtr(Cell);
extern Cell derefStablePtr(int);
extern void freeStablePtr(int);

/* --------------------------------------------------------------------------
 * Misc:
 * ------------------------------------------------------------------------*/

extern void setLastExpr(Cell);
extern Cell getLastExpr(void);
extern List addTyconsMatching(char *, List);
extern List addNamesMatching(char *, List);

/*-------------------------------------------------------------------------*/

/* whatIs1 is faster than whatIs, but it will return NIL if fst(c) is NIL
 * Used with care in the right places it gains us speed.
 */

/* --------------------------------------------------------------------------
 * What the partition above depends on.
 *
 * Every range from TUPMIN up is computed from a table size in sizes.h, so
 * raising one silently shifts every range above it.  These say what may
 * not move.  They are compile-time checks: if one fires, the number that
 * was changed cannot take the value it was given.
 * ------------------------------------------------------------------------*/

/* Tags, then the special values, then the tables: three groups that may
 * not overlap.  TAG_MASK masks everything below SPECMIN. */
static_assert(TAGMIN < BCSTAG, "box tags come before constructor tags");
static_assert(BCSTAG <= SPECMIN, "constructor tags come before the specials");
static_assert(SPECMIN > ~TAG_MASK, "SPECMIN must be out of TAG_MASK range");
static_assert(SPECMIN < TUPMIN, "the specials come before the tables");

/* The tables tile the positive Cell space in this order, and nothing may
 * be given a size of zero, which would make a range empty and let the one
 * below it collide with the one above. */
static_assert(TUPMIN < OFFMIN, "the tuple range is empty");
static_assert(OFFMIN < MODMIN, "the offset range is empty");
static_assert(MODMIN < TYCMIN, "the module range is empty");
static_assert(TYCMIN < NAMEMIN, "the tycon range is empty");
static_assert(NAMEMIN < INSTMIN, "the name range is empty");
static_assert(INSTMIN < CLASSMIN, "the instance range is empty");
static_assert(CLASSMIN < CHARMIN, "the class range is empty");
static_assert(CHARMIN < INTMIN, "the short character range is empty");
static_assert(INTMIN < INTMAX, "the small integer range is empty");

/* whatIs() indexes whatCode[] with any cell that is not a pair, so the
 * partition has to end exactly where that array does, and stay inside a
 * positive Int: a negative cell is a heap pair. */
static_assert(N_WHATCODE == INTMAX + 1, "whatCode must cover the partition");
static_assert(INTMAX < MAXPOSINT, "the partition must fit in a positive Int");

/* whatCode is an unsigned char array that initFastWhat (storage.c) fills
 * with whatCode[i] = i below TUPMIN and with tag codes above it, so both
 * the cell values below TUPMIN and every tag stored have to fit in one. */
static_assert(TUPMIN - 1 <= UCHAR_MAX, "whatCode[i] = i would truncate");
static_assert(OFFSET <= UCHAR_MAX, "the tag codes must fit in whatCode");

/* The module table is indexed off the script numbering. */
static_assert(NUM_MODULE == NUM_SCRIPTS, "one module record per script");

/* Small integers straddle zero. */
static_assert(MINSMALLINT < 0, "the small integers must reach below zero");
static_assert(MAXSMALLINT > 0, "the small integers must reach above zero");

#endif /* __STORAGE_H__ */
