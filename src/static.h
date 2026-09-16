/* --------------------------------------------------------------------------
 * The one thing the static analysis driver (static.c) lends its siblings.
 * The entry points are declared in connect.h.
 * ------------------------------------------------------------------------*/
#ifndef __STATIC_H__
#define __STATIC_H__

#include "prelude.h"
#include "storage.h"

extern void duplicateError(int, Module, Text, char *); /* two definitions */

extern void installStaticAnalysis(void); /* register roots, then reset */
extern void resetStaticAnalysis(void);   /* reset the six sub-modules  */

#endif /* __STATIC_H__ */
