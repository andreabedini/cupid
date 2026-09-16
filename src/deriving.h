/* --------------------------------------------------------------------------
 * Derived instances (deriving.c), as the static analysis sees them.
 * ------------------------------------------------------------------------*/
#ifndef __DERIVING_H__
#define __DERIVING_H__

#include "prelude.h"
#include "storage.h"

extern void checkDerive(Tycon, List, List, Cell); /* record a request       */
extern List derivedInstances(void); /* implement them and return the list    */
extern void resetDeriving(void);    /* clear the derived-instance list       */
extern void installDeriving(void);  /* register roots and reset; INSTALL only*/

#endif /* __DERIVING_H__ */
