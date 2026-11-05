# Why the language is what it is

Cupid is meant to run real Haskell, because an implementation that runs
only examples shows little of how Haskell works.  That is not the same as
following GHC.  This page explains how the line is drawn.  What is on each
side of it is in the language reference: [The language Cupid
accepts](../reference/language.md) for the overall statement, [Language
extensions](../reference/extensions.md) for each feature and for what is
not accepted.

## How a feature gets in

The type system grows one feature at a time.  Each feature is held to the
following rules, which exist to keep the type checker one pipeline that a
reader can follow:

- *Needed by real code.*  A feature comes in because programs people
  have written need it, found by loading packages from Hackage and
  seeing what stops them.  Without that evidence a feature stays out,
  however small; restricted type synonyms, kept from Hugs, are the one
  exception.  Where a need can be met by a library module, or by a
  tool such as the C preprocessor before loading, it is met there.
- *Always on.*  [Overlapping instances
  aside](design.md#deleting-instead-of-switching-off), a feature is good
  enough for every program, or it is not in the tree.
- *One solver.*  A feature extends the existing constraint solver; it
  never adds a second one.
- *Take the common case, reject the rest.*  Where inference is the hard
  part, a type signature is required.  Where termination is the hard
  part, the existing `-c` depth bound comes before any new termination
  check.
- *GHC's syntax, Cupid's rules.*  The syntax is GHC's, so programs
  read the same.  Where the rules differ, the difference is listed in
  [Language extensions](../reference/extensions.md).
- *Small and local.*  A feature is a few named functions in the file
  that owns its stage of the pipeline.
- *Nothing below the type checker changes.*  The type checker turns
  evidence into ordinary expressions (dictionary arguments), so code
  generation never sees a predicate.  A constructor written in GADT
  syntax with a variable not in its result is an ordinary existential
  constructor, and a match on it compiles as an existential match.  A
  feature that needs more than dictionary arguments at run time is not
  a candidate.

## Equality constraints: the rules at work

`a ~ b` is a good example of a feature that fits.  It is a predicate with
no evidence.  A given equality is unified at the point it is assumed, and
a wanted equality is solved by unification where it arises, so it never
waits in the solver and never reaches code generation.  Nothing could make
a stuck equality hold later, because there are no type families.  [How
constraints are solved](how-constraints-are-solved.md#equality-predicates)
follows it in detail.

## What stays out, and which rule keeps it out

Unboxed types and `MagicHash` change the abstract machine and the storage
underneath the type checker, which is the last rule above.  Template
Haskell needs the interpreter to run code while it is checking a module, a
second pipeline inside the first.  Linear types are a second discipline in
the type checker, for a guarantee nothing in Cupid uses.  Rewrite rules
have nothing to rewrite in an implementation that does not optimise.  STM
and asynchronous exceptions need a run-time system with preemptive
threads, which Cupid's cooperative threads are not.

Type families are the open question: they would have to be solved by the
one solver, and whether that can be done is not decided.  Functional
dependencies cover many of the same uses.

GADTs proper aren't in either.  Cupid accepts [GADT
syntax](../reference/extensions.md#gadt-syntax), but a constructor whose
result refines the type's arguments, such as `IntE :: Int -> Expr Int`, is
rejected, so a match never teaches the type checker a local equality.

## The libraries follow base, and owe nothing to the C

Cupid owns its libraries.  `libraries/base` and `libraries/cupid` are
plain Haskell, adapted for Cupid, with no conditional code for other
compilers.  Their API follows GHC's base, roughly base 4.18: the class
hierarchy, and the modules and functions that code written since 2006
imports.

"Small" is a property of the C, not of the libraries.  The C is what a
reader of Cupid reads; a library module is plain Haskell, apart from a
few small C helpers, and adds no C to the interpreter, so the
libraries can follow what programs import without working against the
aim.

Shared library sources and the cpp, cpphs and hsc2hs bootstrap they
needed used to be the largest single source of complexity in the tree.
Both are gone.
