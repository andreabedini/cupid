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
CUDDLES_inline Bool timeChanged(Time now, Time thn) {
    return now != thn;
}

extern Void getFileInfo(String, Time *, Long *);

/* --------------------------------------------------------------------------
 * Prototypes for registry reading
 * ------------------------------------------------------------------------*/

#if USE_REGISTRY

extern String readRegString(HKEY, String, String, String);
extern Bool writeRegString(String, String);
extern String readRegChildStrings(HKEY, String, String, String);
#endif /* USE_REGISTRY */

/* --------------------------------------------------------------------------
 * Search for script files on the CUDDLES path:
 * ------------------------------------------------------------------------*/

extern String RealPath(String);
extern String substPath(String, String);
extern String uniqPath(String);
extern Bool startEdit(Int, String); /* edit.c, linked into cuddles only */

extern Int shellEsc(String);

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
    String *vec; /* the argv itself, NULL-terminated                 */
    size_t used; /* words in it, not counting the terminating NULL   */
    size_t size; /* slots allocated                                  */
} Argv;

extern Void argvInit(Argv *);
extern Void argvWord(Argv *, String);
extern Void argvWords(Argv *, String);
extern Void argvFree(Argv *);
extern Int argvRun(Argv *, Bool);
extern FILE *argvOpen(Argv *, ProcId *);
extern Int argvClose(FILE *, ProcId);
extern Int getTerminalWidth(Void);
extern Void normalTerminal(Void);
extern Bool getEchoTerminal(Int);
extern Void setEchoTerminal(Int, Bool);
extern Bool getBuffTerminal(Int);
extern Void setBuffTerminal(Int, Bool);
extern Void gcStarted(Void);
extern Void gcScanning(Void);
extern Void gcRecovered(Int);
extern Void gcCStack(Void);
extern Void needPrims(Int, void *);

extern String fromEnv(String, String);
extern Bool initSystem(Void);

extern String dirname(String);

#endif /* __MACHDEP_H__ */
