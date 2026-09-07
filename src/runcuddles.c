/* --------------------------------------------------------------------------
 * Standalone cuddles system
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: runcuddles.c,v $
 * $Revision: 1.23 $
 * $Date: 2006/04/26 12:26:44 $
 * ------------------------------------------------------------------------*/

#define CUDDLES_SERVER 1

#include "prelude.h"
#include <stdio.h>
#include <stdlib.h>
#include "storage.h"
#include "connect.h"
#include "machdep.h"
#include "server.h"

#if defined(_MSC_VER) && !defined(_MANAGED)
#include <windows.h>
#endif

extern int  main      (int, char**);
static void check     (void);
static void loadCuddles  (int, char**);

static CuddlesServerAPI* cuddles = 0;

static void check() {
    char* err = cuddles->clearError();
    if (err) {
	fprintf(stderr,"runcuddles: %s\n",err);
	fflush(stderr);
	exit(1);
    }
}

static void loadCuddles(int argc, char* argv[])
{
    cuddles = initCuddlesServer(argc,argv);
    if (NULL == cuddles) {
	fprintf(stderr,"runcuddles: Unable to initialise Cuddles (%s)\n", lastError);
	fflush(stderr);
	exit(1);
    }
    cuddles->setOutputEnable(0);
    check();
}

/* --------------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    int    exitCode = 0;
    char** cuddles_argv;
    int    cuddles_argc;
    char*  progname;

    progname = argv ? argv[0] : "runcuddles";
    if (!initSystem()) {
      fprintf(stderr,"%s: failed to initialize, exiting\n", progname);
      fflush(stderr);
      exit(1);
    }

#if defined(FFI_COMPILER)
    generateFFI = TRUE;
#endif

    /* skip over any option flags before the program name */
    {
	int i = 1; /* ignore first arg - name of this program */
	while (i < argc 
	       && argv[i] /* paranoia */
	       && (argv[i][0] == '+' || argv[i][0] == '-')
	       ) {
	    ++i;
	}
	cuddles_argv = argv;
	cuddles_argc = i;

	argv += i;
	argc -= i;
    }

    if (argc < 1) {
	fprintf(stderr,"%s: missing file argument\n",progname);
	fflush(stderr);
	exit(1);
    }

#if defined(_MSC_VER) && !defined(_MANAGED)
    __try {
#endif

    loadCuddles(cuddles_argc, cuddles_argv);

#if defined(FFI_COMPILER)
    /* all arguments following the module name are passed to the C compiler */
    {
	int i;
	for (i=1; i<argc; ++i)
	    ffiSetFlags(argv[i]);
    }

    cuddles->loadFile(argv[0]);
    check();
#else
    /* all arguments following the module name are available via getArgs */
    cuddles->loadFile(argv[0]);
    check();

    cuddles->setCuddlesArgs(argc,argv);
    cuddles->pushHVal(cuddles->compileExpr("Main","main >> return () :: IO ()"));
    exitCode = cuddles->doIO();
    check();
#endif

#if defined(_MSC_VER) && !defined(_MANAGED)
    } __except ( ((GetExceptionCode() == EXCEPTION_STACK_OVERFLOW) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) ) {
      fatal("C stack overflow");
    }
#endif

    shutdownCuddlesServer(cuddles);
    
    exit(exitCode);
    return 0;/*NOTUSED*/
}

#if WANT_TIMER
/* dummy definition: timers are only available in the interpreter */
void updateTimers (void);
void updateTimers() {}
#endif
