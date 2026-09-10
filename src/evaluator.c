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
Int hpSize = DEFAULTHEAP;   /* Desired heap size               */
String cuddlesPath = 0;     /* String for file search path     */
String cuddlesSuffixes = 0; /* Source filename suffixes        */

/* --------------------------------------------------------------------------
 * Evaluator initialization:
 * ------------------------------------------------------------------------*/
Void startEvaluator(Void) {
    initScripts();

    cuddlesPath = uniqPath(strCopy(CUDDLESPATH));
    cuddlesSuffixes = strCopy(CUDDLESSUFFIXES);
}

/* --------------------------------------------------------------------------
 * Shutdown evaluator.
 * ------------------------------------------------------------------------*/
Void stopEvaluator() {
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

static Type local typeInputExpr(Module m) /* Check the expression waiting in */
{                                         /* the input buffer, in module m   */
    setCurrModule(m);
    scriptFile = 0;
    startNewScript(0); /* Enables recovery of storage      */
                       /* allocated during evaluation      */
    parseExp();
    checkExp();
    defaultDefns = evalDefaults;
    return typeCheckExp(TRUE);
}

static Void local buildInputGraph(Void) /* Compile the checked expression   */
{                                       /* and build the graph to reduce    */
    inputExpr = ap(nameIORun, inputExpr);
    compileExp();
    clearStack();
    run(inputCode, sp);
}

static Void local startEval(Void) /* Reset the counters before reducing */
{
    numCells = 0;
    numReductions = 0;
    numGcs = 0;
    printing = TRUE;
    consGC = FALSE;
}

Int runProgram(Module m) /* Run main in module m and return the exit  */
{                        /* code the program asked for                */
    Cell temp = NIL;
    Int exitCode = 0;

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

Void evaluator(Module m) { /* evaluate expr and print value    */
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
                    printing = FALSE;
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
                printing = FALSE;
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
Void loadPrelude() { /* load in the Prelude module(s). */
    String prelLocation;
    Bool listFlg;

    if (!cuddlesPath)
        fatal("Cuddles search path not defined");

    if (!(prelLocation = findMPathname(STD_PRELUDE_CUDDLES))) {
        Printf("%s not found on current path: \"%s\"\n", STD_PRELUDE_CUDDLES,
               cuddlesPath);
        fatal("Unable to load prelude implementation");
    }
    addScriptName(prelLocation, FALSE);

    /* add the H98 Prelude module to the stack */
    if (!(prelLocation = findMPathname(STD_PRELUDE))) {
        Printf("%s not found on current path: \"%s\"\n", STD_PRELUDE,
               cuddlesPath);
        fatal("Unable to load prelude");
    }
    addScriptName(prelLocation, FALSE);

    everybody(INSTALL);

    /* Hack to temporarily turn off 'listScripts' feature. */
    listFlg = listScripts;
    listScripts = FALSE;
    readScripts(0);
    listScripts = listFlg;
}

/* --------------------------------------------------------------------------
 * Send message to each component of system:
 * ------------------------------------------------------------------------*/

Void everybody(Int what) /* send command `what' to each component of*/
{                        /* system to respond as appropriate ...    */
    machdep(what);       /* The order of calling each component is  */
    storage(what);       /* important for the INSTALL command       */
    substitution(what);
    sccControl(what);
    input(what);
    staticAnalysis(what);
    typeChecker(what);
    compiler(what);
    machine(what);
    charOps(what);
    builtIn(what);
    controlFuns(what);
    ffi(what);
    script(what);
}
