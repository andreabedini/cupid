/*
 * Keeping track of progress towards a goal.
 */
#include "prelude.h"
#include "errors.h"
#include "machdep.h"
#include "opts.h"
#include "goal.h"

/* --------------------------------------------------------------------------
 * Display progress towards goal:
 * ------------------------------------------------------------------------*/
static Target currTarget;
static bool aiming = false;
static int currPos;
static int maxPos;
static int charCount;

/* Set goal for what to be t */
void setGoal(char *what, Target t) {
    if (quiet || showInstRes)
        return;
    currTarget = (t ? t : 1);
    aiming = true;
    if (useDots) {
        currPos = (int)strlen(what);
        maxPos = getTerminalWidth() - 1;
        Printf("%s", what);
    } else
        for (charCount = 0; *what; charCount++)
            Putchar(*what++);
    FlushStdout();
}

/* Indicate progress towards goal has now reached t */
void soFar(Target t) {
    if (quiet || showInstRes)
        return;
    if (useDots) {
        int newPos = (int)((maxPos * ((long)t)) / currTarget);

        if (newPos > maxPos)
            newPos = maxPos;

        if (newPos > currPos) {
            do
                Putchar('.');
            while (newPos > ++currPos);
            FlushStdout();
        }
        FlushStdout();
    }
}

/* Goal has now been achieved */
void done(void) {
    if (quiet || showInstRes)
        return;
    if (useDots) {
        while (maxPos > currPos++)
            Putchar('.');
        Putchar('\n');
    } else
        for (; charCount > 0; charCount--) {
            Putchar('\b');
            Putchar(' ');
            Putchar('\b');
        }
    aiming = false;
    FlushStdout();
}

/* Goal cannot be reached due to errors */
void failed(void) {
    if (aiming) {
        aiming = false;
        Putchar('\n');
        FlushStdout();
    }
}
