/*
 * Running a file without the prompt: cupid with a file runs its main, and
 * cupid --ffi compiles the stubs for its foreign imports.  See src/run.c.
 */
#ifndef __RUN_H__
#define __RUN_H__

#include "prelude.h"

/* argv is cupid's command line: its own name, the options, the file and
 * what follows the file.  Load the Prelude and the file, then run main
 * with the file and what follows as getProgName and getArgs, or, if ffi
 * is true, generate and compile the C stubs for the file's foreign
 * imports, passing what follows to the C compiler.  The caller has taken
 * -i and --ffi out of argv.  Returns the exit code of main.  */
extern int runFile(int argc, char *argv[], bool ffi);

#endif /* __RUN_H__ */
