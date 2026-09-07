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

static Int numScripts;    /* Number of scripts loaded        */
static Int namesUpto;     /* Number of script names set      */
static Int scriptsStable; /* Number of (Prelude) scripts     */
                          /* considered 'stable'             */
                          /* (=> won't be nuked when clearing */
                          /* the script stack / reloading.)   */

static Bool needsImports; /* set to TRUE if imports required */
String scriptFile;        /* Name of current script (if any) */

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/
static Bool local addScript(String, Long);
static Void local freeScript(Int);

/* --------------------------------------------------------------------------
 * Initialising / freeing script stacks:
 * ------------------------------------------------------------------------*/
Void initScripts() {
    scriptFile = 0;
    numScripts = 0;
    namesUpto = 0;
    scriptsStable = 0;
}

Void stopScripts() {
    int i;

    for (i = 0; i < numScripts; i++)
        freeScript(i);
}

static Void local freeScript(Int i) {
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
Void setScriptStableMark() {
    scriptsStable = namesUpto;
}

String getScriptName(Script s) /* access the script name at index 's' */
{
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

String getScriptRealName(Script s) /* access the path of script at index 's' */
{
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

Int getScriptHwMark() { /* return number of on the stack, loaded or not. */
    return namesUpto;
}

Int numLoadedScripts() { /* return number of currently loaded scripts */
    return numScripts;
}

/* --------------------------------------------------------------------------
 * Loading script files:
 * ------------------------------------------------------------------------*/

Void addScriptName(String s, Bool sch) /* Add script to list of scripts   */
                                       /* to be read in ...               */
{                                      /* TRUE => requires pathname search*/
    if (namesUpto >= NUM_SCRIPTS) {
        errHead(0);
        errPrintf("Too many module files (maximum of %d allowed)", NUM_SCRIPTS);
        errFail();
        return;
    }
    if (sch) {
        if (isModuleId(s)) {
            String location = findMPathname(s);
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

static Bool local addScript(String fname,
                            Long len) /* read single script file */
                                      /* name of script file     */
{                                     /* length of script file   */

    if (!quiet) {
        Printf("Reading file \"%s\":\n", fname);
        FlushStdout();
    }
    setLastEdit(fname, 0);

    needsImports = FALSE;
    scriptFile = 0;
    if (!parseScript(fname, len)) { /* process script file */
        /* file or parse error, drop the script */
        forgetAScript(numScripts);
        errFail();
    }
    if (needsImports)
        return FALSE;
    checkDefns();
    typeCheckDefns();
    compileDefns();
    scriptFile = 0;
    preludeLoaded = TRUE;
    return TRUE;
}

Bool chase(List imps) /* Process list of import requests */
{
    Int origPos = numScripts; /* keep track of original position */
    String origDir = scriptName(origPos)->directory;
    for (; nonNull(imps); imps = tl(imps)) {
        String modname = textToStr(textOf(hd(imps)));
        String iname = NULL;
        String rname;
        Bool inOrigDir = FALSE;
        Int i = 0;

        if (origDir) {
            iname = findMInDir(origDir, modname);
            if (iname)
                inOrigDir = TRUE;
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

            scriptName(origPos)->postponed = TRUE;
            needsImports = TRUE;

            if (i >= namesUpto) { /* Name not found (i==namesUpto)   */
                addScriptName(iname, FALSE);
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
Void addScriptsFromArgs(Int argc, String argv[]) {
    Int i;

    for (i = 1; i < argc; ++i) {
        if (argv[i] && argv[i][0] && !isOption(argv[i])) {
            addScriptName(argv[i], TRUE);
        }
    }
}

/* --------------------------------------------------------------------------
 * Dropping script files:
 * ------------------------------------------------------------------------*/
Void forgetScriptsFrom(Script scno) /* remove scripts from system     */
{
    Script i;
    for (i = scno; i < namesUpto; ++i)
        freeScript(i);
    dropScriptsFrom(scno);
    namesUpto = scno;
    if (numScripts > namesUpto)
        numScripts = scno;
}

Void forgetAllScripts() {
    /* Drop all but the stable scripts; i.e., the
   * Prelude and (possibly) its implementation module(s).
   */
    forgetScriptsFrom(scriptsStable);
}

Void forgetAScript(Script scno) /* remove a script from system */
{
    Script i;

    if (scno > namesUpto)
        return;

    freeScript(scno);

    for (i = scno + 1; i < namesUpto; i++)
        *scriptName(i - 1) = *scriptName(i);
    dropAScript(scno);
    namesUpto--;
}

Void readScripts(Int n) /* Reread current list of scripts, */
{                       /* loading everything after and    */
    Time timeStamp;     /* including the first script which*/
    Long fileSize;      /* has been either changed or added*/

    for (; n < numScripts; n++) { /* Scan previously loaded scripts  */
        getFileInfo(scriptName(n)->fileName, &timeStamp, &fileSize);
        if (timeChanged(timeStamp, scriptName(n)->lastChange)) {
            dropScriptsFrom(n);
            numScripts = n;
            break;
        }
    }
    for (; n < NUM_SCRIPTS; n++)          /* No scripts have been postponed  */
        scriptName(n)->postponed = FALSE; /* at this stage                   */

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
        setLastEdit((String)0, 0);
}

Void whatScripts() { /* list scripts in current session */
    int i;
    Printf("\nCuddles session for:");
    for (i = 0; i < numScripts; ++i) {
        Printf("\n%s", scriptName(i)->fileName);
    }
    Putchar('\n');
}

Void script(Int what) {

    /* not much to it..will be more when/if the local tables
   * become resizable.
   */
    switch (what) {
    case RESET:
        return;
    case MARK:
        return;
    case INSTALL:
        return;
    case EXIT:
        stopScripts();
        return;
    case BREAK:
        return;
    }
}
