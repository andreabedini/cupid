/*
 * Maintaining a stack of files / scripts.
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
#include "machdep.h"
#include "opts.h"
#include "strutil.h"
#include "script.h"

/* --------------------------------------------------------------------------
 * Local script state:
 * ------------------------------------------------------------------------*/

/*
 * A script is described by one record in the table storage.c keeps, so
 * that a script's name and the state of the tables before it was read
 * carry the same number.  This module fills in the name half; see the
 * script file control section of storage.h.
 */
CUDDLES_inline struct strScriptName *scriptName(Script s) {
    return &scriptRec(s)->name;
}

static int numScripts;    /* Number of scripts loaded        */
static int namesUpto;     /* Number of script names set      */
static int scriptsStable; /* Number of (Prelude) scripts     */
                          /* considered 'stable'             */
                          /* (=> won't be nuked when clearing */
                          /* the script stack / reloading.)   */

static bool needsImports; /* set to true if imports required */
char *scriptFile;         /* Name of current script (if any) */

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/
static bool addScript(char *, long);
static void freeScript(int);

/* --------------------------------------------------------------------------
 * Initialising / freeing script stacks:
 * ------------------------------------------------------------------------*/
void initScripts(void) {
    scriptFile = 0;
    numScripts = 0;
    namesUpto = 0;
    scriptsStable = 0;
}

void stopScripts(void) {
    int i;

    for (i = 0; i < numScripts; i++)
        freeScript(i);
}

static void freeScript(int i) {
    if (scriptName(i)->fileName)
        free(scriptName(i)->fileName);
    if (scriptName(i)->realName)
        free(scriptName(i)->realName);
    if (scriptName(i)->directory)
        free(scriptName(i)->directory);
}

/* We record the number of scripts that loading the Prelude
 * brought about, so that when the user comes to clear the module
 * stack (e.g., ":l<ENTER>"), only modules later than the Prelude
 * ones are scratched.
 */
void setScriptStableMark(void) {
    scriptsStable = namesUpto;
}

/* Access the script name at index 's' */
char *getScriptName(Script s) {
    if (s >= 0 && s <= numScripts) {
        return scriptName(s)->fileName;
    } else {
        errHead(0);
        errPrintf("getScriptName: Illegal script index %d (max: %d)", s,
                  numScripts);
        errFail();
    }
    return NULL;
}

/* Access the path of script at index 's' */
char *getScriptRealName(Script s) {
    if (s >= 0 && s <= numScripts) {
        return scriptName(s)->realName;
    } else {
        errHead(0);
        errPrintf("getScriptRealName: Illegal script index %d (max: %d)", s,
                  numScripts);
        errFail();
    }
    return NULL;
}

/* Return number of on the stack, loaded or not. */
int getScriptHwMark(void) {
    return namesUpto;
}

/* Return number of currently loaded scripts */
int numLoadedScripts(void) {
    return numScripts;
}

/* --------------------------------------------------------------------------
 * Loading script files:
 * ------------------------------------------------------------------------*/

/* Add script to list of scripts to be read in ... true => requires pathname
 * search */
void addScriptName(char *s, bool sch) {
    if (namesUpto >= NUM_SCRIPTS) {
        errHead(0);
        errPrintf("Too many module files (maximum of %d allowed)", NUM_SCRIPTS);
        errFail();
        return;
    }
    if (sch) {
        if (isModuleId(s)) {
            char *location = findMPathname(s);
            if (!location) {
                errHead(0);
                errPrintf("Can't find module \"%s\"", s);
                errFail();
            }
            scriptName(namesUpto)->fileName = strCopy(location);
            scriptName(namesUpto)->directory = NULL;
        } else {
            scriptName(namesUpto)->fileName = strCopy(findPathname(s));
            scriptName(namesUpto)->directory =
                dirname(scriptName(namesUpto)->fileName);
        }
    } else {
        scriptName(namesUpto)->fileName = strCopy(s);
        scriptName(namesUpto)->directory = NULL;
    }
    scriptName(namesUpto)->realName =
        strCopy(RealPath(scriptName(namesUpto)->fileName));
    scriptName(namesUpto)->chased = !sch;
    namesUpto++;
}

/* Read single script file name of script file length of script file */
static bool addScript(char *fname, long len) {

    if (!quiet) {
        Printf("Reading file \"%s\":\n", fname);
        FlushStdout();
    }
    setLastEdit(fname, 0);

    needsImports = false;
    scriptFile = 0;
    if (!parseScript(fname, len)) { /* process script file */
        /* file or parse error, drop the script */
        forgetAScript(numScripts);
        errFail();
    }
    if (needsImports)
        return false;
    checkDefns();
    typeCheckDefns();
    compileDefns();
    scriptFile = 0;
    preludeLoaded = true;
    return true;
}

/* Process list of import requests */
bool chase(List imps) {
    int origPos = numScripts; /* keep track of original position */
    char *origDir = scriptName(origPos)->directory;
    for (; nonNull(imps); imps = tl(imps)) {
        char *modname = textToStr(textOf(hd(imps)));
        char *iname = NULL;
        char *rname;
        bool inOrigDir = false;
        int i = 0;

        if (origDir) {
            iname = findMInDir(origDir, modname);
            if (iname)
                inOrigDir = true;
        }
        if (iname == NULL)
            iname = findMPathname(modname);
        if (iname == NULL) {
            errHead(0);
            errPrintf("Can't find imported module \"%s\"", modname);
            errFail();
        }

        rname = RealPath(iname);
        for (; i < namesUpto; i++)
            if (filenamecmp(scriptName(i)->realName, rname) == 0)
                break;
        if (i >= origPos) { /* Neither loaded or queued        */
            struct strScriptName tmpScript;

            scriptName(origPos)->postponed = true;
            needsImports = true;

            if (i >= namesUpto) { /* Name not found (i==namesUpto)   */
                addScriptName(iname, false);
                if (inOrigDir)
                    scriptName(i)->directory = strCopy(origDir);
            } else if (scriptName(i)->postponed) { /* imported by itself? */
                errHead(0);
                errPrintf(
                    "Recursive import dependency between \"%s\" and \"%s\"",
                    scriptName(origPos)->fileName, iname);
                errFail();
            }
            /* Right rotate the queued names between numScripts and i so
             * that i ends up with other imports in front of orig. script.
             * Only the names move: the marks of the script being read are
             * recorded at numScripts and stay there for the drop below.
             */
            tmpScript = *scriptName(i);
            for (; i > numScripts; i--)
                *scriptName(i) = *scriptName(i - 1);
            *scriptName(numScripts) = tmpScript;
            origPos++;
        }
    }
    return needsImports;
}

/* --------------------------------------------------------------------------
 * Adding scripts found in an argument vector:
 * ------------------------------------------------------------------------*/
void addScriptsFromArgs(int argc, char *argv[]) {
    int i;

    for (i = 1; i < argc; ++i) {
        if (argv[i] && argv[i][0] && !isOption(argv[i])) {
            addScriptName(argv[i], true);
        }
    }
}

/* --------------------------------------------------------------------------
 * Dropping script files:
 * ------------------------------------------------------------------------*/
/* Remove scripts from system */
void forgetScriptsFrom(Script scno) {
    Script i;
    for (i = scno; i < namesUpto; ++i)
        freeScript(i);
    dropScriptsFrom(scno);
    namesUpto = scno;
    if (numScripts > namesUpto)
        numScripts = scno;
}

void forgetAllScripts(void) {
    /* Drop all but the stable scripts; i.e., the
   * Prelude and (possibly) its implementation module(s).
   */
    forgetScriptsFrom(scriptsStable);
}

/* Remove a script from system */
void forgetAScript(Script scno) {
    Script i;

    if (scno > namesUpto)
        return;

    freeScript(scno);

    for (i = scno + 1; i < namesUpto; i++)
        *scriptName(i - 1) = *scriptName(i);
    dropAScript(scno);
    namesUpto--;
}

/* Reread current list of scripts, loading everything after and including the
 * first script which has been either changed or added */
void readScripts(int n) {
    Time timeStamp;
    long fileSize;

    for (; n < numScripts; n++) { /* Scan previously loaded scripts  */
        getFileInfo(scriptName(n)->fileName, &timeStamp, &fileSize);
        if (timeChanged(timeStamp, scriptName(n)->lastChange)) {
            dropScriptsFrom(n);
            numScripts = n;
            break;
        }
    }
    for (; n < NUM_SCRIPTS; n++)          /* No scripts have been postponed  */
        scriptName(n)->postponed = false; /* at this stage                   */

    while (numScripts < namesUpto) { /* Process any remaining scripts   */
        getFileInfo(scriptName(numScripts)->fileName, &timeStamp, &fileSize);
        scriptName(numScripts)->lastChange = timeStamp;
        startNewScript(scriptName(numScripts)->fileName);
        generate_ffi = generateFFI && !scriptName(numScripts)->chased;
        if (addScript(scriptName(numScripts)->fileName, fileSize))
            numScripts++;
        else
            dropScriptsFrom(numScripts);
    }

    if (listScripts)
        whatScripts();
    if (numScripts <= 1)
        setLastEdit((char *)0, 0);
}

/* List scripts in current session */
void whatScripts(void) {
    int i;
    Printf("\nCuddles session for:");
    for (i = 0; i < numScripts; ++i) {
        Printf("\n%s", scriptName(i)->fileName);
    }
    Putchar('\n');
}

/* Discard every loaded script. */
static void exitScript(void) {
    stopScripts();
}

void script(int what) {

    /* not much to it..will be more when/if the local tables
   * become resizable.
   */
    switch (what) {
    case EXIT:
        exitScript();
        break;
    default:
        break;
    }
}
