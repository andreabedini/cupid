/* --------------------------------------------------------------------------
 * How big the interpreter's tables and heap are.
 *
 * These are the tunable numbers, gathered in one place because most of
 * them are not independent: the sizes of the tuple, offset, module, tycon,
 * name, instance and class tables decide where each range of the Cell
 * integer space begins, so changing one shifts every range above it and
 * the size of the whatCode array with them.  storage.h builds that
 * partition (TUPMIN through INTMAX) and asserts the invariants it relies
 * on at the end of the file.
 *
 * This file is included by prelude.h and needs nothing from it.
 * ------------------------------------------------------------------------*/
#ifndef __SIZES_H__
#define __SIZES_H__

/* Tables that carve up the Cell integer space, in the order storage.h
 * lays them out.  storage.h asserts that they still tile it. */
#define NUM_TUPLES 100
#define NUM_OFFSETS 1024
#define NUM_SCRIPTS 800
#define NUM_MODULE NUM_SCRIPTS /* one module record per script    */
#define NUM_TYCON 2000
#define NUM_NAME 32000
#define NUM_INSTS 4000
#define NUM_CLASSES 1000

/* Tables indexed by an ordinary Int rather than by a Cell */
#define NUM_HANDLES 40
#define NUM_LAT1_CHARS 256
#define NUM_FIXUPS 1000
#define NUM_TEXT 320000
#define NUM_TEXTH 10
#define NUM_TYVARS 8000
#define NUM_STACK 160000
#define NUM_ADDRS 1280000
#define NUM_STABLEPTRS 10000
#define NUM_MALLOCPTRS 10000
#define NUM_DTUPLES 5

/* The heap, in cells.  MAXIMUMHEAP of 0 means no ceiling. */
#define MINIMUMHEAP 19000
#define MAXIMUMHEAP 0
#define DEFAULTHEAP 1000000

/* Some infinite computations generate an infinite depth of alternations
 * between eval() and run().  If they don't use the Cuddles stack or heap,
 * they will overrun the C stack, crashing the interpreter.  To protect
 * against this, we place a limit on the depth of recursion of eval().
 */
#define MAX_EVAL_DEPTH 16384

#endif /* __SIZES_H__ */
