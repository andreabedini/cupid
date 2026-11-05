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

The architecture is Gofer's, first released in 1991 by Mark Jones and
described in his 1994 report, [RR-1030](http://web.cecs.pdx.edu/~mpj/pubs/RR-1030.pdf).  Gofer's goals were compatibility with Haskell,
extensibility as a foundation for further work and experimentation, and
portability down to an 8 MHz 8086 PC with 640K; speed was explicitly not
one (RR-1030, p. 2).  Cupid
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
once.  (The report describes these at pp. 5, 6 and 22.)  Gofer carried
experimental features its report already calls obsolete, or expects to
drop (p. 44); Cupid deletes them.  And Gofer tracked each version of
Haskell and still differed from it, an incompatibility its author came to
find "a little frustrating" (p. 2); Cupid does not chase GHC.  A feature gets
in by [the rules for the language](language-design.md#how-a-feature-gets-in),
and what is left out is written down as well as what is in.

## Where the rest of the argument is

The arguments for particular choices are kept with the subjects they
are about:

- [Why the language is what it is](language-design.md): how a feature gets
  into the language, what stays out and why, and why the libraries follow
  GHC's base.
- [Cells, text and tables](cells-text-and-tables.md): why everything is
  one untyped cell, and what the conservative collector costs.
- [How code is compiled and
  run](how-code-is-compiled-and-run.md#one-invariant-to-respect): why no
  code is generated during evaluation.
- [The running interpreter](the-running-interpreter.md): subsystems,
  interrupts, signals, output, and why commands run without a shell.
- [Platforms and the build](platforms-and-the-build.md): why nothing is
  probed, why there are only two feature options, and why every
  configuration is tested.
- [How the C is written](how-the-c-is-written.md): the conventions, and
  what each is for.
