/* --------------------------------------------------------------------------
 * Command interpreter
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2005, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "command.h"
#include "connect.h"
#include "errors.h"
#include "script.h"
#include "input.h"
#include "opts.h"
#include "strutil.h"
#include "evaluator.h"
#include "machdep.h"
#include "output.h"
#include "module.h"
#include "timer.h" /* IWYU pragma: keep -- updateTimers, under WANT_TIMER */
#include <setjmp.h>
#include <ctype.h>

#include <stdio.h>

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static void interpreter(int, char *[]);
static void initInterpreter(void);
static void menu(void);
static void guidance(void);
static void forHelp(void);
static void changeDir(void);
static void load(void);
static void editor(void);
static void find(void);
static void setModule(void);
static void showtype(void);
static char *objToStr(Module, Cell);
static void info(void);
static void printSyntax(Name);
static void showInst(Inst);
static void describe(Text);
static void listNames(void);
static void expandPath(char *, char *, unsigned int);
static void browse(void);
static void initialize(int, char *[]);
static void clearEvalModule(void);

/* --------------------------------------------------------------------------
 * Local data areas:
 * ------------------------------------------------------------------------*/
static Text evalModule = 0;             /* Name of module we eval exprs in */
static char *defaultArgv[] = {"Cupid"}; /* program name */

/* --------------------------------------------------------------------------
 * UI interpreter initalization:
 * ------------------------------------------------------------------------*/
static void initialize(int argc, char *argv[]) {
    startEvaluator();

    setLastEdit((char *)0, 0);

#if HAVE_WINDOWS_H
# define DEFAULT_EDITOR "\\notepad.exe"
    /*
     * Check first to see if the user has explicitly defined
     * an editor via the environment variable EDITOR..
     */
    cupidEdit = strCopy(fromEnv("EDITOR", NULL));
    if (cupidEdit == NULL) {
        UINT rc;
        int notePadLen = strlen(DEFAULT_EDITOR);
        char *notePadLoc;
        /*
       * Nope, the default editor is used instead. In our case
       * this is 'notepad', which we assume is always residing
       * in the windows directory, so locate it first..
       * (it would be somewhat odd for a user not to have that
       * directory in his/her PATH, but the less we assume, the better.)
       */
        notePadLoc =
# if HAVE_ALLOCA
            alloca
# else
            _alloca
# endif
            (sizeof(char) * (MAX_PATH + notePadLen + 2));
        notePadLoc[0] = '&';
        rc = GetWindowsDirectory(notePadLoc + 1, MAX_PATH);
        if (!(rc == 0 || rc > MAX_PATH)) {
            strcat(notePadLoc, DEFAULT_EDITOR);
            cupidEdit = strCopy(notePadLoc);
        }
    }
#else
    cupidEdit = strCopy(fromEnv("EDITOR", NULL));
#endif

    readOptions("-p\"%s> \" -r$$", false);
    readOptionSettings();
    processOptionVector(argc, argv);

    /* Figure out what Prelude module we're using + hoist it in. */
    if (!loadPrelude())
        fatal("Unable to load Prelude");

    /* Add an empty module as the default, to avoid being inside the Prelude */
    addScriptName(STD_EMPTY_MODULE, true);

    /* We record the number of scripts that loading the Prelude
     * brought about, so that when the user comes to clear the module
     * stack (e.g., ":l<ENTER>"), only modules later than the Prelude
     * ones are scratched.
     */
    setScriptStableMark();

    addScriptsFromArgs(argc, argv);
    setCupidArgs(1, defaultArgv);

    clearEvalModule(); /* evaluate wrt last module by default */
    readScripts(0);
}

/* --------------------------------------------------------------------------
 * Printing the banner
 * ------------------------------------------------------------------------*/
static void printBanner(void);

static void printBanner(void) {
    Printf("Cupid %s, a Haskell interpreter written in C\n", versionString);
    Printf(
        "Derived from Hugs 98, Copyright (c) 1994-2005 Mark P Jones, Alastair Reid,\n");
    Printf(
        "the Yale Haskell Group and the OGI School of Science & Engineering at OHSU\n\n");

    FlushStdout();
}

/* --------------------------------------------------------------------------
 * Cupid entry point:
 * ------------------------------------------------------------------------*/

int main(int, char *[]); /* now every func has a prototype  */

int main(int argc, char *argv[]) {

    CStackBase = &argc; /* Save stack base for use in gc   */

    if (!initSystem()) {
        Printf("%0: failed to initialize, exiting\n", (argv ? argv[0] : ""));
        return 1;
    }
    installSignalHandlers();

    printBanner();

    interpreter(argc, argv);
    Printf("[Leaving Cupid]\n");
    exitSubsystems();
    shutdownCupid();

    return 0;
}

/* --------------------------------------------------------------------------
 * Shutdown interpreter.
 * ------------------------------------------------------------------------*/
void shutdownCupid(void) {
    /* Let go of dynamic storage */
    if (cupidEdit) {
        free(cupidEdit);
        cupidEdit = 0;
    }
    /* empties lastEdit state (and frees it up.) */
    setLastEdit((char *)0, 0);
    stopEvaluator();
}

/* --------------------------------------------------------------------------
 * Print Menu of list of commands:
 * ------------------------------------------------------------------------*/

static struct cmd cmds[] = {
    {":?", HELP},         {":cd", CHGDIR},  {":also", ALSO},
    {":type", TYPEOF},    {":!", SYSTEM},   {":load", LOAD},
    {":reload", RELOAD},  {":gc", COLLECT}, {":edit", EDIT},
    {":quit", QUIT},      {":set", SET},    {":find", FIND},
    {":names", NAMES},    {":info", INFO},  {":module", SETMODULE},
    {":browse", BROWSE},  {":main", MAIN},
#if EXPLAIN_INSTANCE_RESOLUTION
    {":xplain", XPLAIN},
#endif
    {":version", PNTVER}, {"", EVAL},       {0, 0}};

static void menu(void) {
    Printf("LIST OF COMMANDS:  Any command may be abbreviated to :c where\n");
    Printf("c is the first character in the full name.\n\n");
    Printf(":load <filenames>   load modules from specified files\n");
    Printf(":load               clear all files except prelude\n");
    Printf(":also <filenames>   read additional modules\n");
    Printf(":reload             repeat last load command\n");
    Printf(":edit <filename>    edit file\n");
    Printf(":edit               edit last module\n");
    Printf(":module <module>    set module for evaluating expressions\n");
    Printf("<expr>              evaluate expression\n");
    Printf(":type <expr>        print type of expression\n");
    Printf(":?                  display this list of commands\n");
    Printf(":set <options>      set command line options\n");
    Printf(":set                help on command line options\n");
    Printf(":names [pat]        list names currently in scope\n");
    Printf(":info <names>       describe named objects\n");
    Printf(":browse <modules>   browse names exported by <modules>\n");
    Printf(
        ":main <aruments>    run the main function with the given arguments\n");
#if EXPLAIN_INSTANCE_RESOLUTION
    Printf(":xplain <context>   explain instance resolution for <context>\n");
#endif
    Printf(":find <name>        edit module containing definition of name\n");
    Printf(":cd dir             change directory\n");
    Printf(":gc                 force garbage collection\n");
    Printf(":version            print Cupid version\n");
    Printf(":quit               exit Cupid interpreter\n");
}

static void guidance(void) {
    Printf("Command not recognised.  ");
    forHelp();
}

static void forHelp(void) {
    Printf("Type :? for help\n");
}

/* --------------------------------------------------------------------------
 * Change directory command:
 * ------------------------------------------------------------------------*/

/*
 * Poor man's path expansion: expand out ~/
 * expandedPath must have room for maxLen bytes, including the terminating
 * NUL; a path (or its expansion) that does not fit is a clean error, not
 * a truncated copy.
 */
static void expandPath(char *origPath, char *expandedPath,
                       unsigned int maxLen) {

    if (!origPath) {
        return;
    }

    /* If the original path starts with "~/", expand it. */
    if (*origPath == '~' && *(origPath + 1) == '/') {
        char *home = getenv("HOME");
        if (!home) {
            errHead(0);
            errPrintf(
                "Cannot expand \"~\": the HOME environment variable is not set");
            errFail();
        }
        if ((unsigned)snprintf(expandedPath, maxLen, "%s%s", home,
                               origPath + 1) >= maxLen) {
            errHead(0);
            errPrintf("Path \"%s\" is too long", origPath);
            errFail();
        }
        return;
    }
    if ((unsigned)snprintf(expandedPath, maxLen, "%s", origPath) >= maxLen) {
        errHead(0);
        errPrintf("Path \"%s\" is too long", origPath);
        errFail();
    }
}

/* Change directory */
static void changeDir(void) {
    char *path = readFilename();
    char expandedPath[FILENAME_MAX + 1];
    expandPath(path, expandedPath, sizeof(expandedPath));
    if (path && chdir(expandedPath)) {
        errHead(0);
        errPrintf("Unable to change to directory \"%s\"", path);
        errFail();
    }
}

/* --------------------------------------------------------------------------
 * Commands for loading and removing script files:
 * ------------------------------------------------------------------------*/

/* Read filenames from command line and add to list of scripts waiting to be
 * read */
static void load(void) {
    char *s;

    clearEvalModule();
    while ((s = readFilename()) != 0) {
        addScriptName(s, true);
    }
    readScripts(1);
}

/* --------------------------------------------------------------------------
 * Access to external editor:
 * ------------------------------------------------------------------------*/

/* Interpreter-editor interface */
static void editor(void) {
    char *newFile = readFilename();
    if (newFile) {
        setLastEdit(newFile, 0);
        if (readFilename()) {
            errHead(0);
            errPrintf("Multiple filenames not permitted");
            errFail();
        }
    }
    runEditor();
}

/* Edit file containing definition of specified name */
static void find(void) {
    char *nm = readFilename();
    if (!nm) {
        errHead(0);
        errPrintf("No name specified");
        errFail();
    } else if (readFilename()) {
        errHead(0);
        errPrintf("Multiple names not permitted");
        errFail();
    } else {
        Text t;
        Cell c;
        setCurrModule(findEvalModule());
        startNewScript(0);
        if (nonNull(c = findTycon(t = findText(nm)))) {
            if (startEdit(tyconRec(c)->line,
                          getScriptName(scriptThisTycon(c)))) {
                readScripts(1);
            }
        } else if (nonNull(c = findName(t))) {
            if (startEdit(nameRec(c)->line, getScriptName(scriptThisName(c)))) {
                readScripts(1);
            }
        } else {
            errHead(0);
            errPrintf("No current definition for name \"%s\"", nm);
            errFail();
        }
    }
}

/* Run editor on script lastEdit */
void runEditor(void) {
    char *fileToEdit;
    int lastLine;
    char *lastEdit = getLastEdit(&lastLine);

    if (lastEdit == NULL) {
        fileToEdit = fileOfModule(lastModule());
    } else {
        fileToEdit = lastEdit;
    }
    if (startEdit(lastLine, fileToEdit)) { /* at line lastLine              */
        /* reload entire module stack bar the Prelude. */
        readScripts(1);
    }
}

/* --------------------------------------------------------------------------
 * Read and evaluate an expression:
 * ------------------------------------------------------------------------*/

/* Set module in which to evaluate expressions */
static void setModule(void) {
    char *s = readFilename();
    if (s != 0) { /* Locate named module                           */
        Text t = findText(s);
        Module m = findModule(t);
        if (isNull(m)) {
            errHead(0);
            errPrintf("Cannot find module \"%s\"", s);
            errFail();
        } else {
            evalModule = t;
            setLastEdit(fileOfModule(m), 0);
        }
    } else { /* :m clears the current module selection */
        clearEvalModule();
        setLastEdit(fileOfModule(lastModule()), 0);
    }
}

/* Module in which to eval expressions */
Module findEvalModule(void) {
    Module m = findModule(evalModule);
    if (isNull(m))
        m = lastModule();
    return m;
}

static void clearEvalModule(void) {
    evalModule = findText("");
}

/* --------------------------------------------------------------------------
 * Print type of input expression:
 * ------------------------------------------------------------------------*/

/* Print type of expression (if any) */
static void showtype(void) {
    Cell type;

    setCurrModule(findEvalModule());
    startNewScript(0); /* Enables recovery of storage      */
                       /* allocated during evaluation      */
    parseExp();
    checkExp();
    defaultDefns = evalDefaults;
    type = typeCheckExp(printTypeUseDefaults);
    printExp(stdout, inputExpr);
    Printf(" :: ");
    printType(stdout, type);
    Putchar('\n');
}

/* Browse modules or current module */
static void browse(void) {
    int count = 0;
    char *s;
    bool all = false;

    setCurrModule(findEvalModule());
    startNewScript(0); /* for recovery of storage         */
    while ((s = readFilename()) != 0)
        if (strcmp(s, "all") == 0) {
            all = true;
        } else {
            Module mod = findModule(findText(s));
            if (isNull(mod)) {
                Printf("Unknown module %s\n", s);
            } else {
                browseModule(mod, all);
            }
            count++;
        }
    if (count == 0)
        browseModule(findEvalModule(), all);
}

#if EXPLAIN_INSTANCE_RESOLUTION
/* Print type of expression (if any) */
static void xplain(void) {
    Cell d;
    bool sir = showInstRes;

    setCurrModule(findEvalModule());
    startNewScript(0); /* Enables recovery of storage      */
                       /* allocated during evaluation      */
    parseContext();
    checkContext();
    showInstRes = true;
    d = provePred(NIL, NIL, hd(inputContext));
    if (isNull(d)) {
        fprintf(stdout, "not Sat\n");
    } else {
        fprintf(stdout, "Sat\n");
    }
    fflush(stdout);
    showInstRes = sir;
}
#endif

static void runmain(void) {
    int MaxArgs = 255;
    char *args[256];
    char *s;
    int argPos = 1, i;
    args[0] = "Cupid";

    while (argPos < MaxArgs && (s = readFilename())) {
        args[argPos++] = strCopy(s);
    }

    setCupidArgs(argPos, args);
    for (i = 1; i < argPos; i++)
        free(args[i]);

    stringInput((char *)"main");
    breakInput();
    doCommand();
}

/* --------------------------------------------------------------------------
 * Enhanced help system:  print current list of scripts or give information
 * about an object.
 * ------------------------------------------------------------------------*/

static char *objToStr(Module m, Cell c) {
    /* newVar grows to fit "module.name" for whatever names come up; it is
     * never shrunk, and is reused (and, if necessary, enlarged) on every
     * call, since the result is only ever used before the next call. */
    static char *newVar = 0;
    static size_t newVarLen = 0;
    char *qualifier = 0; /* module part, or 0 if unqualified*/
    char *unqual;        /* name part                       */
    size_t needed;

    switch (whatIs(c)) {
    case NAME:
        if (m != nameRec(c)->mod)
            qualifier = textToStr(moduleRec(nameRec(c)->mod)->text);
        unqual = textToStr(nameRec(c)->text);
        break;

    case TYCON:
        if (m != tyconRec(c)->mod)
            qualifier = textToStr(moduleRec(tyconRec(c)->mod)->text);
        unqual = textToStr(tyconRec(c)->text);
        break;

    case CLASS:
        if (m != cclassRec(c)->mod)
            qualifier = textToStr(moduleRec(cclassRec(c)->mod)->text);
        unqual = textToStr(cclassRec(c)->text);
        break;

    default:
        internal("objToStr");
        unqual = ""; /* not reached; quiets a warning   */
    }

    needed = strlen(unqual) + 1 + (qualifier ? strlen(qualifier) + 1 : 0);
    if (needed > newVarLen) {
        /* Into a temporary: errFail longjmps back to the command loop, so a
         * failure here must leave newVar and newVarLen as they were rather
         * than a null buffer still described by the old length. */
        char *grown = realloc(newVar, needed);
        if (!grown) {
            errHead(0);
            errPrintf("Cannot allocate name buffer");
            errFail();
        }
        newVar = grown;
        newVarLen = needed;
    }
    if (qualifier)
        snprintf(newVar, newVarLen, "%s.%s", qualifier, unqual);
    else
        snprintf(newVar, newVarLen, "%s", unqual);
    return newVar;
}

/* Describe objects or give menu of commands */
static void info(void) {
    int count = 0;
    char *s;
    Module evMod;

    evMod = findEvalModule();

    setCurrModule(evMod);
    startNewScript(0); /* for recovery of storage         */
    for (; (s = readFilename()) != 0; count++) {
        char *mod = NULL;
        char *nm = NULL;

        /* In the event of a qualified name, decompose it. */
        splitQualString(s, &mod, &nm);

        if (mod == NULL) {
            describe(findText(nm));
        } else {
            Module homeMod = findModule(findText(mod));
            if (nonNull(homeMod)) {
                setCurrModule(homeMod);
                describe(findText(nm));
            } else
                Printf("Unknown module `%s'\n", mod);
            /* With the module unknown, don't check the name. */
            free(mod);
            mod = NULL;
        }
    }
    if (count == 0) {
        whatScripts();
    }
    setCurrModule(evMod);
}

/* Describe an object */
static void describe(Text t) {
    Tycon tc = findTycon(t);
    Class cl = findClass(t);
    Name nm = findName(t);

    if (nonNull(tc)) { /* as a type constructor           */
        Type t = tc;
        int i;
        Inst in;
        for (i = 0; i < tyconRec(tc)->arity; ++i) {
            t = ap(t, mkOffset(i));
        }
        Printf("-- type constructor");
        if (kindExpert) {
            Printf(" with kind ");
            printKind(stdout, tyconRec(tc)->kind);
        }
        Putchar('\n');
        switch (tyconRec(tc)->what) {
        case SYNONYM:
            Printf("type ");
            printType(stdout, t);
            Printf(" = ");
            printType(stdout, tyconRec(tc)->defn);
            break;

        case NEWTYPE:
        case DATATYPE: {
            List cs = tyconRec(tc)->defn;
            if (tyconRec(tc)->what == DATATYPE) {
                Printf("data ");
            } else {
                Printf("newtype ");
            }
            printType(stdout, t);
            Putchar('\n');
            mapProc(printSyntax, cs);
            if (hasCfun(cs)) {
                Printf("\n-- constructors:");
            }
            for (; hasCfun(cs); cs = tl(cs)) {
                Putchar('\n');
                printExp(stdout, hd(cs));
                Printf(" :: ");
                printType(stdout, nameRec(hd(cs))->type);
            }
            if (nonNull(cs)) {
                Printf("\n-- selectors:");
            }
            for (; nonNull(cs); cs = tl(cs)) {
                Putchar('\n');
                printExp(stdout, hd(cs));
                Printf(" :: ");
                printType(stdout, nameRec(hd(cs))->type);
            }
        } break;

        case RESTRICTSYN:
            Printf("type ");
            printType(stdout, t);
            Printf(" = <restricted>");
            break;
        default:
            break;
        }
        Putchar('\n');
        if (nonNull(in = findFirstInst(tc))) {
            Printf("\n-- instances:\n");
            do {
                showInst(in);
                in = findNextInst(tc, in);
            } while (nonNull(in));
        }
        Putchar('\n');
    }

    if (nonNull(cl)) { /* as a class                      */
        List ins = cclassRec(cl)->instances;
        Kinds ks = cclassRec(cl)->kinds;
        if (nonNull(ks) && isNull(tl(ks)) && hd(ks) == STAR) {
            Printf("-- type class");
        } else {
            Printf("-- constructor class");
            if (kindExpert) {
                Printf(" with arity ");
                printKinds(stdout, ks);
            }
        }
        Putchar('\n');
        mapProc(printSyntax, cclassRec(cl)->members);
        Printf("class ");
        if (nonNull(cclassRec(cl)->supers)) {
            printContext(stdout, cclassRec(cl)->supers);
            Printf(" => ");
        }
        printPred(stdout, cclassRec(cl)->head);
        if (nonNull(cclassRec(cl)->fds)) {
            List fds = cclassRec(cl)->fds;
            char *pre = " | ";
            for (; nonNull(fds); fds = tl(fds)) {
                Printf(pre);
                printFD(stdout, hd(fds));
                pre = ", ";
            }
        }
        if (nonNull(cclassRec(cl)->members)) {
            List ms = cclassRec(cl)->members;
            Printf(" where");
            do {
                Type t = nameRec(hd(ms))->type;
                if (isPolyType(t)) {
                    t = monotypeOf(t);
                }
                Printf("\n  ");
                printExp(stdout, hd(ms));
                Printf(" :: ");
                if (isNull(tl(fst(snd(t))))) {
                    t = snd(snd(t));
                } else {
                    t = ap(QUAL, pair(tl(fst(snd(t))), snd(snd(t))));
                }
                printType(stdout, t);
                ms = tl(ms);
            } while (nonNull(ms));
        }
        Putchar('\n');
        if (nonNull(ins)) {
            Printf("\n-- instances:\n");
            do {
                showInst(hd(ins));
                ins = tl(ins);
            } while (nonNull(ins));
        }
        Putchar('\n');
    }

    if (nonNull(nm)) { /* as a function/name              */
        printSyntax(nm);
        printExp(stdout, nm);
        Printf(" :: ");
        if (nonNull(nameRec(nm)->type)) {
            printType(stdout, nameRec(nm)->type);
        } else {
            Printf("<unknown type>");
        }

        if (isCfun(nm)) {
            Printf("  -- data constructor");
        } else if (isMfun(nm)) {
            Printf("  -- class member");
        } else if (isSfun(nm)) {
            Printf("  -- selector function");
        }
        if (nameRec(nm)->primDef) {
            Printf("   -- primitive");
        }
        Printf("\n\n");
    }

    if (isNull(tc) && isNull(cl) && isNull(nm)) {
        Printf("Unknown reference `%s'\n", textToStr(t));
    }
}

static void printSyntax(Name nm) {
    Syntax sy = syntaxOf(nm);
    Text t = nameRec(nm)->text;
    char *s = textToStr(t);
    if (sy != defaultSyntax(t)) {
        Printf("infix");
        switch (assocOf(sy)) {
        case LEFT_ASS:
            Putchar('l');
            break;
        case RIGHT_ASS:
            Putchar('r');
            break;
        case NON_ASS:
            break;
        default:
            break;
        }
        Printf(" %i ", precOf(sy));
        if (isascii(*s) && isalpha(*s)) {
            Printf("`%s`", s);
        } else {
            Printf("%s", s);
        }
        Putchar('\n');
    }
}

/* Display instance decl header */
static void showInst(Inst in) {
    Printf("instance ");
    if (nonNull(instRec(in)->specifics)) {
        printContext(stdout, instRec(in)->specifics);
        Printf(" => ");
    }
    printPred(stdout, instRec(in)->head);
    Putchar('\n');
}

/* --------------------------------------------------------------------------
 * List all names currently in scope:
 * ------------------------------------------------------------------------*/

/* List names matching optional pat */
static void listNames(void) {
    char *pat = readFilename();
    List names = NIL;
    int width = getTerminalWidth() - 1;
    int count = 0;
    int termPos;
    Module mod = findEvalModule();

    if (pat) { /* First gather names to list      */
        do {
            names = addNamesMatching(pat, names);
        } while ((pat = readFilename()) != 0);
    } else {
        names = addNamesMatching((char *)0, names);
    }
    if (isNull(names)) { /* Then print them out             */
        errHead(0);
        errPrintf("No names selected");
        errFail();
    }
    for (termPos = 0; nonNull(names); names = tl(names)) {
        char *s = objToStr(mod, hd(names));
        int l = (int)strlen(s);
        if (termPos + 1 + l > width) {
            Putchar('\n');
            termPos = 0;
        } else if (termPos > 0) {
            Putchar(' ');
            termPos++;
        }
        Printf("%s", s);
        termPos += l;
        count++;
    }
    Printf("\n(%d names listed)\n", count);
}

/* --------------------------------------------------------------------------
 * print a prompt and read a line of input:
 * ------------------------------------------------------------------------*/

/* Size of (expanded) prompt buffer, should be more than enough.... */
#define MAX_PROMPT_SIZE 1000

void promptForInput(char *moduleName) {
    char promptBuffer[MAX_PROMPT_SIZE];
    char *fromPtr;
    char *toPtr;
    int modLen = (int)strlen(moduleName);
    int roomLeft = MAX_PROMPT_SIZE - 1;

    toPtr = promptBuffer;
    fromPtr = prompt;

    /* Carefully substituting occurrences of %s in the
       prompt string with the module name.
    */
    while (*fromPtr != '\0' && roomLeft > 0) {
        if (*fromPtr == '%' && *(fromPtr + 1) == 's') {
            /* Substitute module name */
            if (modLen > roomLeft) {
                /* Running out of room; copy what we can */
                fromPtr = moduleName;
                while (roomLeft-- > 0) {
                    *toPtr++ = *fromPtr++;
                }
                break;
            } else {
                strcpy(toPtr, moduleName);
                toPtr += modLen;
                roomLeft -= modLen;
                fromPtr += 2;
            }
        } else {
            *toPtr++ = *fromPtr++;
            roomLeft--;
        }
    }
    *toPtr = '\0';

    consoleInput(promptBuffer);
}

/* --------------------------------------------------------------------------
 * main read-eval-print loop, with error trapping:
 * ------------------------------------------------------------------------*/

/* Main interpreter loop */
static void interpreter(int argc, char *argv[]) {
    int errorNumber = setjmp(catch_error);

    breakOn(true);                 /* enable break trapping           */
    if (numLoadedScripts() == 0) { /* only succeeds on first time,    */
        if (errorNumber)           /* before Prelude has been loaded  */
            fatal("Unable to load Prelude");
        initialize(argc, argv);
        forHelp();
    }

    for (;;) {
        initInterpreter();
        if (doCommand())
            break;
    }

    breakOn(false);
}

static void initInterpreter(void) {
    resetSubsystems(); /* reset to sensible initial state */
    /* remove partially loaded scripts */
    dropScriptsFrom(numLoadedScripts());

    promptForInput(textToStr(moduleRec(findEvalModule())->text));
}

/* Read and execute a command returns true on QUIT (:quit) */
bool doCommand(void) {
    Command cmd;
    cmd = readCommand(cmds, (Char)':', (Char)'!');
#if WANT_TIMER
    updateTimers();
#endif
    switch (cmd) {
    case EDIT:
        editor();
        break;
    case FIND:
        find();
        break;
    case LOAD:
        forgetAllScripts();
        load();
        break;
    case ALSO:
        forgetScriptsFrom(numLoadedScripts());
        load();
        break;
    case RELOAD:
        readScripts(1);
        break;
    case SETMODULE:
        setModule();
        break;
    case EVAL:
        evaluator(findEvalModule());
        break;
    case TYPEOF:
        showtype();
        break;
    case BROWSE:
        browse();
        break;
#if EXPLAIN_INSTANCE_RESOLUTION
    case XPLAIN:
        xplain();
        break;
#endif
    case NAMES:
        listNames();
        break;
    case HELP:
        menu();
        break;
    case BADCMD:
        guidance();
        break;
    case SET:
        setOptions();
        break;
    case SYSTEM:
        if (shellEsc(readLine()))
            Printf("Warning: Shell escape terminated abnormally\n");
        break;
    case CHGDIR:
        changeDir();
        break;
    case INFO:
        info();
        break;
    case PNTVER:
        Printf("-- Cupid Version %s\n", versionString);
        break;
    case QUIT:
        return true;
    case COLLECT:
        consGC = false;
        garbageCollect();
        consGC = true;
        Printf("Garbage collection recovered %d cells\n", cellsRecovered);
        break;
    case NOCMD:
        break;
    case MAIN:
        runmain();
        break;
    default:
        break;
    }
#if WANT_TIMER
    updateTimers();
    Printf("Elapsed time (ms): %ld (user), %ld (system)\n",
           millisecs(userElapsed), millisecs(systElapsed));
#endif
    return false;
}

/*-------------------------------------------------------------------------*/
