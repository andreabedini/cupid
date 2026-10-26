# Cells, text and tables

Everything the interpreter works on is built from one data type.
Tokens, parse trees, types, kinds, predicates, the compiler's
intermediate forms and the graph of the running program are all trees
of `Cell`s, allocated from one heap, and the things that have names
live in fixed tables that cells index.  This page explains that
representation and the reasons for it.  The design is Gofer's, from
1991, and most of the reasons its author gave for it still hold; where
the code has moved on since, this page says so.  The code is
`src/storage.h` and `src/storage.c`.

## Text: strings interned once

Every identifier, keyword and string literal is stored once, in a
character table, and referred to by its offset in that table, a
`Text`.  `findText` looks a string up, adding it the first time, and
`textToStr` goes back.  Two strings are equal if and only if their
`Text`s are equal, so a symbol-table lookup compares one integer
instead of walking two strings, and a name used throughout a program
costs its characters once.

The table is a fixed array (`NUM_TEXT` in `src/sizes.h`), searched
through `NUM_TEXTH` levels of hashing before a linear scan.  The
layering is old: Gofer's first version used one hash table to pick a
starting point for a linear search, and profiling showed that loading
a large program spent most of its time in `findText`; several levels
of hashing made loading five times faster.

Names the compiler invents, for the lambda-lifted functions and the
dictionary parameters it introduces, need no characters.  `inventText`
counts upward from the top of the table and `inventDictText` counts
downward from -1, so an invented `Text` lies outside the table's
range and can never collide with a real string.  `textToStr` prints
them as `vN` and `dN`, which is what you see when a lambda or an
overloaded function is shown at the prompt.

## A cell is an integer in a range

A `Cell` is a C `int`, and which kind of value it is depends only on
which range of integers it falls in:

| range | value |
|-------|-------|
| negative | a pair in the heap |
| 0 | `NIL`: the empty list, and the general "nothing" |
| `TAGMIN` .. `BCSTAG`-1 | a *box tag*: the first component of a pair whose second component is not a cell (`INTCELL`, `STRCELL`, `VARIDCELL`, ...), and the indirection tags `INDIRECT` and `INDIRECT1` |
| `BCSTAG` .. `SPECMIN`-1 | a *constructor tag*: the first component of a pair whose second component is a cell (`FROMQUAL`, `LETREC`, `FATBAR`, ...) |
| `SPECMIN` .. | special constants such as `WILDCARD`, `STAR`, `SKOLEM` |
| `TUPMIN` .. | the tuple constructors, `mkTuple(n)` |
| `OFFMIN` .. | offsets, `mkOffset(n)`: type variables in a type scheme, bound variables in compiled code |
| `MODMIN`, `TYCMIN`, `NAMEMIN`, `INSTMIN`, `CLASSMIN` .. | handles into the module, type constructor, name, instance and class tables |
| `CHARMIN` .. | the first `NUM_SHORT_CHARS` characters |
| `INTMIN` .. `INTMAX` | small integers, centred on `INTZERO` |

The ranges are laid out in this order in `storage.h`, each one's start
computed from the previous one's size, and `static_assert`s at the end
of the file check that none of them overlap.

The reasons for encoding cells as integers rather than pointers were
given in 1994 and are still the reasons the layout is what it is.  A
value that is used constantly, a name, a type constructor or a small
integer, needs no storage at all, however many times it occurs.  Testing
for a pair is a sign test, `isPair(c)` is `c < 0`, and `whatIs`, which
the whole interpreter dispatches on, is a table lookup for every
non-negative cell (`whatCode[]`) and a read of the first component for
a pair.  `whatIs` was measured as one of the most frequently executed
functions in the system, which is why it is worth this much care.  The
costs are also as stated then: decoding a cell takes a few more
instructions than following a pointer, and the ranges are fixed when
the interpreter is built, so a table that fills up reports "storage
space exhausted" rather than growing.  The limits are in
`src/sizes.h`.

The tags are a `typedef enum CellTag`, so a `switch (whatIs(c))` is
one that `-Wswitch` can check; [The design of Cupid](design.md#the-c-code)
says how such switches are written.

**Boxed values.**  An integer outside the small range, a character
beyond `NUM_SHORT_CHARS`, a string, a `Double`: each is a pair whose
first component is a box tag and whose second component holds the
value, allocated by `mkInt`, `mkChar`, `mkStr` and so on.  Every
arithmetic primitive therefore handles both the unboxed and the boxed
form.  The small-integer range is 4096 wide, a balance between boxing
fewer integers and keeping the `whatCode[]` table small; Gofer gave
small integers almost the whole positive range of an `int`, but its
cells were 16 bits wide and its `Int` was boxed anyway.

**Why not a tagged union.**  A `Cell` is an alias for `int`, and so are
`Type`, `Kind`, `Name`, `List` and the other domain types.  They
document intent without checking it; nothing stops a `Type` being
passed where a `Name` is wanted.  Gofer's author weighed introducing
distinct struct types and decided that the ability to pun, treating a
value of one kind as another, was too convenient to give up.  Cupid
keeps the aliases and adds run-time shape checks at the boundaries
between categories in a `checks` build; [The design of
Cupid](design.md#data-and-memory) explains that choice.

## The heap is two arrays

A pair is a negative cell, and its two components are read from two
parallel arrays: `fst(c)` is `heapTopFst[c]` where `heapTopFst` points
one past the end of `heapFst`, so a negative index lands inside the
array.  There is no array of two-word structs.  Two arrays were a
concession to the 64K segments of the 16-bit PC Gofer was written on,
and the layout has never changed, because the garbage collector was
designed around it.  The consequence is that the heap holds pairs and
nothing else: there are no variable-sized objects, so an array is a
linked structure of pairs, not a block.  The heap's size is set once
with `-h` and never grows.

## Garbage collection

`pair(x, y)` takes a cell from `freeList`.  When the list is empty it
collects: every live cell is marked, then the whole heap is swept and
the unmarked cells are linked into a new free list.  A sweep costs
time proportional to the size of the heap, not to the amount in use,
and allocating from a free list costs a test per pair; a copying
collector with a bump pointer would be cheaper on both counts and
would allow variable-sized objects.  It is not possible here, because a
copying collector must update every pointer to a moved object, and
some of the pointers to the heap are in C local variables.

That is the central compromise of the storage layer.  Any C function
in the interpreter may call `pair`, and any of them may be holding
cells in locals when the collection happens, so the collector treats
the C stack as a source of roots: every word on it that looks like a
heap cell keeps that cell alive (`gcCStack`, marking with
`markWithoutMove`, which never rewrites the word, since it may be an
integer or a return address that happens to fall in range).  This is
what lets the compiler and the type checker be written in ordinary C
that builds lists freely.  It is also why those files are compiled
without optimisation, why the type checker must never run with the
conservative scan switched off, and why a value that looks like a
pointer can keep a large structure alive longer than it is needed;
[The design of Cupid](design.md#data-and-memory) states the rules.
Gofer's author called the scheme "an enormous pain" and "very poor C
programming practice", and then observed that it had survived programs
running for days through tens of thousands of collections.  Both
remarks remain fair.

The other roots are explicit.  The symbol tables are marked field by
field (`markStorage` marks each name's type and definition, each type
constructor's constructors, and so on); every module that keeps cells
in static variables lists them in a table it hands to `registerRoots`
once, at install time; structured state is marked by a module's
`gcMark` hook; and every entry on the interpreter's own stack (below)
is a root.  Inside `pair` itself the two arguments are saved in
`lsave` and `rsave` so that a collection triggered by the allocation
does not lose them.  The marking phase keeps its own explicit stack
rather than recursing in C, because a long list once overflowed the C
stack while being marked.

When a module gains a new static cell, the rule from 1994 still
applies in its current form: the cell must be in a `registerRoots`
table, and it must be set to `NIL` when the module is installed and
reset, or the storage it points to is kept for longer than it is used.

## Tables, high-water marks and checkpoints

Names, type constructors, classes, instances, modules and scripts each
live in a fixed array with a high-water mark (`nameHw`, `tyconHw`,
`classHw`, `instHw`, ...) pointing at the next free slot.  A handle
such as a `Name` is the index of its record, and `nameRec(n)` turns it
into a pointer that stays valid because the arrays never move.

The marks give the interpreter a checkpoint mechanism for free.
`startNewScript` records the value of every high-water mark, the text
table's included, and `dropScriptsFrom(s)` rewinds them all to the
values recorded before script `s` was read, trims the text hash
tables, clears the name and type-constructor hashes (they are rebuilt
per module) and prunes the instance lists of the classes that survive.
This one pair of functions is the whole of error recovery and
reloading: a script that fails to load is dropped by rewinding; the
prompt starts an anonymous script before each expression and drops it
afterwards; and `:reload` finds the first script whose file has
changed, drops it and everything loaded after it, and reads from there
on.  Scripts form a strict stack, which is why editing a script
reloads it and every later one, but never an earlier one.  Gofer's
author noted that a dependency analysis between scripts could avoid
some of that reloading and that in practice it is fast enough not to
matter; that is still the case.

## The stack

`cellStack` is a single stack of cells with `push`, `pop`, `top` and
`pushed(n)`, the entry `n` places below the top.  Three parts of the
system share it: the parser, which shadows bison's stack on it so that
parse fragments survive a collection ([How source is
read](how-source-is-read.md)); the strongly-connected-components
sort; and the abstract machine, which keeps function arguments and
intermediate values on it ([How code is compiled and
run](how-code-is-compiled-and-run.md)).  Every entry between the
bottom and the stack pointer is a garbage-collection root.

## See also

- [The design of Cupid](design.md#data-and-memory) — the rules that
  follow from this representation
- [How Cupid runs a program](how-cupid-runs-a-program.md)
- [History](history.md) — where the design came from
