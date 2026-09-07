/* --------------------------------------------------------------------------
 * Strongly connected components algorithm for static.c and type.c.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "scc.h"

/* --------------------------------------------------------------------------
 * A couple of parts of this program require an algorithm for sorting a list
 * of values (with some added dependency information) into a list of strongly
 * connected components in which each value appears before its dependents.
 *
 * The algorithm used here is based on those described in:
 * 1) Robert Tarjan, Depth-first search and Linear Graph Algorithms,
 *    SIAM J COMPUT, vol 1, no 2, June 1972, pp.146-160.
 * 2) Aho, Hopcroft and Ullman, Design and Analysis of Algorithms,
 *    Addison Wesley, 1972.  pp.189-195.
 * The version used here probably owes most to the latter presentation but
 * has been modified to simplify the algorithm and improve the use of space.
 *
 * The callers differ only in where a node keeps its dependency list, so
 * they pass accessors for that field; the working state below is shared
 * because the sort never nests.
 * ------------------------------------------------------------------------*/

static List          daSccs = NIL;	/* SCCs found so far (a GC root)   */
static Int           daCount;		/* next depth first search number  */
static SccDepends    depends;		/* accessors for the current sort  */
static SccSetDepends setDepends;

#define visited(d) (isInt(depends(d)))	/* node already visited?	   */

static Int  local sccMin  (Int,Int);
static Int  local lowlink (Cell);

static Int local sccMin(Int x, Int y)		/* calculate minimum of x,y	   */
{				/* (unless y is zero)		   */
    return (x<=y || y==0) ? x : y;
}

static Int local lowlink(Cell v)		/* calculate `lowlink' of v	   */
{
    Int  low = daCount;
    Int  dfn = daCount; 		/* depth first search no. of v	   */
    List ws  = depends(v);		/* adjacency list for v		   */

    setDepends(v,mkInt(daCount++));     /* push v onto stack		   */
    push(v);

    while (nonNull(ws)) {		/* scan adjacency list for v	   */
	Cell w = hd(ws);
	ws     = tl(ws);
	low    = sccMin(low, (visited(w) ? intOf(depends(w)) : lowlink(w)));
    }

    if (low == dfn) {			/* start a new scc?		   */
	List temp=NIL;
	do {				/* take elements from stack	   */
	    setDepends(top(),mkInt(0));
	    temp = cons(top(),temp);
	} while (pop()!=v);
	daSccs = cons(temp,daSccs);	/* make new strongly connected comp*/
    }

    return low;
}

List sccSort(List bs, List cs, SccDepends d, SccSetDepends s)
{				/* sort lists with added dependency*/
    depends    = d;			/* info into SCCs		   */
    setDepends = s;
    clearStack();
    daSccs = NIL;			/* clear current list of SCCs	   */

    for (daCount=1; nonNull(bs); bs=tl(bs))	 /* visit each binding	   */
	if (!visited(hd(bs)))
	    lowlink(hd(bs));
    for (; nonNull(cs); cs=tl(cs))
	if (!visited(hd(cs)))
	    lowlink(hd(cs));

    return rev(daSccs);			/* reverse to obtain correct order */
}

Void sccControl(Int what)
{
    switch (what) {
	case RESET : daSccs = NIL;
		     break;
	case MARK  : mark(daSccs);
		     break;
    }
}

/*-------------------------------------------------------------------------*/
