/* --------------------------------------------------------------------------
 * This file provides a simple mechanism for measuring elapsed time on Unix
 * machines (more precisely, on any machine with an rusage() function).
 * A somewhat limited version for other systems is also included, believed
 * to be ANSI compatible, but not guaranteed ...
 *
 * It is included in the Cuddles distribution for the purpose of benchmarking
 * the Cuddles interpreter, comparing its performance across a variety of
 * different machines, and with other systems for similar languages.
 *
 * It is compiled in with the meson option -Dtimer=true, which defines
 * WANT_TIMER for the callers in cuddles.c and evaluator.c.
 *
 * It would be somewhat foolish to try to use the timings produced in this
 * way for anything other than the purpose described above.  In particular,
 * using timings to compare the performance of different versions of an
 * algorithm is likely to give very misleading results.  The current
 * implementation of Cuddles as an interpreter, without any significant
 * optimizations, means that there are much more significant overheads than
 * can be accounted for by small variations in Cuddles code.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/

#include "timer.h"

long userElapsed = 0, systElapsed = 0; /* microseconds, or clock ticks     */

#if HAVE_SYS_TIME_H && HAVE_SYS_RESOURCE_H
# include <sys/time.h>
# include <sys/resource.h>

void updateTimers(void) {
    static long lastUser = 0;
    static long lastSyst = 0;
    long curr;
    struct rusage ruse;

    getrusage(RUSAGE_SELF, &ruse);
    curr = ruse.ru_utime.tv_sec * 1000000L + ruse.ru_utime.tv_usec;
    userElapsed = curr - lastUser;
    lastUser = curr;
    curr = ruse.ru_stime.tv_sec * 1000000L + ruse.ru_stime.tv_usec;
    systElapsed = curr - lastSyst;
    lastSyst = curr;
}

long millisecs(long t) {
    return (t + 500) / 1000;
}

#else
# include <time.h>

void updateTimers(void) {
    static clock_t lastUser = 0;
    clock_t curr;

    curr = clock();
    userElapsed = (long)(curr - lastUser);
    lastUser = curr;
}

long millisecs(long t) {
    return (t * 1000) / CLOCKS_PER_SEC;
}
#endif

/*-------------------------------------------------------------------------*/
