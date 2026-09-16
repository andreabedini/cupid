/* --------------------------------------------------------------------------
 * Basic data type definitions, prototypes and standard macros including
 * machine dependent variations...
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/
#ifndef __PRELUDE_H__
#define __PRELUDE_H__

/* Cuddles is compiled as standard C23.  Strict mode hides the POSIX and BSD
 * interfaces we use (fdopen, fileno, realpath, tm_gmtoff, ...) unless a
 * feature macro asks for them, and it must be defined before the first
 * system header. */
#if !defined(_WIN32)
# ifndef _DEFAULT_SOURCE
#  define _DEFAULT_SOURCE 1
# endif
# ifndef _DARWIN_C_SOURCE
#  define _DARWIN_C_SOURCE 1
# endif
#endif

#include "platform.h" /* IWYU pragma: keep -- SIZEOF_* in the static assertions below */
#include "sysdeps.h"
#include "config.h"
#include <stdio.h>

#include <stdlib.h> /* IWYU pragma: keep -- the common preamble; farCalloc, and every caller of malloc */
#include <sys/types.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

/* platform.h states the sizes as constants; make sure the compiler agrees */
_Static_assert(sizeof(int) == SIZEOF_INT, "platform.h: SIZEOF_INT");
_Static_assert(sizeof(float) == SIZEOF_FLOAT, "platform.h: SIZEOF_FLOAT");
_Static_assert(sizeof(double) == SIZEOF_DOUBLE, "platform.h: SIZEOF_DOUBLE");
_Static_assert(sizeof(void *) == SIZEOF_INTP, "platform.h: SIZEOF_INTP");

/*---------------------------------------------------------------------------
 * Most of the configuration code from earlier versions of Cuddles has been moved
 * into config.h (which is usually automatically generated).  
 *
 * Most of the configuration code is "feature based".  That is, the 
 * configure script looks to see if a particular feature (or misfeature)
 * is present on the compiler/OS.  
 *
 * A small amount of configuration code is still "system based": it tests
 * flags to determine what kind of compiler/system it's running on - from
 * which it infers what features the compiler/system has.  Use of system
 * based tests generally indicates that we can't remember/figure out
 * what the original problem was and so we can't add an appropriate feature
 * test to the configure script.
 *-------------------------------------------------------------------------*/

#if defined(_WIN32)
# define IS_WINDOWS 1
#else
# define IS_WINDOWS 0
#endif

/* using a (possibly multi-byte) encoding of Chars in Strings and I/O? */
#if CHAR_ENCODING_LOCALE || CHAR_ENCODING_UTF8
# define CHAR_ENCODING 1
#endif

#if defined(_WIN32)
# define STDCALL_SUPPORTED 1
#endif

/*---------------------------------------------------------------------------
 * Configuration options
 *
 * Most configuration options are arguments to the configure script
 * (try running "configure --help").  The following options are either
 * experimental or require changes to "Prelude.hs", the standard libraries
 * and demos and therefore cannot be modified using the configure script.
 * Most users should leave them alone!
 *
 *   HASKELL_ARRAYS to include support for Haskell array primitives.
 *   IO_MONAD       to include the IO monad primitives and support.
 *   IO_HANDLES     to include the IO file operations.
 *   IO_REFS        Ref type for IO_MONAD, and simple operations.
 *   FLUSHEVERY     to force a fflush after every char in putStr/hPutStr.
 *   BIGNUMS        to include support for Integer bignums.
 *   GC_STABLEPTRS  to include support for safely passing Haskell
 *                  pointers over to C
 *                  (only required if you use callbacks in the foreign
 *                  language interface)
 *   GC_MALLOCPTRS  to include support for automatic deallocation of 
 *                  C objects when Haskell is done with them.
 *   GC_WEAKPTRS    to include support for weak pointers.
 *   STABLE_NAMES   stable names a la Simon PJ
 *   REDIRECT_OUTPUT ability to redirect stdout/stderr to a buffer.
 *                  runcuddles captures the interpreter's own output this
 *                  way, and prints it only if something goes wrong.
 *   WORD_OPS       to include operations on unsigned ints
 *   ADDR_OPS       to include operations on addresses
 *   PROVIDE_INT64  to include 64 bit Ints
 *   SHORT_CIRCUIT_COERCIONS to try to apply these rewrites at runtime:
 *                    integerToInt (intToInteger x) -> x
 *                    rationalToFloat  (fromDouble {dict} x) -> doubleToFloat x
 *                    rationalToDouble (fromDouble {dict} x) -> x
 *   FAST_WHATIS    to use a macro instead of a func for whatIs(), for speed
 *   FAST_WHATIS1   to use whatIs1(), a faster version of whatIs(), for speed
 *-------------------------------------------------------------------------*/

#define HASKELL_ARRAYS 1
#define IO_MONAD 1
#define IO_HANDLES 1
#define IO_REFS 1 /* Experimental IO Ref type                      */
#define FLUSHEVERY 1
#define BIGNUMS 1       /* Experimental bignum implementation           */
#define GC_STABLEPTRS 1 /* May be required by external libraries        */
#define GC_MALLOCPTRS 1 /* May be required by external libraries        */
#define GC_WEAKPTRS 1
#define STABLE_NAMES 1
#define REDIRECT_OUTPUT 1
#define WORD_OPS 1
#define ADDR_OPS 1
#define PROVIDE_INT64 1
/* (as opposed to dynamically growable ones)  */

#define UNICODE_CHARS 1 /* Char is Unicode (ISO-10646) */

#define SHORT_CIRCUIT_COERCIONS 1

#define FAST_WHATIS 1
#define FAST_WHATIS1 1 /* can only use if FAST_WHATIS is 1 */

/*---------------------------------------------------------------------------
 * Platform-dependent settings:
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Macros used in declarations:
 *  local declarations
 *  CUDDLES_noreturn/CUDDLES_unused (prevent spurious warnings)
 *  result type of main
 *  dynamic linking declarations
 *-------------------------------------------------------------------------*/

/* CUDDLES_inline marks the small functions in the headers that replace the
 * old accessor macros.  They must inline even in the files built without
 * optimisation (see src/meson.build), so the attribute is not optional. */
#if defined(__GNUC__) || defined(__clang__)
# define CUDDLES_inline static inline __attribute__((always_inline))
#elif defined(_MSC_VER)
# define CUDDLES_inline static __forceinline
#else
# define CUDDLES_inline static inline
#endif

/* CUDDLES_printf(f,a): the function takes a printf format as parameter f
 * and its arguments from parameter a on, so the compiler checks them */
#if defined(__GNUC__) || defined(__clang__)
# define CUDDLES_printf(f, a) __attribute__((format(printf, f, a)))
#else
# define CUDDLES_printf(f, a)
#endif

#ifdef __GNUC__ /* Avoid spurious warnings                             */
# if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 7
#  define CUDDLES_noreturn __attribute__((noreturn))
#  define CUDDLES_unused __attribute__((unused))
# else
#  define CUDDLES_noreturn
#  define CUDDLES_unused
# endif
#else
# define CUDDLES_noreturn
# define CUDDLES_unused
#endif

/*---------------------------------------------------------------------------
 * Dynamic linking tricks
 *-------------------------------------------------------------------------*/

#if defined(_WIN32) /* Microsoft Windows */
# define DLLIMPORT(rty) __declspec(dllimport) rty
# define DLLEXPORT(rty) __declspec(dllexport) rty
#else
# define DLLIMPORT(rty) rty
# define DLLEXPORT(rty) rty
#endif /* Don't need to declare DLL exports */

/*---------------------------------------------------------------------------
 * String operations:
 *-------------------------------------------------------------------------*/

#include <string.h>
#define strCompare strcmp

#include <ctype.h>
#ifndef isascii
# define isascii(c) (((unsigned)(c)) < 128)
#endif

/*---------------------------------------------------------------------------
 * Printf-related operations:
 *-------------------------------------------------------------------------*/

#include <stdarg.h>

/*---------------------------------------------------------------------------
 * Pipe-related operations:
 *
 * On Windows, many standard Unix names acquire a leading underscore.
 * Irritating, but easy to work around.
 *-------------------------------------------------------------------------*/

#if !HAVE_POPEN && HAVE__POPEN
# define popen(x, y) _popen(x, y)
#endif
#if !HAVE_PCLOSE && HAVE__PCLOSE
# define pclose(x) _pclose(x)
#endif

/*---------------------------------------------------------------------------
 * Interrupting execution (signals, allowBreak):
 *-------------------------------------------------------------------------*/

#define sigProto(nm) void nm(int)
#define sigRaise(nm) nm(1)
#define sigHandler(nm) void nm(int sig_arg)
#define sigResume return

/* allowBreak: call to allow user to interrupt computation
 * ctrlbrk:    set control break handler
 */

/* The interrupt handler must not do unsafe things (longjmp, stdio, the
 * allocator) from signal context.  On every platform we therefore only
 * set a flag (the global variable "broken") from the handler to request
 * an interrupt; all potentially infinite loops of the evaluator check
 * the flag using the "allowBreak" call, which raises breakHandler() as
 * an ordinary (safe) function call once we are back in normal control
 * flow.  This used to be Windows-only (the interrupt handler runs in a
 * separate thread there, so it cannot longjmp out of the computation);
 * POSIX now uses the same scheme so that the SIGINT handler itself stays
 * async-signal-safe.
 */

#if IS_WINDOWS

# define ctrlbrk(bh)                                                           \
     do {                                                                      \
         signal(SIGINT, bh);                                                   \
         signal(SIGBREAK, bh);                                                 \
     } while (0)

#else /* Unix */

# include <signal.h>
/* SA_RESTART is deliberately left off: a blocking read() at the
    * prompt must return EINTR on Ctrl-C rather than silently resume,
    * so that the prompt can be redrawn instead of hanging.
    */
# define ctrlbrk(bh)                                                           \
     do {                                                                      \
         struct sigaction cuddles_sigbrk_sa;                                   \
         cuddles_sigbrk_sa.sa_handler = (bh);                                  \
         sigemptyset(&cuddles_sigbrk_sa.sa_mask);                              \
         cuddles_sigbrk_sa.sa_flags = 0;                                       \
         sigaction(SIGINT, &cuddles_sigbrk_sa, NULL);                          \
     } while (0)

#endif

#ifndef SIGBREAK /* Sigh, not defined in cygwin32 beta release 16 */
# define SIGBREAK 21
#endif

/*---------------------------------------------------------------------------
 * Floating point support
 *-------------------------------------------------------------------------*/

/* Can we fit floats into ints? */

#define FloatImpType float
#define FloatPro double /* type to use in prototypes               */
                        /* strictly ansi (i.e. gcc) conforming  */
                        /* but breaks data hiding :-(      */
#define FloatFMT "%.7g"

#define DoubleImpType double
#define DoublePro double
#define DoubleFMT "%.15g"

#include <float.h>

#define CUDDLES_FLT_RADIX FLT_RADIX
#define CUDDLES_FLT_MANT_DIG FLT_MANT_DIG
#define CUDDLES_FLT_MIN_EXP FLT_MIN_EXP
#define CUDDLES_FLT_MAX_EXP FLT_MAX_EXP

#define CUDDLES_DBL_MANT_DIG DBL_MANT_DIG
#define CUDDLES_DBL_MIN_EXP DBL_MIN_EXP
#define CUDDLES_DBL_MAX_EXP DBL_MAX_EXP

/*---------------------------------------------------------------------------
 * Memory allocation
 *-------------------------------------------------------------------------*/

/* Zeroed, as the name says.  This was valloc when HAVE_VALLOC, which does
 * not zero: heapFst and heapSnd got away with it because storage(INSTALL)
 * writes every cell onto the free list, but the profiler's heapThd has no
 * pass of its own and every cell began with a garbage producer.  The page
 * alignment bought nothing -- an allocation this size is mmap'd, and so
 * page-aligned, either way. */
#define farCalloc(n, s) (void *)calloc((size_t)(n), (size_t)(s))

/* bison-generated parsers like to have alloca - so try to define it */

#if HAVE_ALLOCA_H
# include <alloca.h>
#else
# if HAVE__ALLOCA && !defined(__SYMBIAN32__)
#  include <malloc.h>
# endif
#endif

/*---------------------------------------------------------------------------
 * Assertions
 *-------------------------------------------------------------------------*/

#include <assert.h>

/*---------------------------------------------------------------------------
 * Preprocessor support
 *-------------------------------------------------------------------------*/

/*
 * Note: initially defined as
 *   #define SUPPORT_PREPROCESSOR USE_PREPROCESSOR && (defined(HAVE_POPEN) || defined(HAVE__POPEN))
 *
 * which worked OK with GNU cpp, but MSVC didn't expand the 'defined' correctly.
 *
 */
#if USE_PREPROCESSOR
# if HAVE_POPEN || HAVE__POPEN
#  define SUPPORT_PREPROCESSOR 1
# else
#  define SUPPORT_PREPROCESSOR 0
# endif
#else
# define SUPPORT_PREPROCESSOR 0
#endif

/*---------------------------------------------------------------------------
 * Environment variables and the registry
 *-------------------------------------------------------------------------*/

/* On Win32 we can use the registry to supplement info in environment 
 * variables.
 */
#define USE_REGISTRY (HAVE_WINDOWS_H && !__MSDOS__)

/*---------------------------------------------------------------------------
 * File operations:
 *-------------------------------------------------------------------------*/

#if !HAVE_UNISTD_H && !_MSC_VER
extern int chdir(const char *);
#endif

#if HAVE_DIRECT_H
# include <direct.h>
#endif

#ifndef FILENAME_MAX /* should already be defined in an ANSI compiler*/
# define FILENAME_MAX 256
#elif FILENAME_MAX < 256
# undef FILENAME_MAX
# define FILENAME_MAX 256
#elif FILENAME_MAX > 8192
/* Systems with no limit on path length (e.g. the Hurd), will have */
/* FILENAME_MAX impossibly large.  Ideally we should dynamically   */
/* allocate/grow these buffers and not use FILENAME_MAX at all.    */
# undef FILENAME_MAX
# define FILENAME_MAX 8192
#endif

/* DOS_FILENAMES is a system fact set by sysdeps.h from _WIN32. */
/* ToDo: can we replace this with a feature test? */
#define CASE_INSENSITIVE_FILENAMES DOS_FILENAMES

#if CASE_INSENSITIVE_FILENAMES
# if HAVE_STRCASECMP
#  define filenamecmp(s1, s2) strcasecmp(s1, s2)
# elif HAVE__STRICMP
#  define filenamecmp(s1, s2) _stricmp(s1, s2)
# elif HAVE_STRICMP
#  define filenamecmp(s1, s2) stricmp(s1, s2)
# elif HAVE_STRCMPI
#  define filenamecmp(s1, s2) strcmpi(s1, s2)
# endif
#else
# define filenamecmp(s1, s2) strcmp(s1, s2)
#endif

/*---------------------------------------------------------------------------
 * Optimisations:
 *-------------------------------------------------------------------------*/

#ifdef __GNUC__                      /* look for GCC 2.x extensions         */
# if __GNUC__ >= 2 && !defined(NeXT) /* NeXT cc lies and says it's 2.x  */

/* WARNING: if you use the following optimisations to assign registers for
 * particular global variables, you should be very careful to make sure that
 * storage(RESET) is called after a longjump (usually resulting from an error
 * condition) and before you try to access the heap.  The current version of
 * main deals with this using everybody(RESET) at the head of the main read,
 * eval, print loop
 */

# endif
#endif /* defined(__GNUC__) */

/*---------------------------------------------------------------------------
 * General settings:
 *-------------------------------------------------------------------------*/

#include <stdint.h>

typedef int Char; /* a Unicode code point, or EOF */

#define doNothing()                                                            \
    do {                                                                       \
    } while (0) /* Null statement */

#ifndef STD_PRELUDE
# define STD_PRELUDE "Prelude"
# define STD_PRELUDE_CUDDLES "Cuddles.Prelude"
# define STD_EMPTY_MODULE "Cuddles"
#endif

/* The table and heap sizes, which are a group of their own */
#include "sizes.h" /* IWYU pragma: keep -- the common preamble; thirteen files take the table sizes from here */

#if PROFILING
# define DEF_PROFINTDIV 10 /* hpsize/this cells between samples*/
#endif

/* Representation of Int */
#define MAXPOSINT 0x7fffffff
#define MINNEGINT (-MAXPOSINT - 1)
#define MAXCUDDLESWORD 0xffffffffU

/* Representation of Integer: requires BIGBASE == 10^BIGEXP */
#define BIGBASE 10000
#define BIGEXP 4

#define DECTABLE(tab) tab[]
#define DEFTABLE(tab, sz) tab[sz]

#define minRecovery 1000
#define bitsPerWord 32
#define wordShift 5
#define wordMask 31

CUDDLES_inline int bitArraySize(int n) {
    return n / bitsPerWord + 1;
}
/* N is a (negative) pair cell */
CUDDLES_inline int placeInSet(int n) {
    return (-n - 1) >> wordShift;
}
/* Unsigned: the shift can reach bit 31 */
CUDDLES_inline unsigned maskInSet(int n) {
    return 1u << ((-n - 1) & wordMask);
}

/*-------------------------------------------------------------------------*/

#endif /* __PRELUDE_H__ */
