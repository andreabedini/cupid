/* --------------------------------------------------------------------------
 * Shape checks for types, kinds and predicates; see tagcheck.h.
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "errors.h"
#include "tagcheck.h"

Bool isKindExpr(Cell k) {
    switch (whatIs(k)) {
    case STAR:
    case OFFSET:
    case INTCELL:
        return TRUE;
    case AP:
        return isKindExpr(fst(k)) && isKindExpr(snd(k));
    default:
        return FALSE;
    }
}

Bool isKindsSig(Cell ks) /* k1 -> ... -> kn -> *, as generalize */
{                        /* builds it; NIL entries and a NIL end  */
    for (; isAp(ks);
         ks = snd(ks)) /* are the shape before fixKinds, and         */
        if (nonNull(fst(ks)) && !isKindExpr(fst(ks))) /* a class's kinds  */
            return FALSE;
    return ks == STAR || isNull(ks);
}

Bool isTypeExpr(Cell t) {
    switch (whatIs(t)) {
    case TYCON:
    case TUPLE:
    case OFFSET:
    case INTCELL:
    case VARIDCELL:
    case VAROPCELL:
        return TRUE;
    case AP:
        return isTypeExpr(fst(t)) && isTypeExpr(snd(t));
    case POLYTYPE:
        return isKindsSig(polySigOf(t)) && isTypeExpr(monotypeOf(t));
    case QUAL:
    case CDICTS:
        return isPredList(fst(snd(t))) && isTypeExpr(snd(snd(t)));
    case RANK2:
    case EXIST:
        return isInt(fst(snd(t))) && isTypeExpr(snd(snd(t)));
    default:
        return FALSE;
    }
}

Bool isPredExpr(Cell pi) {
    for (; isAp(pi); pi = fst(pi))
        if (!isTypeExpr(snd(pi)))
            return FALSE;
    return isClass(pi);
}

Bool isPredList(Cell ps) {
    for (; isPair(ps); ps = tl(ps))
        if (!isPredExpr(hd(ps)))
            return FALSE;
    return isNull(ps);
}

Void tagError(const char *what, const char *fn, Cell c) {
    static char msg[120];
    /* c is by definition not the handle the caller expected, so it may be
     * any value at all.  whatIs reads heapTopFst[c] for a negative cell and
     * whatCode[c] for the rest, neither of which is bounds-checked, so ask
     * for the tag only once c is known to be in range: a reporter that
     * faults on a bad cell reports nothing. */
    if (isGenPair(c) || (0 <= c && c < N_WHATCODE))
        snprintf(msg, sizeof(msg), "%s: not a %s (cell %d, tag %d)", fn, what,
                 (int)c, (int)whatIs(c));
    else
        snprintf(msg, sizeof(msg), "%s: not a %s (cell %d, out of range)", fn,
                 what, (int)c);
    internal(msg);
}

/*-------------------------------------------------------------------------*/
