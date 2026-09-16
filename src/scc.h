/* --------------------------------------------------------------------------
 * Strongly connected components of a dependency graph, used by the static
 * analyser and the type checker to order binding groups.
 * ------------------------------------------------------------------------*/

#ifndef SCC_H
#define SCC_H

#include "prelude.h"
#include "storage.h"

/* The graph is not materialised: the caller says how to read and write the
 * dependency field of a node.  Before sorting, the field of each node holds
 * the list of nodes it depends on; the sort uses the same field for its
 * bookkeeping and leaves an Int in it afterwards.
 */
typedef Cell (*SccDepends)(Cell);
typedef void (*SccSetDepends)(Cell, Cell);

/* Sort the nodes of bs, then those of cs (which may be NIL), into a list of
 * strongly connected components, each a list of nodes, in which every
 * component comes after the ones it depends on.
 */
extern List sccSort(List, List, SccDepends, SccSetDepends);
extern void installScc(void); /* register roots; INSTALL only */
extern void resetScc(void);   /* clear the sccs found so far  */

#endif /* SCC_H */
