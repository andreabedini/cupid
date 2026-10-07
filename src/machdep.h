/* --------------------------------------------------------------------------
 * Machine dependent code
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * ------------------------------------------------------------------------*/
#ifndef __MACHDEP_H__
#define __MACHDEP_H__
#include "prelude.h"

#include <time.h>

#if USE_REGISTRY
# if HAVE_WINDOWS_H
#  include <windows.h>
# endif
#endif

/* --------------------------------------------------------------------------
 * Find information about a file:
 * ------------------------------------------------------------------------*/

typedef time_t Time;
CUDDLES_inline bool timeChanged(Time now, Time thn) {
    return now != thn;
}

extern void getFileInfo(char *, Time *, long *);

extern void installMachdep(void); /* register roots, install signal/term  */
extern void resetMachdep(void);   /* clear the pending breakpoint buffer  */
extern void exitMachdep(void);    /* restore the terminal's normal mode   */

/* --------------------------------------------------------------------------
 * Prototypes for registry reading
 * ------------------------------------------------------------------------*/

#if USE_REGISTRY

extern char *readRegString(HKEY, char *, char *, char *);
extern bool writeRegString(char *, char *);
extern char *readRegChildStrings(HKEY, char *, char *, char *);
#endif /* USE_REGISTRY */

/* --------------------------------------------------------------------------
 * Search for script files on the CUDDLES path:
 * ------------------------------------------------------------------------*/

extern char *RealPath(char *);
extern char *substPath(char *, char *);
extern char *uniqPath(char *);
extern bool startEdit(int, char *); /* edit.c, linked into cuddles only */

extern int shellEsc(char *);

/* --------------------------------------------------------------------------
 * Running an external program without a shell:
 *
 * Build the command as an argv - argvWord for one argument taken verbatim
 * (a file name), argvWords for a configured string split on whitespace -
 * and run it with argvRun, or with argvOpen/argvClose to read what it
 * writes on its standard output.  Nothing is quoted because no shell sees
 * it.  argvRun and argvClose return the program's exit status, or -1 if it
 * could not be run or did not exit normally.
 * ------------------------------------------------------------------------*/

#if !IS_WINDOWS
typedef pid_t ProcId;
#else
typedef intptr_t ProcId;
#endif

typedef struct {
    char **vec;  /* the argv itself, NULL-terminated                 */
    size_t used; /* words in it, not counting the terminating NULL   */
    size_t size; /* slots allocated                                  */
} Argv;

extern void argvInit(Argv *);
extern void argvWord(Argv *, char *);
extern void argvWords(Argv *, char *);
extern void argvFree(Argv *);
extern int argvRun(Argv *, bool);
extern FILE *argvOpen(Argv *, ProcId *);
extern int argvClose(FILE *, ProcId);
extern int getTerminalWidth(void);
extern void normalTerminal(void);
extern bool getEchoTerminal(int);
extern void setEchoTerminal(int, bool);
extern bool getBuffTerminal(int);
extern void setBuffTerminal(int, bool);
extern void gcStarted(void);
extern void gcScanning(void);
extern void gcRecovered(int);
extern void gcCStack(void);
extern void needPrims(int, void *);

extern char *fromEnv(char *, char *);
extern bool initSystem(void);
extern void installSignalHandlers(void);

extern char *dirname(char *);

#endif /* __MACHDEP_H__ */
