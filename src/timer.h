/* --------------------------------------------------------------------------
 * Elapsed-time measurement for benchmarking (timer.c), built only with
 * -Dtimer=true; callers test WANT_TIMER.
 * ------------------------------------------------------------------------*/

#ifndef TIMER_H
#define TIMER_H

extern long userElapsed, systElapsed; /* time used since the previous      */
                                      /* updateTimers()            */
extern void updateTimers(void);
extern long millisecs(long); /* convert either to milliseconds  */

#endif /* TIMER_H */
