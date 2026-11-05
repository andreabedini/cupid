# The design of Cupid

Cupid has one aim: to be a Haskell implementation you can sit down and
read.  Three properties follow from it and do not change: Cupid is an
interpreter written in C, it does not self-host, and it stays small.
Most of the choices described in these pages are consequences of that
aim, and each page says why it chose what it did, so that the next
change can be checked against the same reasons.

It isn't there yet.  Much of the C is still Hugs's, tidied rather than
rewritten.  But the aim is what a change is judged against.

## Deleting instead of switching off

The rule behind most of what follows is that a feature which doesn't
serve the aim is deleted.  A flag would be a promise to keep two code
paths working forever, and every such promise makes the code harder to
read.  So Cupid has no language-extension flags and no compatibility
modes.  The extensions it has are always on, except overlapping
instances, which need the `+o` toggle (`+O` for the unsafe kind).  The
extensions Hugs had that it doesn't keep are gone from the source.

## What is inherited and what is not

The architecture is Gofer's, designed in 1991 by Mark Jones and
described in his 1994 report ([History](history.md#gofer-and-hugs)
says where to find it).  Gofer's goals were compatibility with Haskell,
extensibility as a test bed for type-system ideas, and portability
down to an 8 MHz PC with 640K; speed was explicitly not one.  Cupid
keeps the parts of that design that serve readability, and they are
most of it: one cell type for everything, fixed tables whose
high-water marks double as checkpoints, a conservative collector so
that ordinary C can build in the heap, a type checker that rewrites
the program in place, and a small abstract machine.  Where the report
gives a reason for a choice, these pages repeat it, because the reason
is usually still the reason.

The departures follow from the code's age and the different aim.  Gofer
was written for K&R compilers and 16-bit machines, so it leaned on the
preprocessor: `Args(...)` prototypes, map-like macros, a source file
included several times as a template, error reporting as macros.  Cupid
targets C23 on 64-bit systems and makes each of these a function or a
plain declaration.  Gofer's components were driven by a `control`
function per file answering `INSTALL`, `RESET`, `MARK` and `EXIT` codes;
Cupid makes each phase a named function in a subsystem table.  Gofer
marked its static cells in each file's `MARK` case; Cupid registers them
once.  Gofer kept old features compilable behind definitions, with a
footnote wishing it could remove some; Cupid deletes them.  And Gofer
aimed at compatibility with a moving target, which its author came to
find "a little frustrating"; Cupid does not chase GHC.  A feature gets
in by [the rules for the language](language-design.md#how-a-feature-gets-in),
and what is left out is written down as well as what is in.

## Data and memory

**Everything is a cell, checked by its tag.**  Types, kinds,
predicates, lists and expressions are all trees of heap cells (`Cell`).
The C type system can't tell them apart.  Wrapper structs would fight
the thousands of uses of generic operations such as `hd` and `cons`,
and a typed heap would be a rewrite of the runtime.  Instead, under
`-Dchecks`, the entry points where categories meet check a tree's
shape at run time (`tagcheck.c`).  The typedef aliases (`Type`, `Kind`
and so on) remain as documentation.

**Tables are fixed arrays with asserted limits.**  Names, type
constructors, classes, instances and scripts each live in a fixed
array.  A handle such as a `Name` is a `Cell`, because handles are
stored in heap cells and `NIL` must be a valid value.  An accessor
such as `nameRec(n)` turns a handle into a pointer to its record, and
the pointer stays good because the arrays never move.  The table sizes
are in `src/sizes.h`.  The values that partition the cell space are
order-sensitive, so `static_assert`s at the end of `storage.h` check
them all.  A full table is reported as "storage space exhausted".

**The code generator's state and the type checker's result are passed,
not left in globals.**  The code generator's state is a `CodeGen` record
that its entry points create and pass on.  The type checker returns a
`Typing` record.  Those signatures then say what the functions read and
write; the rest of `type.c`'s working state is still file-scope.

**Garbage collection: roots are registered.**  The collector marks the
root tables that modules register at install time, each with
`registerRoots(xRoots)`.  A `gcMark` hook is needed only for structured
data.  While a program is loaded and checked it also scans the C stack
conservatively.  The fifteen files that hold cells in C local variables
during allocation, listed as `gc_sensitive` in `src/meson.build`, are
compiled at `-O0`.  An optimiser could keep only an interior pointer to
a cell, which the conservative scan would not find.  The type checker
relies on that scan, so it must never run with conservative collection
switched off.  During evaluation the scan is off (`startEval` clears
`consGC`), so `machine.c` and the primitives are written not to depend
on it and are optimised as usual.  The exception is `digitsQrm` in
`prims/bignums.c`, which turns the scan on for its own duration although
it is optimised too.

**No code is generated during evaluation.**  `run()` keeps a raw
pointer into the code area, which would be invalid if the area grew
under it.  The area can only grow when code is generated, so
`growMemory` reports an internal error if that ever happens during
evaluation.  An index instead of a pointer would cost an extra
register reload on every instruction.

[How Cupid runs a program](how-cupid-runs-a-program.md) puts these
pieces in the order a program meets them.

## Where the rest of the argument is

The arguments for particular choices are kept with the subjects they
are about:

- [Why the language is what it is](language-design.md): how a feature gets
  into the language, what stays out and why, and why the libraries follow
  GHC's base.
- [The running interpreter](the-running-interpreter.md): subsystems,
  interrupts, signals, output, and why commands run without a shell.
- [Platforms and the build](platforms-and-the-build.md): why nothing is
  probed, why there are only two feature options, and why every
  configuration is tested.
- [How the C is written](how-the-c-is-written.md): the conventions, and
  what each is for.
