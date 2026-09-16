/*
 * Keeping track of progress towards a goal.
 */
#ifndef __GOAL_H__
#define __GOAL_H__

#include "prelude.h"

typedef long Target;
extern void setGoal(char *, Target);
extern void soFar(Target);
extern void done(void);
extern void failed(void);

#endif /* __GOAL_H__ */
