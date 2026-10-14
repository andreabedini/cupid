/* --------------------------------------------------------------------------
 * Infix resolution (fixity.c), as the static analysis sees it.
 * ------------------------------------------------------------------------*/
#ifndef __FIXITY_H__
#define __FIXITY_H__

#include "prelude.h"
#include "storage.h"

extern Cell tidyInfix(int, Cell);   /* turn an INFIX cell into a tree       */
extern Cell tidySection(int, Cell); /* turn a SECTION cell into a tree     */

#endif /* __FIXITY_H__ */
