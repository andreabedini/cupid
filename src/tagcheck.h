/* --------------------------------------------------------------------------
 * Shape checks for the cell trees the type checker passes around.
 *
 * Types, kinds and predicates share one representation, so the compiler
 * cannot tell them apart; their tags can.  A type is a tree over
 * constructors, tuples and variables; a kind is a tree over STAR,
 * CONSTRAINT and variables; a predicate is a class applied to types, or
 * EQUALITY applied to two of them.  The predicates
 * below say which a cell is, and the assert* macros check it at the entry
 * points where the three meet.  With -Dchecks (CHECK_TAGS) a failed
 * check is an internal error naming the function; otherwise the macros
 * cost nothing.
 * ------------------------------------------------------------------------*/

#ifndef TAGCHECK_H
#define TAGCHECK_H

#include "prelude.h"
#include "storage.h"

extern bool isTypeExpr(Cell); /* a type (skeleton), possibly       */
                              /* qualified, quantified or rank-2 */
extern bool isKindExpr(Cell); /* a kind (skeleton)                 */
extern bool isKindsSig(Cell); /* the kinds of a scheme's variables:*/
                              /* k1 -> .. -> kn -> *, or a list  */
extern bool isPredExpr(Cell); /* a class applied to types          */
extern bool isPredList(Cell); /* a list of those                   */

#if CHECK_TAGS
# define assertType(t)                                                         \
     do {                                                                      \
         if (!isTypeExpr(t))                                                   \
             tagError("type", __func__, (t));                                  \
     } while (0)
# define assertKind(k)                                                         \
     do {                                                                      \
         if (!isKindExpr(k))                                                   \
             tagError("kind", __func__, (k));                                  \
     } while (0)
# define assertKinds(ks)                                                       \
     do {                                                                      \
         if (!isKindsSig(ks))                                                  \
             tagError("kind signature", __func__, (ks));                       \
     } while (0)
# define assertPred(p)                                                         \
     do {                                                                      \
         if (!isPredExpr(p))                                                   \
             tagError("predicate", __func__, (p));                             \
     } while (0)
# define assertPredList(p)                                                     \
     do {                                                                      \
         if (!isPredList(p))                                                   \
             tagError("predicate list", __func__, (p));                        \
     } while (0)
#else
# define assertType(t) ((void)0)
# define assertKind(k) ((void)0)
# define assertKinds(ks) ((void)0)
# define assertPred(p) ((void)0)
# define assertPredList(p) ((void)0)
#endif

#endif /* TAGCHECK_H */
