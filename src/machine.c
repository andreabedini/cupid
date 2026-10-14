/* --------------------------------------------------------------------------
 * Graph reduction engine, code generation and execution
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
#include "opts.h" /* IWYU pragma: keep -- declares debugCode, defined here under DEBUG_CODE */
#if DEBUG_CODE
# include "output.h" /* printExp, for the disassembler's headings */
#endif
#include <setjmp.h>

#if DEBUG_CODE
bool debugCode = false; /* true => print G-code to screen  */
#endif

int evalDepth; /* depth of nested eval()'s        */

/* --------------------------------------------------------------------------
 * C stack overflow detection:
 *
 * STACK_CHECK (storage.h) is used at every point in the interpreter that
 * recurses on the C stack with no other bound (eval, the static checker,
 * the type checker, the compiler passes that walk the AST): it compares
 * the address of a local against cStackLimit and, when that fails, calls
 * cStackCheck() below, which reports "Control stack overflow" through
 * the usual error path (cupidStackOverflow(), storage.c) instead of
 * letting the recursion run off the end of the stack and segfault.
 *
 * CStackBase (machdep.c) is recorded once, very early in main(); it is
 * also used by the conservative collector.  The usable size of the
 * stack comes from getrlimit(RLIMIT_STACK): read lazily, on the first
 * check, so that a test harness which lowers the limit with setrlimit()
 * before exec (as tests/run-tests does for NAME.stack) is honoured.
 * When the limit is unavailable or unlimited we fall back to a
 * conservative 8 MB, the default stack size on every supported target.
 * All supported targets grow the C stack downward (see STACK_DIRECTION
 * in sysdeps.h), so the limit is the top of the stack minus the usable
 * size.  The top is not CStackBase: the rlimit counts the whole stack,
 * and exec puts the argument and environment strings above main()'s
 * frame (cStackTop below).
 *
 * STACK_MARGIN is headroom kept below the computed limit: room for the
 * deepest primitive that still runs after STACK_CHECK trips, plus the
 * error-reporting path itself (formatting and printing the message,
 * which recurses a little to show the expression being evaluated).
 * 64 KB was chosen empirically: measurements on this build showed under
 * 8 KB of further C stack use between a STACK_CHECK tripping in eval()
 * and the error message reaching the user, so 64 KB leaves a wide
 * safety factor for the deeper recursions in the type checker and
 * compiler passes.
 * ------------------------------------------------------------------------*/
#if HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
extern char **environ;
#endif

#define STACK_MARGIN (64 * 1024)           /* headroom kept below the limit   */
#define FALLBACK_C_STACK (8 * 1024 * 1024) /* used when the rlimit is unknown */

char *cStackLimit = (char *)(-1); /* lowest address considered safe; */
                                  /* see STACK_CHECK in storage.h    */

/* The top of the stack, as far as it can be found: the end of the
 * highest environment string, which on Linux, macOS and the BSDs lies
 * above the argument strings and below only the program's path.  The
 * environment can be tens of kilobytes; left out, it is taken from the
 * margin, and a deep recursion then runs off the stack before the check
 * trips.  A string further away than the usable size was moved off the
 * stack by setenv() and does not count. */
static char *cStackTop(unsigned usable) {
    char *base = (char *)CStackBase;
    char *top = base;
#if HAVE_SYS_RESOURCE_H
    for (char **e = environ; e && *e; e++) {
        char *end = *e + strlen(*e) + 1;
        if (end > top && (size_t)(end - base) < usable) {
            top = end;
        }
    }
#endif
    return top;
}

static void initCStackLimit(void) {
    unsigned usable = FALLBACK_C_STACK;
#if HAVE_SYS_RESOURCE_H
    struct rlimit rl;
    if (getrlimit(RLIMIT_STACK, &rl) == 0 && rl.rlim_cur != RLIM_INFINITY) {
        usable = (unsigned)rl.rlim_cur;
    }
#endif
    if (usable <= STACK_MARGIN) {     /* absurdly small rlimit; keep */
        usable = STACK_MARGIN + 4096; /* a sliver of headroom anyway */
    }
    cStackLimit = cStackTop(usable) - (usable - STACK_MARGIN);
}

/* Slow path of STACK_CHECK */
void cStackCheck(void) {
    char here;
    if (cStackLimit == (char *)(-1)) {
        initCStackLimit();
    }
    if (&here <= cStackLimit) {
        cupidStackOverflow();
    }
}

/* --------------------------------------------------------------------------
 * mkTuple(n) and mkOffset(o) (storage.h) used to build a tagged cell
 * without checking n against NUM_TUPLES or o against NUM_OFFSETS.  Once a
 * count or offset ran past its limit the resulting cell aliased the next
 * tag range up (OFFSET for an over-large tuple, MODULE for an over-large
 * offset), so something downstream that inspected the cell's tag -- often
 * far from where the excess count or offset first arose, e.g. a 200-tuple
 * or a function with over 1000 let-bindings -- hit an "INTERNAL ERROR"
 * sanity check instead of a clean message.  Checking here, at the single
 * place both kinds of cell are built, catches every over-large tuple or
 * offset regardless of which caller produced it.
 * ------------------------------------------------------------------------*/

/* MkTuple(n) with n>=NUM_TUPLES */
void tooManyTupleComponents(void) {
    errHead(0);
    errPrintf("Tuples with more than %d components are not supported",
              NUM_TUPLES);
    errFail();
}

/* MkOffset(o) with o>=NUM_OFFSETS */
void tooManyOffsets(void) {
    errHead(0);
    errPrintf("Compiled code too complex");
    errFail();
}

/* --------------------------------------------------------------------------
 * Data structures for machine memory (program storage):
 * ------------------------------------------------------------------------*/

/* This list defines the sequence of all instructions that can be used in
 * the abstract machine code for Cupid.  The Ins() macro is used to
 * ensure that the correct mapping of instructions to labels is used when
 * compiling the GCC_THREADED version.
 */

#define INSTRLIST                                                              \
    Ins(iLOAD), Ins(iCELL), Ins(iCHAR), Ins(iINT), Ins(iDOUBLE), Ins(iSTRING), \
        Ins(iMKAP), Ins(iUPDATE), Ins(iUPDAP), Ins(iEVAL), Ins(iRETURN),       \
        Ins(iTEST), Ins(iGOTO), Ins(iSETSTK), Ins(iROOT), Ins(iSLIDE),         \
        Ins(iSTAP), Ins(iTABLE), Ins(iLEVAL), Ins(iRUPDAP), Ins(iRUPDATE),     \
        Ins(iFAIL), Ins(iALLOC)

#define Ins(x) x
typedef enum { INSTRLIST } Instr;
#undef Ins

typedef int Label;

typedef union {
    int mint;
    Float mfloat;
    Cell cell;
    Text text;
    Addr addr;
    Instr instr;
    Label lab;
} MemCell;

typedef MemCell *Memory;
static Memory memory;
/* The code table grows by realloc, so `memory' moves.  Everything that reads
 * code goes through the accessors below, which re-index `memory' on each
 * access; the one exception is run(), which keeps a MemCell * in a register
 * for the whole of a supercombinator.  That is sound only because code is
 * generated exclusively by the load and prompt pipelines -- codeGen and
 * implementCfun, reached from compileDefns, compileExp and checkDefns --
 * and neither can run while the evaluator is on the C stack: run() itself
 * calls only eval(), the heap constructors and evalFails(), and every route
 * from eval() to the compiler would have to pass through a primitive.
 * growMemory() checks that invariant rather than leaving it to be
 * rediscovered. */
DynTable *dynMemory = NULL;

void growMemory(void) {
    /* evalDepth is 0 outside evaluation and >= 1 inside eval().  A nonzero
     * depth here means code is being generated underneath a live run(),
     * whose pc the realloc below would leave dangling. */
    if (evalDepth != 0)
        internal("code generated during evaluation");
    growDynTable(dynMemory);
    memory = (Memory)(dynMemory->data);
    if (memory == 0)
        fatal("Cannot allocate program memory");
}

#define intAt(m) memory[m].mint
#define floatAt(m) memory[m].mfloat
#define cellAt(m) memory[m].cell
#define textAt(m) memory[m].text
#define setAddrAt(m, a) memory[m].addr = ((a) - (m))
#define addrAt(m) (memory[m].addr + (m))
#define instrAt(m) memory[m].instr
#define labAt(m) memory[m].lab

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

typedef struct CodeGen CodeGen;

static void instrNone(CodeGen *, Instr);
static void instrInt(CodeGen *, Instr, int);
static void instrDouble(CodeGen *, Instr, DoublePro);
static void instrCell(CodeGen *, Instr, Cell);
static void instrText(CodeGen *, Instr, Text);
static void instrLab(CodeGen *, Instr, Label);
static void instrCellLab(CodeGen *, Instr, Cell, Label);

static void atLabel(CodeGen *, Label);
static void endLabel(CodeGen *, Label, Label);
static void fixAddr(CodeGen *, Addr);
static void asSTART(CodeGen *);
static Label newLabel(CodeGen *, Label);
static void asEND(CodeGen *);
static void asINTEGER(CodeGen *, int);
static void asDOUBLE(CodeGen *, DoublePro);
static void asSTRING(CodeGen *, Text);
static void asCHAR(CodeGen *, int);
static void asLOAD(CodeGen *, int);
static void asALLOC(CodeGen *, int);
static void asROOT(CodeGen *, int);
static void asSETSTK(CodeGen *, int);
static void asSTAP(CodeGen *);
static void asRETURN(CodeGen *);
static void asCELL(CodeGen *, Cell);
static void asEVAL(CodeGen *);
static void asTEST(CodeGen *, Cell, Label);
static void asSLIDE(CodeGen *, int);
static void asMKAP(CodeGen *, int);
static void asUPDATE(CodeGen *, int);
static void asRUPDATE(CodeGen *);
static void asGOTO(CodeGen *, Label);

#if DEBUG_CODE
static void dissassemble(Addr, Addr);
static Addr dissInstr(Addr);
static void printCell(Cell);
static Addr dissNone(Addr, char *);
static Addr dissInt(Addr, char *);
static Addr dissDouble(Addr, char *);
static Addr dissCell(Addr, char *);
static Addr dissText(Addr, char *);
static Addr dissAddr(Addr, char *);
static Addr dissCellAddr(Addr, char *);
#endif

static void build(CodeGen *, Cell, int);
static void buildGuards(CodeGen *, List, int);
static int buildLoc(CodeGen *, List, int);
static void buildBignum(CodeGen *, Bignum);

static void make(CodeGen *, Cell, int, Label, Label);
static void makeCond(CodeGen *, Cell, Cell, Cell, int, Label, Label);
static void makeNumcase(CodeGen *, Triple, int, Label, Label);
static void testGuard(CodeGen *, Pair, int, Label, Label, Label);
static void testCase(CodeGen *, Pair, int, Label, Label, Label);
static void makeTests(CodeGen *,
                      void (*)(CodeGen *, Cell, int, Label, Label, Label), List,
                      int, Label, Label);

static void analyseAp(CodeGen *, Cell);
static void buildAp(CodeGen *, Cell, int, Label, bool);

static void evalString(Cell);

/* --------------------------------------------------------------------------
 * Assembler: (Low level, instruction code storage)
 *
 * Generating code for one supercombinator runs on the state below.  It was
 * a dozen file-scope variables; it is one record now, which the two entry
 * points make and hand to everything they call, so that the state a
 * function reads and writes is in its signature.
 * ------------------------------------------------------------------------*/

struct CodeGen {
    Addr startInstr;           /* first instruction after START   */
    Addr lastInstr;            /* last instr written (for peephole*/
                               /* optimisations etc.)             */
    bool newBasicBlock;        /* lastInstr ends a basic block    */
                               /* (so no peeping at lastInstr)    */
    int srsp;                  /* simulated runtime stack pointer */
    int offsPosn[NUM_OFFSETS]; /* mapping from logical to physical*/
                               /* offset positions                */
    Label nextLab;             /* next label number to allocate   */
    Addr fixups[NUM_FIXUPS];   /* fixup table maps Label -> Addr  */
    int nonRoots;              /* #args which can't get from root */
    int rootPortion;           /* portion of root used ...        */
    Name definingName;         /* name of func being defined,if any*/
    int definingArity;         /* arity of definingName           */
};

static Addr noMatch; /* address of a single FAIL instr  */

/* Opcode with no operands */
static void instrNone(CodeGen *cg, Instr opc) {
    cg->lastInstr = getMem(1);
    instrAt(cg->lastInstr) = opc;
    cg->newBasicBlock = false;
}

/* Opcode with integer operand */
static void instrInt(CodeGen *cg, Instr opc, int n) {
    cg->lastInstr = getMem(2);
    instrAt(cg->lastInstr) = opc;
    intAt(cg->lastInstr + 1) = n;
    cg->newBasicBlock = false;
}

/* Opcode with Double operand */
static void instrDouble(CodeGen *cg, Instr opc, DoublePro fl) {
    cg->lastInstr = getMem(3);
    instrAt(cg->lastInstr) = opc;
    cellAt(cg->lastInstr + 1) = part1Double(fl);
    cellAt(cg->lastInstr + 2) = part2Double(fl);
    cg->newBasicBlock = false;
}

/* Opcode with Cell operand */
static void instrCell(CodeGen *cg, Instr opc, Cell c) {
    cg->lastInstr = getMem(2);
    instrAt(cg->lastInstr) = opc;
    cellAt(cg->lastInstr + 1) = c;
    cg->newBasicBlock = false;
}

/* Opcode with Text operand */
static void instrText(CodeGen *cg, Instr opc, Text t) {
    cg->lastInstr = getMem(2);
    instrAt(cg->lastInstr) = opc;
    textAt(cg->lastInstr + 1) = t;
    cg->newBasicBlock = false;
}

/* Opcode with label operand */
static void instrLab(CodeGen *cg, Instr opc, Label l) {
    cg->lastInstr = getMem(2);
    instrAt(cg->lastInstr) = opc;
    labAt(cg->lastInstr + 1) = l;
    if (l < 0)
        internal("bad Label");
    cg->newBasicBlock = false;
}

/* Opcode with cell, label operands */
static void instrCellLab(CodeGen *cg, Instr opc, Cell c, Label l) {
    cg->lastInstr = getMem(3);
    instrAt(cg->lastInstr) = opc;
    cellAt(cg->lastInstr + 1) = c;
    labAt(cg->lastInstr + 2) = l;
    if (l < 0)
        internal("bad Label");
    cg->newBasicBlock = false;
}

/* --------------------------------------------------------------------------
 * Main low level assembler control: (includes label assignment and fixup)
 *
 * Labels are used as a simple form of continuation during the code gen:
 *  RUNON    => produce code which does not make jump at end of construction
 *  UPDRET   => produce code which performs RUPDATE at end
 *  VALRET   => produce code which performs RETURN at end
 *  other(d) => produce code which branches to label d at end
 * ------------------------------------------------------------------------*/

#define SHOULDNTFAIL (-1)
#define RUNON (-2)
#define UPDRET (-3)
#define VALRET (-4)

/* Label n is here */
static void atLabel(CodeGen *cg, Label n) {
    cg->newBasicBlock = true;
    cg->fixups[n] = getMem(0);
}

/* L is where code falling out the bottom of a construction lands */
static void endLabel(CodeGen *cg, Label d, Label l) {
    if (d == RUNON)
        atLabel(cg, l);
}

/* Resolve the label at a */
static void fixAddr(CodeGen *cg, Addr a) {
    setAddrAt(a, cg->fixups[labAt(a)]);
}

/* Initialise assembler */
static void asSTART(CodeGen *cg) {
    cg->fixups[0] = noMatch;
    cg->nextLab = 1;
    cg->startInstr = getMem(0);
    cg->lastInstr = cg->startInstr - 1;
    cg->newBasicBlock = true;
    cg->srsp = 0;
    cg->offsPosn[0] = 0;
    cg->nonRoots = 0;
    cg->rootPortion = 0;
    cg->definingName = NIL;
    cg->definingArity = 0;
}

/* Allocate new label */
static Label newLabel(CodeGen *cg, Label d) {
    if (d == RUNON) {
        if (cg->nextLab >= NUM_FIXUPS) {
            errHead(0);
            errPrintf("Compiled code too complex");
            errFail();
        }
        return cg->nextLab++;
    }
    return d;
}

/* Fix addresses in assembled code */
static void asEND(CodeGen *cg) {
    Addr pc = cg->startInstr;

    while (pc <= cg->lastInstr)
        switch (instrAt(pc)) {
        case iEVAL: /* opcodes taking no arguments      */
        case iFAIL:
        case iSTAP:
        case iRUPDATE:
        case iRUPDAP:
        case iRETURN:
            pc++;
            break;

        case iGOTO:
            fixAddr(cg, pc + 1); /* opcodes taking one argument      */
        case iSETSTK:
        case iALLOC:
        case iSLIDE:
        case iROOT:
        case iLOAD:
        case iLEVAL:
        case iCELL:
        case iCHAR:
        case iINT:
        case iSTRING:
        case iMKAP:
        case iUPDATE:
        case iUPDAP:
            pc += 2;
            break;

        case iDOUBLE:
            pc += 3;
            break;

        case iTEST:
            fixAddr(cg, pc + 2);
            pc += 3;
            break;

        default:
            internal("fixAddrs");
        }
}

/* --------------------------------------------------------------------------
 * Assembler Opcodes: (includes simple peephole optimisations)
 *
 * Each of these emits one instruction and says what it does to the
 * simulated runtime stack pointer.
 * ------------------------------------------------------------------------*/

static void asINTEGER(CodeGen *cg, int n) {
    instrInt(cg, iINT, n);
    cg->srsp++;
}

static void asDOUBLE(CodeGen *cg, DoublePro fl) {
    instrDouble(cg, iDOUBLE, fl);
    cg->srsp++;
}

static void asSTRING(CodeGen *cg, Text t) {
    instrText(cg, iSTRING, t);
    cg->srsp++;
}

static void asCHAR(CodeGen *cg, int n) {
    instrInt(cg, iCHAR, n);
    cg->srsp++;
}

static void asLOAD(CodeGen *cg, int n) {
    instrInt(cg, iLOAD, n);
    cg->srsp++;
}

static void asALLOC(CodeGen *cg, int n) {
    instrInt(cg, iALLOC, n);
    cg->srsp += n;
}

static void asROOT(CodeGen *cg, int n) {
    instrInt(cg, iROOT, n);
    cg->srsp++;
}

static void asSETSTK(CodeGen *cg, int n) {
    instrInt(cg, iSETSTK, n);
    cg->srsp = n;
}

static void asSTAP(CodeGen *cg) {
    instrNone(cg, iSTAP);
    cg->srsp--;
}

static void asRETURN(CodeGen *cg) {
    instrNone(cg, iRETURN);
}

static void asCELL(CodeGen *cg, Cell c) {
    instrCell(cg, iCELL, c);
    cg->srsp++;
}

/* Peephole optimisations are unsafe if we're at the start of a new
   'basic block' (newBasicBlock is true) -- e.g.,

        f a b = if (if a then True else b) then foo else bar

   After having emitted the code for the conditional, we want to evaluate
   it, BUT we better _not_ look at the last instruction of the code
   comprising the conditional expression and decide whether or not to
   peephole the EVAL into an LEVAL.
*/

/* Load and eval stack element */
static void asEVAL(CodeGen *cg) {
    if (!cg->newBasicBlock && instrAt(cg->lastInstr) == iLOAD)
        /* Peephole optimisation:          */
        instrAt(cg->lastInstr) = iLEVAL; /* LOAD n; EVAL ===> LEVAL n       */
    else
        instrNone(cg, iEVAL);
    cg->srsp--;
}

/* Test whnf and branch on mismatch */
static void asTEST(CodeGen *cg, Cell c, Label l) {
    switch (whatIs(c)) {
    case TUPLE:
        return; /* typing guarantees that tags will*/
        /* match without further tests     */
    case NAME:
        if (isCfun(c) && cfunOf(c) == 0)
            return;
    default:
        break;
    }
    instrCellLab(cg, iTEST, c, l);
}

/* Slide results down stack */
static void asSLIDE(CodeGen *cg, int n) {
    if (!cg->newBasicBlock && instrAt(cg->lastInstr) == iSLIDE)
        /* Peephole optimisation:          */
        intAt(cg->lastInstr + 1) += n; /* SLIDE n;SLIDE m ===> SLIDE (n+m)*/
    else
        instrInt(cg, iSLIDE, n);
    cg->srsp -= n;
}

/* Make application nodes ... */
static void asMKAP(CodeGen *cg, int n) {
    if (!cg->newBasicBlock && instrAt(cg->lastInstr) == iMKAP)
        /* Peephole optimisation:          */
        intAt(cg->lastInstr + 1) += n; /* MKAP n; MKAP m  ===> MKAP (n+m) */
    else
        instrInt(cg, iMKAP, n);
    cg->srsp -= n;
}

/* Update node ... */
static void asUPDATE(CodeGen *cg, int n) {
    if (!cg->newBasicBlock && instrAt(cg->lastInstr) == iMKAP) {
        int m = intAt(cg->lastInstr + 1); /* Peephole optimisations:     */
        nextInstr(cg->lastInstr);
        if (m == 1) /* MKAP 1; UPDATE p ===> UPDAP p   */
            instrInt(cg, iUPDAP, n);
        else {                          /* MKAP m; UPDATE p            */
            instrInt(cg, iMKAP, m - 1); /*   ===> MKAP (m-1); UPDAP p  */
            instrInt(cg, iUPDAP, n);
        }
    } else
        instrInt(cg, iUPDATE, n);
    cg->srsp--;
}

/* Update node and return ... */
static void asRUPDATE(CodeGen *cg) {
    if (!cg->newBasicBlock && instrAt(cg->lastInstr) == iMKAP) {
        int m = intAt(cg->lastInstr + 1); /* Peephole optimisations:     */
        nextInstr(cg->lastInstr);
        if (m == 1) /* MKAP 1; RUPDATE ===> RUPDAP     */
            instrNone(cg, iRUPDAP);
        else {                          /* MKAP m; RUPDATE                 */
            instrInt(cg, iMKAP, m - 1); /*       ===> MKAP (m-1); RUPDAP   */
            instrNone(cg, iRUPDAP);
        }
    } else
        instrNone(cg, iRUPDATE);
}

/* End evaluation of expr in manner indicated by l */
static void asGOTO(CodeGen *cg, Label l) {
    switch (l) { /* inaccurate srsp */
    case UPDRET:
        asRUPDATE(cg);
        break;
    case VALRET:
        asRETURN(cg);
    case RUNON:
        break;
    default:
        instrLab(cg, iGOTO, l);
        break;
    }
}

/* --------------------------------------------------------------------------
 * Constructor function tables:
 *
 * Tables of constructor functions for enumerated types are needed to
 * produce derived instances.
 * ------------------------------------------------------------------------*/

/* Add a constructor fun table to constructors for tycon tc */
void addCfunTable(Tycon tc) {
    if (isTycon(tc) && tyconRec(tc)->what == DATATYPE) {
        List cs = tyconRec(tc)->defn;
        if (hasCfun(cs) && hasCfun(tl(cs)) && nameRec(hd(cs))->code <= 0) {
            int l = length(cs);
            Addr a = getMem(2 + l);
            instrAt(a) = iTABLE;
            intAt(a + 1) = l;
            for (l = 0; nonNull(cs); l++, cs = tl(cs)) {
                cellAt(a + l + 2) = hd(cs);
                nameRec(hd(cs))->code = a;
            }
        }
    }
}

/* Get next constructor in sequence or NIL, if none */
Name succCfun(Name n) {
    if (cfunOf(n) == 0)
        return NIL;
    else {
        int d = cfunOf(n) + 1;
        Addr a = nameRec(n)->code;
        return (d > intAt(a + 1)) ? NIL : cellAt(a + d + 1);
    }
}

/* Get next constructor in series or NIL, if none */
Name nextCfun(Name n1, Name n2) {
    if (cfunOf(n1) == 0) /* For product constructors, the   */
        return n1;       /* only possibility is n1 == n2    */
    else {
        int d = 2 * cfunOf(n2) - cfunOf(n1);
        Addr a = nameRec(n1)->code;
        return (d <= 0 || d > intAt(a + 1)) ? NIL : cellAt(a + d + 1);
    }
}

/* Get ith constructor (0<=i<m) for enumerated datatype with a representative
 * cfun n */
Name cfunByNum(Name n, int i) {
    if (cfunOf(n) == 0)
        return i == 0 ? n : NIL;
    else {
        Addr a = nameRec(n)->code;
        return (i >= 0 && i < intAt(a + 1)) ? cellAt(a + i + 2) : NIL;
    }
}

/* --------------------------------------------------------------------------
 * Dissassembler:
 * ------------------------------------------------------------------------*/

#if DEBUG_CODE
# define printAddr(a) Printf("0x%04X", a) /* printable representation of Addr */

/* Print dissassembly of code */
static void dissassemble(Addr pc, Addr end) {
    while (pc <= end) {
        printAddr(pc);
        Printf("\t");
        pc = dissInstr(pc);
    }
}

/* Print dissassembly of instruction */
static Addr dissInstr(Addr pc) {
    switch (instrAt(pc)) {
    case iLOAD:
        pc = dissInt(pc, "LOAD");
        break;
    case iLEVAL:
        pc = dissInt(pc, "LEVAL");
        break;
    case iCELL:
        pc = dissCell(pc, "CELL");
        break;
    case iCHAR:
        pc = dissInt(pc, "CHAR");
        break;
    case iINT:
        pc = dissInt(pc, "INT");
        break;
    case iDOUBLE:
        pc = dissDouble(pc, "DOUBLE");
        break;
    case iSTRING:
        pc = dissText(pc, "STRING");
        break;
    case iMKAP:
        pc = dissInt(pc, "MKAP");
        break;
    case iUPDATE:
        pc = dissInt(pc, "UPDATE");
        break;
    case iRUPDATE:
        pc = dissNone(pc, "RUPDATE");
        break;
    case iUPDAP:
        pc = dissInt(pc, "UPDAP");
        break;
    case iRUPDAP:
        pc = dissNone(pc, "RUPDAP");
        break;
    case iEVAL:
        pc = dissNone(pc, "EVAL");
        break;
    case iSTAP:
        pc = dissNone(pc, "STAP");
        break;
    case iRETURN:
        pc = dissNone(pc, "RETURN");
        break;
    case iTEST:
        pc = dissCellAddr(pc, "TEST");
        break;
    case iGOTO:
        pc = dissAddr(pc, "GOTO");
        break;
    case iSETSTK:
        pc = dissInt(pc, "SETSTK");
        break;
    case iALLOC:
        pc = dissInt(pc, "ALLOC");
        break;
    case iSLIDE:
        pc = dissInt(pc, "SLIDE");
        break;
    case iROOT:
        pc = dissInt(pc, "ROOT");
        break;
    case iFAIL:
        pc = dissNone(pc, "FAIL");
        break;
    case iTABLE:
        pc = dissNone(pc, "TABLE");
        pc += intAt(pc) + 1;
        break;
    default:
        internal("unknown instruction");
    }
    return pc;
}

/* Printable representation of Cell */
static void printCell(Cell c) {
    if (isName(c))
        Printf("%s", textToStr(nameRec(c)->text));
    else
        Printf("$%d", c);
}

/* Dissassemble instr no args */
static Addr dissNone(Addr pc, char *s) {
    Printf("%s\n", s);
    return pc + 1;
}

/* Dissassemble instr with Int arg */
static Addr dissInt(Addr pc, char *s) {
    Printf("%s\t%d\n", s, intAt(pc + 1));
    return pc + 2;
}

/* Dissassemble instr with Double arg */
static Addr dissDouble(Addr pc, char *s) {
    Printf("%s\t%s\n", s,
           doubleToString(doubleFromParts(cellAt(pc + 1), cellAt(pc + 2))));
    return pc + 3;
}

/* Dissassemble instr with Cell arg */
static Addr dissCell(Addr pc, char *s) {
    Printf("%s\t", s);
    printCell(cellAt(pc + 1));
    Printf("\n");
    return pc + 2;
}

/* Dissassemble instr with Text arg */
static Addr dissText(Addr pc, char *s) {
    Printf("%s\t%s\n", s, textToStr(textAt(pc + 1)));
    return pc + 2;
}

/* Dissassemble instr with Addr arg */
static Addr dissAddr(Addr pc, char *s) {
    Printf("%s\t", s);
    printAddr(addrAt(pc + 1));
    Printf("\n");
    return pc + 2;
}

/* Dissassemble instr with Cell/Addr */
static Addr dissCellAddr(Addr pc, char *s) {
    Printf("%s\t", s);
    printCell(cellAt(pc + 1));
    Printf("\t");
    printAddr(addrAt(pc + 2));
    Printf("\n");
    return pc + 3;
}
#endif

/* --------------------------------------------------------------------------
 * Compile expression to code which will build expression without any
 * evaluation.
 * ------------------------------------------------------------------------*/

/* Generate code which will build instance of given expression but perform no
 * evaluation */
static void build(CodeGen *cg, Cell e, int co) {
    int n;

    stackCheck();
    switch (whatIs(e)) {

    case LETREC:
        n = buildLoc(cg, fst(snd(e)), co);
        build(cg, snd(snd(e)), co + n);
        asSLIDE(cg, n);
        break;

    case FATBAR:
        build(cg, snd(snd(e)), co);
        build(cg, fst(snd(e)), co);
        asCELL(cg, nameFatbar);
        asMKAP(cg, 2);
        break;

    case COND:
        build(cg, thd3(snd(e)), co);
        build(cg, snd3(snd(e)), co);
        build(cg, fst3(snd(e)), co);
        asCELL(cg, nameIf);
        asMKAP(cg, 3);
        break;

    case GUARDED:
        buildGuards(cg, snd(e), co);
        break;

    case AP:
        buildAp(cg, e, co, SHOULDNTFAIL, false);
        break;

    case TUPLE:
    case NAME:
        asCELL(cg, e);
        break;

    case ZERONUM:
    case POSNUM:
    case NEGNUM:
        buildBignum(cg, e);
        break;

    case INTCELL:
        asINTEGER(cg, intOf(e));
        break;

    case DOUBLECELL:
        asDOUBLE(cg, doubleOf(e));
        break;

    case STRCELL:
        asSTRING(cg, textOf(e));
        break;

    case CHARCELL:
        asCHAR(cg, charOf(e));
        break;

    case OFFSET:
        asLOAD(cg, cg->offsPosn[offsetOf(e)]);
        break;

    default:
        internal("build");
    }
}

/* Generate code to compile list of guards to a conditional expr without
 * evaluation */
static void buildGuards(CodeGen *cg, List gs, int co) {
    if (isNull(gs)) {
        asCELL(cg, nameFail);
    } else {
        buildGuards(cg, tl(gs), co);
        build(cg, snd(hd(gs)), co);
        build(cg, fst(hd(gs)), co);
        asCELL(cg, nameIf);
        asMKAP(cg, 3);
    }
}

/* Generate code to build local var bindings on stack, with no eval */
static int buildLoc(CodeGen *cg, List vs, int co) {
    int n = length(vs);
    int i;

    for (i = 1; i <= n; i++)
        cg->offsPosn[co + i] = cg->srsp + i;
    asALLOC(cg, n);
    for (i = 1; i <= n; i++) {
        build(cg, hd(vs), co + n);
        asUPDATE(cg, cg->offsPosn[co + i]);
        vs = tl(vs);
    }
    return n;
}

/* Generate code to build bignum */
static void buildBignum(CodeGen *cg, Bignum b) {
    if (b == ZERONUM) {
        asCELL(cg, ZERONUM);
    } else {
        List rs = snd(b) = rev(snd(b));
        asCELL(cg, NIL);
        for (; nonNull(rs); rs = tl(rs)) {
            asCELL(cg, hd(rs));
            asMKAP(cg, 1);
        }
        snd(b) = rev(snd(b)); /* put digits back in order        */
        asCELL(cg, fst(b));
        asMKAP(cg, 1);
    }
}

/* --------------------------------------------------------------------------
 * Compile expression to code which will build expression evaluating guards
 * and testing cases to avoid building complete graph.
 * ------------------------------------------------------------------------*/

/* Code for a list of alternatives, ct being what compiles one of them:
 * each but the last lands on the next one when its test fails.
 */
static void makeTests(CodeGen *cg,
                      void (*ct)(CodeGen *, Cell, int, Label, Label, Label),
                      List tests, int co, Label f, Label d) {
    Label l1 = newLabel(cg, d);
    List xs = tests;
    while (nonNull(tl(xs))) {
        Label l2 = newLabel(cg, RUNON);
        int savesp = cg->srsp;
        ct(cg, hd(xs), co, f, l2, l1);
        atLabel(cg, l2);
        cg->srsp = savesp;
        xs = tl(xs);
    }
    ct(cg, hd(xs), co, f, f, d);
    endLabel(cg, d, l1);
}

/* Construct code to build e, given current offset co, and branch to f on
 * failure, d on completion */
static void make(CodeGen *cg, Cell e, int co, Label f, Label d) {
    stackCheck();
    switch (whatIs(e)) {

    case LETREC: {
        int n = buildLoc(cg, fst(snd(e)), co);
        if (d == UPDRET || d == VALRET)
            make(cg, snd(snd(e)), co + n, f, d);
        else {
            make(cg, snd(snd(e)), co + n, f, RUNON);
            asSLIDE(cg, n);
            asGOTO(cg, d);
        }
    } break;

    case FATBAR: {
        Label l1 = newLabel(cg, RUNON);
        Label l2 = newLabel(cg, d);
        int savesp = cg->srsp;

        make(cg, fst(snd(e)), co, l1, l2);

        atLabel(cg, l1);
        cg->srsp = savesp;
        asSETSTK(cg, cg->srsp);
        make(cg, snd(snd(e)), co, f, l2);

        endLabel(cg, d, l2);
    } break;

    case COND:
        makeCond(cg, fst3(snd(e)), snd3(snd(e)), thd3(snd(e)), co, f, d);
        break;

    case NUMCASE:
        makeNumcase(cg, snd(e), co, f, d);
        break;

    case CASE:
        make(cg, fst(snd(e)), co, SHOULDNTFAIL, RUNON);
        asEVAL(cg);
        makeTests(cg, testCase, snd(snd(e)), co, f, d);
        break;

    case GUARDED:
        makeTests(cg, testGuard, snd(e), co, f, d);
        break;

    case AP: {
        Cell h = getHead(e);
        if (h == nameAnd && argCount == 2) {
            /* x && y ==> if x then y else False      */
            makeCond(cg, arg(fun(e)), arg(e), nameFalse, co, f, d);
            break;
        } else if (h == nameOr && argCount == 2) {
            /* x || y ==> if x then True else y       */
            makeCond(cg, arg(fun(e)), nameTrue, arg(e), co, f, d);
            break;
        }
    }
        buildAp(cg, e, co, f, true);
        asGOTO(cg, d);
        break;

    case TUPLE:
    case NAME:
        asCELL(cg, e);
        asGOTO(cg, d);
        break;

    case ZERONUM:
    case POSNUM:
    case NEGNUM:
        buildBignum(cg, e);
        asGOTO(cg, d);
        break;

    case INTCELL:
        asINTEGER(cg, intOf(e));
        asGOTO(cg, d);
        break;

    case DOUBLECELL:
        asDOUBLE(cg, doubleOf(e));
        asGOTO(cg, d);
        break;

    case STRCELL:
        asSTRING(cg, textOf(e));
        asGOTO(cg, d);
        break;

    case CHARCELL:
        asCHAR(cg, charOf(e));
        asGOTO(cg, d);
        break;

    case OFFSET:
        asLOAD(cg, cg->offsPosn[offsetOf(e)]);
        asGOTO(cg, d);
        break;

    default:
        internal("make");
    }
}

/* Build code for conditional */
static void makeCond(CodeGen *cg, Cell i, Cell t, Cell e, int co, Label f,
                     Label d) {
    Label l1 = newLabel(cg, RUNON);
    Label l2 = newLabel(cg, d);
    int savesp;

    make(cg, i, co, f, RUNON);
    asEVAL(cg);

    savesp = cg->srsp;
    asTEST(cg, nameTrue, l1);
    make(cg, t, co, f, l2);

    cg->srsp = savesp;
    atLabel(cg, l1);
    make(cg, e, co, f, (d == RUNON ? d : l2));

    endLabel(cg, d, l2);
}

/* Build code for numcase */
static void makeNumcase(CodeGen *cg, Triple nc, int co, Label f, Label d) {
    Cell discr = snd3(nc);
    Cell h = getHead(discr);
    make(cg, fst3(nc), co, SHOULDNTFAIL, RUNON);
    switch (whatIs(h)) {
    case NAME:
        if (h == nameFromInt) {
            asINTEGER(cg, intOf(arg(discr)));
            make(cg, arg(fun(discr)), co, SHOULDNTFAIL, RUNON);
            asCELL(cg, namePmInt);
        } else if (h == nameFromInteger) {
            buildBignum(cg, arg(discr));
            make(cg, arg(fun(discr)), co, SHOULDNTFAIL, RUNON);
            asCELL(cg, namePmInteger);
        } else if (h == nameFromDouble) {
            asDOUBLE(cg, doubleOf(arg(discr)));
            make(cg, arg(fun(discr)), co, SHOULDNTFAIL, RUNON);
            asCELL(cg, namePmFlt);
        }
        asMKAP(cg, 3);
        asEVAL(cg);
        asTEST(cg, nameTrue, f);
        make(cg, thd3(nc), co, f, d);
        break;
    default:
        break;
    }
}

/* Produce code for guard */
static void testGuard(CodeGen *cg, Pair g, int co, Label f, Label cf, Label d) {
    if (fst(g) != nameTrue) {
        make(cg, fst(g), co, SHOULDNTFAIL, RUNON);
        asEVAL(cg);
        asTEST(cg, nameTrue, cf);
    }
    make(cg, snd(g), co, f, d);
}

/* Produce code for guard labels determine where to go if: match succeeds, but
 * rest fails this match fails */
static void testCase(CodeGen *cg, Pair c, int co, Label f, Label cf, Label d) {
    int n = discrArity(fst(c));
    int i;
    asTEST(cg, fst(c), cf);
    for (i = 1; i <= n; i++)
        cg->offsPosn[co + i] = ++cg->srsp;
    make(cg, snd(c), co + n, f, d);
}

/* --------------------------------------------------------------------------
 * We frequently encounter functions which call themselves recursively with
 * a number of initial arguments preserved:
 * e.g.  (map f) []     = []
 *       (map f) (x:xs) = f x : (map f) xs
 * Lambda lifting, in particular, is likely to introduce such functions.
 * Rather than reconstructing a new instance of the recursive function and
 * its arguments, we can extract the relevant portion of the root of the
 * current redex.
 *
 * The following functions implement this optimisation.
 * ------------------------------------------------------------------------*/

/* Determine if any portion of an application can be built using a portion of
 * the root */
static void analyseAp(CodeGen *cg, Cell e) {
    if (isAp(e)) {
        analyseAp(cg, fun(e));
        if (cg->nonRoots == 0 && cg->rootPortion > 1 && isOffset(arg(e)) &&
            offsetOf(arg(e)) == cg->rootPortion - 1)
            cg->rootPortion--;
        else
            cg->nonRoots++;
    } else if (e == cg->definingName)
        cg->rootPortion = cg->definingArity + 1;
    else
        cg->rootPortion = 0;
}

/* Build application, making use of root optimisation if poss. */
static void buildAp(CodeGen *cg, Cell e, int co, Label f, bool str) {
    int nr, rp, i;

    cg->nonRoots = 0;
    analyseAp(cg, e);
    nr = cg->nonRoots;
    rp = cg->rootPortion;

    for (i = 0; i < nr; ++i) {
        build(cg, arg(e), co);
        e = fun(e);
    }

    if (0 < rp && rp <= cg->definingArity) {
        asROOT(cg, rp - 1);
    } else if (str)
        make(cg, e, co, f, RUNON);
    else
        build(cg, e, co);

    if (nr > 0) {
        asMKAP(cg, nr);
    }
}

/* --------------------------------------------------------------------------
 * Code generator entry points:
 * ------------------------------------------------------------------------*/

/* Generate code for expression e, treating return value of CAFs differently to
 * functs with args */
Addr codeGen(Name n, int arity, Cell e) {
    CodeGen state;
    CodeGen *cg = &state;

#if DEBUG_CODE
    if (debugCode) {
        Printf("------------------\n");
        if (nonNull(n))
            Printf("name=%s\n", textToStr(nameRec(n)->text));
        Printf("Arity   = %d\n", arity);
        Printf("codeGen = ");
        printExp(stdout, e);
        Putchar('\n');
    }
#endif
    asSTART(cg);
    cg->definingName = n;
    cg->definingArity = arity;
    if (nonNull(n)) {
        int i;
        for (i = 1; i <= arity; i++)
            cg->offsPosn[i] = ++cg->srsp;
        make(cg, e, arity, noMatch, (arity > 0 ? UPDRET : VALRET));
    } else {
        build(cg, e, 0);
        asRETURN(cg);
    }
    asEND(cg);
#if DEBUG_CODE
    if (debugCode) {
        if (nonNull(n))
            Printf("name=%s\n", textToStr(nameRec(n)->text));
        dissassemble(cg->startInstr, cg->lastInstr);
        Printf("------------------\n");
    }
#endif
    return cg->startInstr;
}

/* Build implementation for constr fun c. scs lists integers (1..) in incr
 * order of strict comps. */
void implementCfun(Name c, List scs) {
    int a = nameRec(c)->arity;
    if (a == 0 || isNull(scs))
        nameRec(c)->defn = c; /* Name ==> no special imp.        */
    else {
        CodeGen state;
        CodeGen *cg = &state;
        Name n = newName(inventText(), c);
        int i = 1;
        nameRec(c)->defn = pair(scs, n); /* (scs,n) => strict components    */
        nameRec(n)->arity = a;           /* Initialize name data as approp. */

        asSTART(cg); /* inaccurate cg->srsp doesn't matter  */
        asCELL(cg, c);
        for (; i <= a; i++)
            if (nonNull(scs) && intOf(hd(scs)) == i) {
                asSTAP(cg);
                scs = tl(scs);
            } else
                asMKAP(cg, 1);
        asRUPDATE(cg);
        asEND(cg);
        nameRec(n)->code = cg->startInstr;
#if DEBUG_CODE
        if (debugCode) {
            Printf("Implement constructor ");
            printExp(stdout, c);
            Printf(" using ");
            printExp(stdout, n);
            Printf(" with code:\n");
            dissassemble(cg->startInstr, cg->lastInstr);
            Printf("------------------\n");
        }
#endif
    }
}

/* --------------------------------------------------------------------------
 * Evaluator:
 * ------------------------------------------------------------------------*/

int whnfArgs;       /* number of arguments of whnf term*/
Cell whnfHead;      /* head cell of term in whnf       */
int whnfInt;        /* value of INTCELL (in whnf)      */
Float whnfFloat;    /* value of FLOATCELL (in whnf)    */
Double whnfDouble;  /* value of DOUBLECELL (in whnf)   */
long numReductions; /* number of reductions counted    */
#if PROFILING
# define saveProducer(n)                                                       \
     {                                                                         \
         Name old = producer;                                                  \
         producer = n
# define restoreProducer()                                                     \
     producer = old;                                                           \
     if ((numReductions % profInterval) == 0)                                  \
         garbageCollect();                                                     \
     }
#else
# define saveProducer(n)   /* nothing */
# define restoreProducer() /* nothing */
#endif

static Cell exception;         /* Exception thrown                */
static jmp_buf *evalError = 0; /* jump buffer for eval errors     */

/* Graph reduction evaluator */
void eval(Cell n) {
    StackPtr base = sp;
    int ar;

    stackCheck();
    if (++evalDepth >= MAX_EVAL_DEPTH)
        cupidStackOverflow();

unw:
    switch (whatIs(n)) { /* unwind spine of application     */

    case AP:
        push(n);
        n = fun(n);
        goto unw;

    case INDIRECT:
        n = arg(n);
        allowBreak();
        goto unw;

    case NAME:
        allowBreak();
        {
#if DEBUG_CODE
            Name saveName = n;
            if (debugCode) {
                Printf("%*sEntering name(%d): %s\n", base, "", n - NAMEMIN,
                       textToStr(nameRec(n)->text));
            }
#endif
            if (!isCfun(n) && (ar = nameRec(n)->arity) <= (sp - base)) {
                if (ar > 0) { /* fn with args*/
                    StackPtr root;

                    push(NIL); /* rearrange   */
                    root = sp;
                    do {
                        stack(root) = arg(stack(root - 1));
                        --root;
                    } while (--ar > 0);

                    saveProducer(n);
                    if (nameRec(n)->primDef) /* reduce      */
                        (*nameRec(n)->primDef)(root);
                    else
                        run(nameRec(n)->code, root);
                    numReductions++;
                    restoreProducer();

                    sp = root; /* continue... */
                    n = pop();
                } else {                            /* CAF         */
                    if (isNull(nameRec(n)->defn)) { /* build CAF   */
                        StackPtr root = sp;
                        push(n); /* save CAF    */
                        saveProducer(n);
                        if (nameRec(n)->primDef)
                            (*nameRec(n)->primDef)(sp);
                        else
                            run(nameRec(n)->code, sp);
                        numReductions++;
                        restoreProducer();
                        nameRec(n)->defn = top();
                        sp = root; /* drop CAF    */
                    }
                    n = nameRec(n)->defn; /*already built*/
                    if (sp > base)
                        fun(top()) = n;
                }
#if DEBUG_CODE
                if (debugCode) {
                    Printf("%*sLeaving name(%d): %s\n", base, "",
                           saveName - NAMEMIN,
                           textToStr(nameRec(saveName)->text));
                }
#endif
                goto unw;
            }
        }
        break;

    case INTCELL:
        whnfInt = intOf(n);
        break;

    case FLOATCELL:
        whnfFloat = (Float)floatOf(n);
        break;

    case DOUBLECELL:
        whnfDouble = (Double)doubleOf(n);
        break;

    case STRCELL:
        evalString(n);
        goto unw;
    default:
        break;
    }

    whnfHead = n;         /* rearrange components of term on  */
    whnfArgs = sp - base; /* stack, now in whnf ...           */
    for (ar = whnfArgs; ar > 0; ar--) {
        fun(stack(base + ar)) = n;
        n = stack(base + ar);
        stack(base + ar) = arg(n);
    }
    evalDepth--;
}

/* Unwind spine of application; like eval except that we always treat the
 * expression n as if it were already in whnf. */
void unwind(Cell n) {
    whnfArgs = 0;

unw:
    switch (whatIs(n)) {
    case AP:
        push(arg(n));
        whnfArgs++;
        n = fun(n);
        goto unw;

    case INDIRECT:
        n = arg(n);
        allowBreak();
        goto unw;

    case INTCELL:
        whnfInt = intOf(n);
        break;

    case FLOATCELL:
        whnfFloat = (Float)floatOf(n);
        break;

    case DOUBLECELL:
        whnfDouble = (Double)doubleOf(n);
        break;

    case STRCELL:
        evalString(n);
        goto unw;
    default:
        break;
    }
    whnfHead = n;
}

/* Expand STRCELL at node n */
static void evalString(Cell n) {
    Text t = textOf(n);
    char *s = textToStr(t);
    Char c;
    if (*s == 0) {
        fst(n) = INDIRECT;
        snd(n) = nameNil;
        return;
    }
    c = getStrChr(&s);
    push(n); /* protect n during mkStr           */
    fst(n) = consChar(c);
    snd(n) = mkStr(t + (Text)(s - textToStr(t)));
    drop();
}

/* Execute code beginning at given address with local stack starting at given
 * root offset */
void run(Addr start, StackPtr root) {
    register Memory pc = memory + start;

#if !DEBUG_CODE && HAVE_LABELS_AS_VALUES
# define Ins(x) &&l##x
    static void *labs[] = {INSTRLIST};
# undef Ins
# define Case(x) l##x
# define Continue goto *labs[(pc++)->instr]
# define Dispatch Continue;
# define EndDispatch
#else
# if DEBUG_CODE
#  define Dispatch                                                             \
      for (;;) {                                                               \
          if (debugCode) {                                                     \
              Printf("%*s0x%04X: ", root, "", pc - memory);                    \
              dissInstr(pc - memory);                                          \
          }                                                                    \
          switch ((pc++)->instr) {
# else
#  define Dispatch                                                             \
      for (;;) {                                                               \
          switch ((pc++)->instr) {
# endif
# define Case(x) case x
# define Continue continue
# define EndDispatch                                                           \
 default:                                                                      \
     internal("illegal instruction");                                          \
     break;                                                                    \
     }                                                                         \
     }
#endif

    Dispatch

        Case(iLOAD)
        : push(stack(root + pc->mint)); /* load from stack*/
    pc++;
    Continue;

    Case(iCELL) : push(pc->cell); /* load const Cell*/
    pc++;
    Continue;

    Case(iCHAR) : push(mkChar(pc->mint)); /* load char const*/
    pc++;
    Continue;

    Case(iINT) : push(mkInt(pc->mint)); /* load int const */
    pc++;
    Continue;

    Case(iDOUBLE)
        : push(mkDouble(doubleFromParts /* load dbl const */
                        (pc->cell, (pc + 1)->cell)));
    pc += 2;
    Continue;

    Case(iSTRING) : push(mkStr(pc->text)); /* load str const */
    pc++;
    Continue;

    Case(iMKAP) : {
        int i = pc->mint; /* make AP nodes  */
        while (0 < i--) {
            pushed(1) = ap(pushed(0), pushed(1));
            drop();
        }
    }
    pc++;
    Continue;

    Case(iUPDATE) : {
        Cell t = stack(root /* update cell ...*/
                       + pc->mint);
        Cell r = pop();
        while (isPair(r) && fst(r) == INDIRECT)
            r = snd(r);
        fst(t) = INDIRECT;
        snd(t) = r;
    }
    pc++;
    Continue;

    Case(iRUPDATE) : {
        Cell t = stack(root); /* update and ret */
        Cell r = top();
        while (isPair(r) && fst(r) == INDIRECT)
            r = snd(r);
        fst(t) = INDIRECT;
        snd(t) = r;
    }
    return;

    Case(iUPDAP) : {
        Cell t = stack(root /* update AP node */
                       + pc->mint);
        fst(t) = pop();
        snd(t) = pop();
    }
    pc++;
    Continue;

    Case(iRUPDAP) : {
        Cell t = stack(root); /* updap and ret  */
        fst(t) = pop();
        snd(t) = top();
    }
    return;

    Case(iEVAL) : eval(pop()); /* evaluate top() */
    Continue;

    Case(iLEVAL) : eval(stack(root + pc->mint)); /* eval from stack*/
    pc++;
    Continue;

    Case(iSTAP) : eval(pushed(1)); /* strict apply   */
    sp -= whnfArgs;
    pushed(1) = ap(top(), pushed(1));
    drop();
    Continue;

    Case(iRETURN) : return; /* terminate      */

    Case(iTEST)
        : if (whnfHead == pc->cell) /* test for cell  */
          pc += 2;
    else pc += 1 + (pc + 1)->addr;
    Continue;

    Case(iGOTO) : pc += pc->addr; /* goto label     */
    Continue;

    Case(iSETSTK) : sp = root + pc->mint; /* set stack ptr  */
    pc++;
    Continue;

    Case(iALLOC) : {
        int i = pc->mint; /* alloc loc vars */
        chkStack(i);
        while (0 < i--)
            onto(ap(NIL, NIL));
    }
    pc++;
    Continue;

    Case(iROOT) : {
        Cell t = stack(root); /* partial root   */
        int i = pc->mint;
        while (fst(t) == INDIRECT) {
            allowBreak();
            t = arg(t);
        }
        while (0 < i--) {
            t = fun(t);
            while (fst(t) == INDIRECT) {
                allowBreak();
                t = arg(t);
            }
        }
        push(t);
    }
    pc++;
    Continue;
    Case(iSLIDE) : pushed(pc->mint) = top(); /* remove loc vars*/
    sp -= pc->mint;
    pc++;
    Continue;

    Case(iTABLE) : Case(iFAIL) : evalFails(root); /* cannot reduce  */
    return;                                       /*NOT REACHED*/

    EndDispatch

#undef Dispatch
#undef Case
#undef Continue
#undef EndDispatch
}

/* Evaluate expression, returning NIL if successful, Exception value if not... */
Cell evalWithNoError(Cell e) {
    Cell caughtEx;
    jmp_buf *oldCatch = evalError;
    int savedDepth = evalDepth; /* throwException() longjmps straight*/
    StackPtr savedSp = sp;      /* past eval()'s own bookkeeping, so */
                                /* an exception caught here must undo   */
                                /* whatever the aborted evaluation left */
                                /* behind: eval() only decrements       */
                                /* evalDepth, and restores sp, along    */
                                /* its normal return path.              */

    jmp_buf catcherr[1];
    evalError = catcherr;
    if (setjmp(catcherr[0]) == 0) {
        eval(e);
        caughtEx = NIL;
    } else
        caughtEx = exception;
    evalError = oldCatch;
    if (nonNull(caughtEx)) {
        evalDepth = savedDepth;
        sp = savedSp;
    }
    return caughtEx;
}

/* Eval of current redex fails */
void evalFails(StackPtr root) {
    Cell errorRedex = stack(root); /* get error & bypass indirections */
    while (isPair(errorRedex) && fst(errorRedex) == INDIRECT)
        errorRedex = snd(errorRedex);
    throwException(
        ap(namePatternMatchFail,
           ap(ap(ap(nameNPrint, mkInt(MIN_PREC)), errorRedex), nameNil)));
}

void throwException(Cell ex) {
    exception = ex;
    if (evalError)
        longjmp(*evalError, 1);
    else
        internal("uncaught exception");
}

/* --------------------------------------------------------------------------
 * Machine control:
 * ------------------------------------------------------------------------*/

/* Allocate the program memory and write out the FAIL every failing
 * match branches to; done once, before the Prelude is read. */
void installMachine(void) {
    /* prelude.h defines NUM_ADDRS; 0 is the upper bound (=> unbounded.) */
    /* the elements are MemCells, not pointers to them */
    dynMemory = allocDynTable(sizeof(MemCell), NUM_ADDRS, 0, "memory");
    memory = (Memory)(dynMemory->data);
    if (memory == 0)
        fatal("Cannot allocate program memory");
    /* the FAIL that every failing match branches to; it belongs to no
     * code generation run, so it is written straight out             */
    noMatch = getMem(1);
    instrAt(noMatch) = iFAIL;
}

/* Clear the exception jump buffer and the evaluator's recursion depth. */
void resetMachine(void) {
    evalError = 0;
    evalDepth = 0;
}

/* Free the program memory. */
void exitMachine(void) {
    if (dynMemory)
        freeDynTable(dynMemory);
}

/* ------------------------------------------------------------------------*/
