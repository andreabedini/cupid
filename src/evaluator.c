/*
 * The Cuddles evaluator / command interpreter + support functions.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 */
#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "script.h"
#include "output.h"
#include "strutil.h"
#include "opts.h"
#include "machdep.h"
#include "evaluator.h"
#include "scc.h"
#include "subst.h"
#include "input.h"
#include "static.h"
#include "char.h"
#include "ffi.h"
#include "prims/builtin.h"
#include "timer.h" /* IWYU pragma: keep -- updateTimers, under WANT_TIMER */

#if HAVE_WINDOWS_H
# include <windows.h>
#endif

/* --------------------------------------------------------------------------
 * Flags and options:
 *
 * Note: definitions here are restricted to interpreter state that's independent
 *       of the UI used.
 *
 * ------------------------------------------------------------------------*/
int hpSize = DEFAULTHEAP;  /* Desired heap size               */
char *cuddlesPath = 0;     /* String for file search path     */
char *cuddlesSuffixes = 0; /* Source filename suffixes        */

/* --------------------------------------------------------------------------
 * Evaluator initialization:
 * ------------------------------------------------------------------------*/
void startEvaluator(void) {
    initScripts();

    cuddlesPath = uniqPath(strCopy(CUDDLESPATH));
    cuddlesSuffixes = strCopy(CUDDLESSUFFIXES);
}

/* --------------------------------------------------------------------------
 * Shutdown evaluator.
 * ------------------------------------------------------------------------*/
void stopEvaluator(void) {
    /* Let go of dynamic storage */
    if (cuddlesPath) {
        free(cuddlesPath);
        cuddlesPath = 0;
    }
    if (cuddlesSuffixes) {
        free(cuddlesSuffixes);
        cuddlesSuffixes = 0;
    }
    if (prompt) {
        free(prompt);
        prompt = 0;
    }
    if (repeatStr) {
        free(repeatStr);
        repeatStr = 0;
    }
}

/* --------------------------------------------------------------------------
 * Compiling and running an expression:
 *
 * The prompt and runcuddles both take an expression, check it against the
 * module it is to run in and build its graph; what they do with the result
 * is where they differ, so the three steps they share are here.
 * ------------------------------------------------------------------------*/

/* Check the expression waiting in the input buffer, in module m */
static Type typeInputExpr(Module m) {
    setCurrModule(m);
    scriptFile = 0;
    startNewScript(0); /* Enables recovery of storage      */
                       /* allocated during evaluation      */
    parseExp();
    checkExp();
    defaultDefns = evalDefaults;
    return typeCheckExp(true);
}

/* Compile the checked expression and build the graph to reduce */
static void buildInputGraph(void) {
    inputExpr = ap(nameIORun, inputExpr);
    compileExp();
    clearStack();
    run(inputCode, sp);
}

/* Reset the counters before reducing */
static void startEval(void) {
    numCells = 0;
    numReductions = 0;
    numGcs = 0;
    printing = true;
    consGC = false;
}

/* Run main in module m and return the exit code the program asked for */
int runProgram(Module m) {
    Cell temp = NIL;
    int exitCode = 0;

    stringInput("main >> return () :: IO ()");
    typeInputExpr(m);
    buildInputGraph();
    startEval();

    if (nonNull(temp = evalWithNoError(pop())))
        abandon("Program execution", temp);
    if (whnfHead == nameLeft) { /* Left n  => the program exited with n */
        if (nonNull(temp = evalWithNoError(pop())))
            abandon("Program execution", temp);
        exitCode = whnfInt;
    } else /* Right () => it ran to the end        */
        drop();
    stopAnyPrinting();
    return exitCode;
}

/* Evaluate expr and print value */
void evaluator(Module m) {
    Type type, bd, t;
    Kinds ks = NIL;
    Cell temp = NIL;

    type = typeInputExpr(m);
    if (isPolyType(type)) {
        ks = polySigOf(type);
        bd = monotypeOf(type);
    } else
        bd = type;

    if (whatIs(bd) == QUAL) {
        errHead(0);
        errPrintf("Unresolved overloading");
        errPrintf("\n*** Type       : ");
        errType(type);
        errPrintf("\n*** Expression : ");
        errExpr(inputExpr);
        errPrintf("\n");
        errFail();
    }

#if PROFILING
    if (profiling)
        profilerLog("profile.hp");
    numReductions = 0;
    garbageCollect();
#endif

#if WANT_TIMER
    updateTimers();
#endif
    if ((t = getProgType(ks, type)) != 0) {
        if (displayIO) {
            Cell printer = namePrint;
            if (useShow) {
                Cell d = resolvePred(ks, ap(classShow, t));
                if (isNull(d)) {
                    printing = false;
                    errHead(0);
                    errPrintf("Cannot find \"show\" function for IO result:");
                    errPrintf("\n*** Expression : ");
                    errExpr(inputExpr);
                    errPrintf("\n*** Of type    : ");
                    errType(type);
                    errPrintf("\n");
                    errFail();
                }
                printer = ap(nameShowsPrec, d);
            }
            printer = ap(ap(nameFlip, ap(printer, mkInt(MIN_PREC))), nameNil);
            printer = ap(ap(nameComp, namePutStr), printer);
            inputExpr = ap(ap(nameIOBind, inputExpr), printer);
        }
    } else {
        Cell printer = namePrint;
        if (useShow) {
            Cell d = resolvePred(ks, ap(classShow, bd));
            if (isNull(d)) {
                printing = false;
                errHead(0);
                errPrintf("Cannot find \"show\" function for:");
                errPrintf("\n*** Expression : ");
                errExpr(inputExpr);
                errPrintf("\n*** Of type    : ");
                errType(type);
                errPrintf("\n");
                errFail();
            }
            printer = ap(nameShowsPrec, d);
        }
        inputExpr = ap(ap(ap(printer, mkInt(MIN_PREC)), inputExpr), nameNil);
        inputExpr = ap(namePutStr, inputExpr);
    }
    buildInputGraph();
#if DEBUG_CODE
    if (debugCode) {
        Printf("evaluator() builds: ");
        printExp(stdout, top());
        Putchar('\n');
    }
#endif
    startEval();
    if (nonNull(type) && addType) {
        onto(NIL);
        pushed(0) = pushed(1);
        pushed(1) = type;
        if (nonNull(temp = evalWithNoError(pop()))) {
            abandon("Program execution", temp);
        }
        drop();
        if (whnfHead == nameRight) {
            Printf(" :: ");
            printType(stdout, pop());
        }
    } else {
        if (nonNull(temp = evalWithNoError(pop()))) {
            abandon("Program execution", temp);
        }
    }
    stopAnyPrinting();
}

/* --------------------------------------------------------------------------
 * Read in prelude module(s):
 * ------------------------------------------------------------------------*/
/* Load in the Prelude module(s).
 *
 * Returns false, having printed why, when there is no search path or
 * either Prelude script is not on it; what to do then is the program's
 * decision.  An error inside a Prelude script is reported like any other
 * script error, by errFail returning to the caller's handler. */
bool loadPrelude(void) {
    char *prelLocation;
    bool listFlg;

    if (!cuddlesPath) {
        Printf("Cuddles search path not defined\n");
        return false;
    }

    if (!(prelLocation = findMPathname(STD_PRELUDE_CUDDLES))) {
        Printf("%s not found on current path: \"%s\"\n", STD_PRELUDE_CUDDLES,
               cuddlesPath);
        return false;
    }
    addScriptName(prelLocation, false);

    /* add the H98 Prelude module to the stack */
    if (!(prelLocation = findMPathname(STD_PRELUDE))) {
        Printf("%s not found on current path: \"%s\"\n", STD_PRELUDE,
               cuddlesPath);
        return false;
    }
    addScriptName(prelLocation, false);

    installSubsystems();

    /* Hack to temporarily turn off 'listScripts' feature. */
    listFlg = listScripts;
    listScripts = false;
    readScripts(0);
    listScripts = listFlg;
    return true;
}

/* --------------------------------------------------------------------------
 * The interpreter's lifecycle: one row per subsystem, naming its hooks
 * for the five phases a null hook means it has nothing to do in.  The
 * order is the order INSTALL runs the subsystems in, which matters (the
 * type checker's wired-in types must exist before builtIn predefines
 * names that use them, and so on); RESET, MARK, EXIT and BREAK keep the
 * same order for the same reason everybody() always used one order for
 * every phase.
 * ------------------------------------------------------------------------*/
static const Subsystem subsystems[] = {
    {"machdep", installMachdep, resetMachdep, NULL, exitMachdep, exitMachdep},
    {"storage", installStorage, resetStorage, markStorage, exitStorage, NULL},
    {"substitution", installSubstitution, resetSubstitution, markSubstitution,
     exitSubstitution, NULL},
    {"scc", installScc, resetScc, NULL, NULL, NULL},
    {"input", installInput, resetInput, NULL, NULL, breakInput},
    {"staticAnalysis", installStaticAnalysis, resetStaticAnalysis, NULL, NULL,
     NULL},
    {"typeChecker", installTypeChecker, resetTypeChecker, markTypeChecker, NULL,
     NULL},
    {"compiler", installCompiler, resetCompiler, NULL, NULL, NULL},
    {"machine", installMachine, resetMachine, NULL, exitMachine, NULL},
    {"charOps", installCharOps, NULL, markCharOps, exitCharOps, NULL},
    {"builtIn", installBuiltIn, resetBuiltIn, NULL, NULL, NULL},
    {"controlFuns", installControlFuns, resetControlFuns, markControlFuns,
     exitControlFuns, breakControlFuns},
    {"ffi", installFfi, resetFfi, NULL, NULL, NULL},
    {"script", NULL, NULL, NULL, exitScript, NULL},
};

#define NUM_SUBSYSTEMS (sizeof(subsystems) / sizeof(subsystems[0]))

/* Call every subsystem's install hook, in table order; once, before the
 * Prelude is read. */
void installSubsystems(void) {
    size_t i;
    for (i = 0; i < NUM_SUBSYSTEMS; i++)
        if (subsystems[i].install)
            subsystems[i].install();
}

/* Call every subsystem's reset hook, in table order; before each script
 * load or evaluation. */
void resetSubsystems(void) {
    size_t i;
    for (i = 0; i < NUM_SUBSYSTEMS; i++)
        if (subsystems[i].reset)
            subsystems[i].reset();
}

/* Call every subsystem's mark hook, in table order; during garbage
 * collection, to mark structured data a registered root table cannot
 * describe. */
void markSubsystems(void) {
    size_t i;
    for (i = 0; i < NUM_SUBSYSTEMS; i++)
        if (subsystems[i].gcMark)
            subsystems[i].gcMark();
}

/* Call every subsystem's exit hook, in table order; just before exit(). */
void exitSubsystems(void) {
    size_t i;
    for (i = 0; i < NUM_SUBSYSTEMS; i++)
        if (subsystems[i].atExit)
            subsystems[i].atExit();
}

/* Call every subsystem's break hook, in table order; after an
 * interrupt. */
void breakSubsystems(void) {
    size_t i;
    for (i = 0; i < NUM_SUBSYSTEMS; i++)
        if (subsystems[i].onBreak)
            subsystems[i].onBreak();
}
