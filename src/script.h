/*
 * Maintaining a stack of files / scripts.
 * 
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 */
#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "prelude.h"
#include "storage.h"

/*
 * The Script module handles the management of the stack of scripts
 * known to the interpreter. 
 *
 * When the interpreter processes a module, it adds the module's
 * imports to the script stack which are then subsequently loaded in.
 * Assuming the imported modules were all found and loaded without error,
 * the original module and its imports is then processed again, 
 * but this time all the imports are found on the stack of scripts
 * already loaded in.
 *
 */

extern void initScripts(void);
extern void stopScripts(void);
extern void setScriptStableMark(void);

extern char *getScriptName(Script);
extern char *getScriptRealName(Script);
extern int getScriptHwMark(void);
extern int numLoadedScripts(void);

extern void addScriptName(char *, bool);
extern bool chase(List);
extern void addScriptsFromArgs(int, char *[]);

extern void forgetScriptsFrom(Script);
extern void forgetAllScripts(void);
extern void forgetAScript(Script);

extern char *scriptFile;

extern void whatScripts(void);
extern void readScripts(int);

extern void exitScript(void); /* discard every loaded script */

#endif /* __SCRIPT_H__ */
