# Cells, text and tables

Everything the interpreter works on is built from one data type.  Tokens,
parse trees, types, kinds, predicates and the compiler's intermediate forms
are mostly trees of `Cell`s, though nodes can be shared even there, and the
running program is a graph in which sharing is the point.  They are all
allocated from one heap, and the things that have names live in fixed tables
that cells index.  The design comes from Gofer, the 1991 interpreter behind
Hugs; where the code has moved on since, this chapter says so.  The code is `src/storage.h` and
`src/storage.c`, and the table sizes are in `src/sizes.h`.

The way to learn this representation is to watch something go through it,
and the forms below are a debugger's view of the heap, with the numbers of
the heap cells left out because they change from run to run.  Everything is
printed in terms of pairs.  `(a . b)` is a pair, so the application `f x`
prints as `(f . x)`.  `TAG(a)` is a pair whose first half is the tag `TAG`.
`VARIDCELL xs` is a pair whose second half is a piece of text, shown decoded.

Here is one expression from [`Total.hs`](how-cupid-runs-a-program.md), the
right-hand side of `total`'s second equation, `x + total xs`, as it stands
when the parser has finished with it:

<!-- session: cells-parsed -->
```
INFIX(((VARIDCELL + . ONLY(VARIDCELL x)) . (VARIDCELL total . VARIDCELL xs)))
```

That looks like a lot of machinery for four tokens.  Most of it is one idea
used repeatedly, and we'll take it from the inside out.

## `xs`: a name is a number, and a pair holds it

The innermost thing is `VARIDCELL xs`, the value the lexer returned for the
token `xs`.  It starts with the string.

Every identifier, keyword and string literal is stored once, in a character
table, and referred to by its offset in that table, a `Text`.  `findText`
looks a string up, adding it the first time, and `textToStr` goes back.  Two
strings are equal if and only if their `Text`s are equal, so a symbol-table
lookup compares one integer instead of walking two strings, and a name used
throughout a program costs its characters once.

A `Text` is only a number, so the lexer has to say what kind of thing it's
looking at.  `mkVar(t)` makes a pair of two integers, the tag `VARIDCELL` and
the `t`, and this pair is the first cell we've allocated.  A `Cell` is a C
`int`, and a pair is a negative one.  If `c` is that pair, `fst(c)` is
`VARIDCELL`, `snd(c)` is the `Text`, and `textOf(c)` gets at the latter.

The two accessors are worth looking at, because they're the whole heap:

```c
#define fst(c) heapTopFst[c]
#define snd(c) heapTopSnd[c]
```

`heapTopFst` points one past the end of the array `heapFst`, so a negative
index lands inside it, and testing for a pair is a sign test: `isPair(c)` is
`c < 0`.  There is no array of two-word structs.  Two arrays were a concession
to the PC Gofer was written on, which limited an array to 64K ([RR-1030](http://web.cecs.pdx.edu/~mpj/pubs/RR-1030.pdf), p. 11),
and the layout has stayed.  The report admits that this makes variable-sized
objects much more difficult, and adds that the collector's constraints would have
made them almost impossible anyway.  So the heap holds pairs and nothing else, and
an array is a linked structure of pairs, not a block.  The heap's size is set once
with `-h` and never grows.

## `total xs`: the same pair, without a tag

The next thing out is `(VARIDCELL total . VARIDCELL xs)`, the application.
`ap(f, x)` makes a pair of the function and the argument, and nothing else: an
application has no tag.  So how does anyone know it's an application?  By the
first half not being a tag.  This is `whatIs`, which the whole interpreter
dispatches on, and which profiling suggested was one of the most frequently
executed functions in Gofer (RR-1030, p. 10):

```c
CUPID_inline CellTag whatIs(Cell c) {
    return (CellTag)(isPair(c) ? (isTag(fst(c)) ? fst(c) : AP) : whatCode[c]);
}
```

`AP` is what it returns for a pair with no tag, and `fun(c)` and `arg(c)` are the
same `fst` and `snd` under names that say what they mean here.  So are `hd(c)` and
`tl(c)`: a list is a chain of pairs ending in `NIL`, the cell 0, which is the empty
list and the general "nothing".  Nothing in the C type system tells these apart.

That's deliberate.  A `Cell` is an alias for `int`, and so are `Type`, `Kind`,
`Name`, `List` and the other domain types.  They document intent without checking
it; nothing stops a `Type` being passed where a `Name` is wanted.  Gofer's author
weighed introducing distinct struct types, which some C compilers of the day
couldn't pass as arguments, and added that the ability to pun, treating a value of
one type as another, had turned out to be quite convenient (RR-1030, p. 5).  Cupid
keeps the aliases for a further reason: wrapper structs would fight the thousands
of uses of generic operations such as `hd` and `cons`, and a typed heap would be a
rewrite of the runtime.  Instead, in a `checks` build, the entry points where
categories meet check a tree's shape at run time (`tagcheck.c`).

## `INFIX` and `ONLY`: two kinds of tag

What remains of the printed form is the chain, which reads from the middle: `ONLY x`
is the first operand, `+` is paired onto it, and that is applied to `total xs`.
`INFIX(...)` marks the whole as not yet sorted out.  `INFIX` and `ONLY` are tags too,
but of a different kind from `VARIDCELL`, and the kind is decided by the tag's number:

- A *box tag*, such as `VARIDCELL`, `INTCELL` or `STRCELL`, says that the second half
  is **not** a cell.  It's a raw `Text`, or an integer.
- A *constructor tag*, such as `INFIX`, `ONLY`, `LETREC` or `FATBAR`, says that the
  second half is a cell.

The difference matters chiefly to the garbage collector, which has to know which
integers to follow.

## `+`: some cells aren't in the heap at all

Dependency analysis resolves the operators and the imported names, and the same
expression becomes this:

<!-- session: cells-resolved -->
```
((+ . VARIDCELL x) . (VARIDCELL total . VARIDCELL xs))
```

The chain is gone, and `+` has changed from a box holding the text `+` to the `Name` of
the class method.  This print-out shows it by name only, because a `Name` isn't a pair.
It's a non-negative cell, in a range of integers reserved for names.  Which kind of
value a non-negative cell is depends only on which range it falls in: tags come first,
then offsets, handles into the tables, characters and small integers, in the order
[given at the end of this chapter](#the-ranges).  That's the last part of `whatIs`:
`whatCode[]` is a table with an entry for every non-negative cell.

A value that is used constantly, a name, a type constructor or a small integer, needs
no storage at all, however many times it occurs.  The costs are that decoding a cell
takes a few more instructions than following a pointer, and that the ranges are fixed
when the interpreter is built, so a table that fills up reports an error rather than
growing, usually "storage space exhausted".

The record for `+` is at `nameRec(n)`, which turns the handle into a pointer into a
fixed array.  A handle has to be a `Cell`, because handles are stored in heap cells and
`NIL` must be a valid value, and the pointer stays valid because the arrays never move.

## The tree is rewritten where it stands

The type checker comes next, and it doesn't build a new expression.  It overwrites this
one:

<!-- session: cells-typed -->
```
(((+ . (_indirect . DICTVAR d)) . VARIDCELL x)
 . ((VARIDCELL total . (_indirect . DICTVAR d)) . VARIDCELL xs))
```

Each use of an overloaded name now has a dictionary argument.  A pair can be overwritten
because `fst` and `snd` are lvalues, and that's how the dictionary gets in after the fact: the
placeholder for the dictionary was already in this tree, and the solver wrote over it, an
application of `_indirect` to the parameter `d`.  [How types are
inferred](how-types-are-inferred.md#the-expression-is-rewritten-in-place) tells that
story.

`d` is a name the compiler invented, and an invented name needs no characters.
`inventDictText` counts downward from -1 and `inventText`, which makes the `vN` names of
lambda-lifted functions, counts upward from the top of the character table, so an invented
`Text` lies outside the table's range and can never collide with a real string.
`textToStr` prints them as `dN` and `vN`, and a `vN` is what you see when a pattern match
fails inside an invented function at the prompt.

The compiler then gets rid of the names of variables altogether:

<!-- session: cells-compiled -->
```
(((+ . OFFSET 2) . OFFSET 4) . ((total . OFFSET 2) . OFFSET 3))
```

`OFFSET 2` is the dictionary, 3 is `xs` and 4 is `x`: the same numbers as the `LOAD`s in the
listing for `total`.  An offset is another number from the ranges, and `mkOffset(n)` allocates
nothing, so at this point the expression holds no boxes and no text.  `total` has become a `Name` as well:
a reference to one of the module's own top-level bindings stays a variable until a later
stage has a use for the name, and here that's the pattern-matching compiler.  This is the
tree `codeGen` reads, to emit the instructions that [How code is compiled and
run](how-code-is-compiled-and-run.md#reading-a-listing) goes through.

And then it's discarded.  Once `codeGen` has emitted its instructions nothing refers to this
tree, and the next collection takes its cells.  At run time the machine has only those
instructions, and each call of `total` runs them to construct a new piece of graph with the
same shape.  Unlike the compiled tree, that graph holds real values and can share nodes with the
graph around it.  Here's the root of the first call of `total`, after its code has overwritten
it, for the list `[1, 2, 3]`:

<!-- session: cells-running -->
```
(((+ . dNumInt) . ((fromInt . dNumInt) . int 1))
 . ((total . dNumInt)
    . ((: . ((fromInt . dNumInt) . int 2))
       . ((: . ((fromInt . dNumInt) . int 3)) . []))))
```

The offsets have been replaced by what they pointed at, in a graph made of the same pairs
the parser used.  `:` and `[]` are `Name`s, and `(total . dNumInt)` is the node `ROOT 1`
reused.  `int 1` is a small-integer cell, which again takes no storage.  An integer outside
the small range, which is 4096 wide and centred on zero, is a pair whose first half is
`INTCELL`, allocated by `mkInt`.  A character beyond `NUM_SHORT_CHARS` and a string are boxed
likewise, by `mkChar` and `mkStr`.  A `Double` does not fit in one component, so `mkDouble`
makes a `DOUBLECELL` pair holding a pair of two integer cells.  Since an integer may be small
or boxed, `isInt`, `intOf` and `mkInt` handle both forms, and the primitives go through them.
The width of the small range is a balance between boxing fewer integers and keeping the
`whatCode[]` table small.

## What survives a collection

Every one of those pairs came from the same function.  `pair(x, y)` takes a cell from
`freeList`, and when the list is empty it collects: every live cell is marked, then the
whole heap is swept and the unmarked cells are linked into a new free list.  A sweep costs
time proportional to the size of the heap, not to the amount in use; a copying collector
would avoid that and would allow variable-sized objects.  It is not possible here, for a
reason we'll get to.

"Live" means reachable from a root, and walking from a root is where the two kinds of tag
matter.  `markSlot` looks at a pair's first half.  If that's another pair, it follows both.
If it's a constructor tag or a name, it follows only the second half.  If it's a box tag it
stops, since the second half is a raw number that merely looks like a cell.  The walk keeps
its own explicit stack rather than recursing in C, because the graph can be arbitrarily deep
and would overflow the C stack.  On its way it also shortens chains of `INDIRECT` cells, so an
indirection that nothing needs any more doesn't survive.  `INDIRECT` is the tag the machine writes
when [a call's result already exists
elsewhere](how-code-is-compiled-and-run.md#updating-and-why-there-are-two-ways); it isn't the type
checker's `_indirect`, which the compiler removes.

The roots are what you'd want to keep.  Follow our expression:

- **While loading,** `valDefns` holds the whole program, and it's a root: every module that
  keeps cells in static variables lists them in a table it hands to `registerRoots` once, at
  install time.  So the parse tree survives any collection triggered by the type checker's
  allocating.
- **After loading,** it doesn't.  `compileDefns` walks `valDefns` down to `NIL` as it goes, and
  sets each compiled name's `defn` to `NIL`, so the equations are unreachable and the next
  collection takes them.  What's left of `total` is its record in the name table, which
  `markStorage` marks field by field along with each type constructor's constructors and so on,
  so its type survives, and its code, which is in the code area and not in the heap at all.
- **While running,** the graph above is held by the interpreter's stack.  `cellStack` is a single
  stack of cells with `push`, `pop`, `top` and `pushed(n)`, the entry `n` places below the top,
  and every entry between the bottom and the stack pointer is a root.  So `MKAP`, which does
  `pushed(1) = ap(pushed(0), pushed(1))`, is safe: the arguments are on the stack when the
  allocation happens.  For code whose arguments aren't, `pair` itself saves them in `lsave` and
  `rsave` so that a collection triggered by the allocation does not lose them.  The parser and
  the strongly-connected-components sort use the same stack, and [How source is
  read](how-source-is-read.md#the-parsers-stack-is-shadowed) shows the parser's use of it.
- **A constant** keeps its value for as long as its script is loaded, since the value is stored
  in the name's `defn` and that's marked with the name.

You can watch that last one happen.  `tests/doc-sessions/Cells.hs` defines one constant:

<!-- source: Cells.hs -->
```haskell
table :: [Int]
table = [1 .. 20000]
```

and `:gc` forces a collection and reports how many cells it freed.  Your counts may differ with
a different revision of the libraries, but they should keep these relations:

```
$ cupid -i Cells.hs
Cells> :gc
Garbage collection recovered 982494 cells
Cells> length table
20000
Cells> :gc
Garbage collection recovered 924446 cells
Cells> length (map negate table)
20000
Cells> :gc
Garbage collection recovered 924463 cells
```

After `length table`, 58,048 fewer cells are free, and they stay that way, because `table` is
a constant and holds the list.  You can account for nearly all of them.  Each `x : xs` is two
application pairs, as in the graph above, which makes 40,000.  An integer above about 2,000
doesn't fit in the small range, so it takes a box, which makes about 18,000 more.  `map negate
table` allocated a new list of the same size, and no constant holds that one, so the collection
took it all back.

### The conservative scan

There is one more kind of root, and it's the compromise the whole storage layer turns on.  Any C
function in the interpreter may call `pair`, and any of them may be holding cells in locals when
the collection happens.  The type checker builds a list in a local variable, calls something that
allocates, and goes on using the list.  Nothing has registered that local.  So while source is being
loaded and compiled the collector treats the C stack as a source of roots: every word on it that looks like a
heap cell keeps that cell alive (`gcCStack`).  `markWithoutMove` marks the cell and never rewrites
the word, since it may be an integer or a return address that happens to fall in range.

A copying collector is impossible for that reason: it must update every pointer to a moved object,
and some of the pointers to the heap are in C local variables that can't be told from integers.  In
exchange, the compiler and the type checker can be written in ordinary C that builds lists freely.
Gofer's author called reading the C stack "very poor C programming practice" and the collector as a
whole "an enormous pain", and then observed that it had survived programs running for several days
through several tens of thousands of collections (RR-1030, p. 15).

The scan shows up in the build.  Fifteen files that hold cells in C local variables during allocation,
listed as `gc_sensitive` in `src/meson.build`, are compiled at `-O0`.  An optimiser may compute
`&heapTopFst[c]`, discard `c` and keep only the address.  The scan wouldn't recognise that, so the pair
could be freed while still in use.  During evaluation the scan is off (`startEval` clears `consGC`), so
`machine.c` and the primitives are written not to depend on it, with one exception in the bignum code,
and are optimised as usual.  The cost is that a value which
merely looks like a pointer keeps its target alive, so a large structure can live longer than it is
needed.  [How the C is written](how-the-c-is-written.md#cells-held-in-c) has the rules this imposes.

## Tables and checkpoints

Names, type constructors, classes, instances, modules and scripts each live in a fixed array with a
high-water mark (`nameHw`, `tyconHw`, `classHw`, `instHw`, ...) pointing at the next free slot.  A
handle such as a `Name` is its record's index plus the table's start in the cell range (`NAMEMIN`).  The
record for `total` was added at the mark when its file was read.

The marks give the interpreter a checkpoint mechanism for free.  `startNewScript` records the value of
every high-water mark, the text table's included.  `dropScriptsFrom(s)` rewinds them all to the values
recorded before script `s` was read, and with that `total`'s record is gone.  It also trims the text hash
tables, clears the name and type-constructor hashes, which are rebuilt per module, and prunes the instance
lists of the classes that survive.

Error recovery and reloading are built on this pair of functions: a script that fails to load is dropped by
rewinding; the prompt starts an anonymous script before each expression and drops it afterwards; and
`:reload` finds the first script whose file has changed, drops it and everything loaded after it, and reads
from there on.  Scripts form a strict stack, so editing a script reloads it and every later one, but never
an earlier one.  Gofer's author noted that a dependency analysis between scripts could avoid some of that
reloading and that in practice it is fast enough not to matter (RR-1030, p. 4).

The character table is one of these arrays (`NUM_TEXT` in `src/sizes.h`).  One hash picks a row of
`NUM_TEXTH` entries in `textHash`; if none of them matches, `findText` scans on linearly.  The rows are old:
Gofer's first version used one hash table to pick a starting point for a linear search, until over 80% of the
time to load a large program turned out to be spent in `findText`.  Ten levels of hash tables gave large
programs a fivefold increase in compilation speed (RR-1030, p. 7).

## The ranges

For reference, these are all the kinds of cell.  The tags are a `typedef enum CellTag`, and [How the C is
written](how-the-c-is-written.md) says how a `switch (whatIs(c))` is written.

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

The ranges are laid out in this order in `storage.h`, each one's start computed from the previous one's size.
They are order-sensitive, so `static_assert`s at the end of the file check that none of them overlap.
