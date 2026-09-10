/* --------------------------------------------------------------------------
 * Value declarations as bindings (binding.c), as the rest of the static
 * analysis sees them.
 * ------------------------------------------------------------------------*/
#ifndef __BINDING_H__
#define __BINDING_H__

#include "prelude.h"
#include "storage.h"

extern List eqnsToBindings(List, List, List, List); /* equations to bindings */
extern List extractSigdecls(List);   /* the type signatures in a list         */
extern List extractBindings(List);   /* the untyped bindings in a list        */
extern List extractFixdecls(List);   /* the fixity declarations in a list     */
extern Cell findBinding(Text, List); /* the binding of a variable, if any   */
extern Void dupFixity(Int, Text);    /* error: two fixities for one name      */
extern Void missFixity(Int, Text);   /* error: a fixity with no binding       */

#endif /* __BINDING_H__ */
