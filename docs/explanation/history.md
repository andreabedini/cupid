# History

Where Hugs came from, what happened to it, and what this tree is.

## Gofer and Hugs

In September 1991 Mark Jones released Gofer, a compact and portable
implementation of a Haskell-like language that also served as a test
bed for type-system ideas, several of which later entered Haskell:
constructor classes, and the treatment of type classes that became
multi-parameter classes and functional dependencies.  Gofer was small
enough to run on the machines of the day and was widely used for
teaching.

Jones described its implementation in a 1994 research report, [*The
implementation of the Gofer functional programming
system*](http://web.cecs.pdx.edu/~mpj/pubs/RR-1030.pdf) (Yale
University, YALEU/DCS/RR-1030, May 1994).  It covers Gofer 2.30,
before the module system, but the architecture
it sets out is the one this tree still has: everything a cell in one
heap, fixed tables with high-water marks as checkpoints, a
mark-and-sweep collector that scans the C stack, a bison parser whose
stack is shadowed so the collector can see it, structure-shared types
in a type checker that rewrites the program in place, lambda lifting
to supercombinators and a G-machine variant with the root
optimisation.  The explanation pages on
[storage](cells-text-and-tables.md), [the front
end](how-source-is-read.md), [type inference](how-types-are-inferred.md)
and [the compiler and machine](how-code-is-compiled-and-run.md) draw on
it and say where Hugs and Cupid moved on.  The report closes with the
hope that "someone, perhaps even me, has the opportunity to develop a
new, cleaner version of the system, building on this experience";
Cupid's aim of an implementation one person can read is in that
spirit, if thirty years late.

On 14 February 1995 Jones released Hugs, the Haskell User's Gofer
System, a derivative of Gofer with much closer conformance to Haskell.
Hugs versions were named after the version of Haskell they
implemented: Hugs 1.3, Hugs 1.4, and from January 1999 Hugs 98.  Hugs
98 was the first released implementation of the Haskell 98 standard
and the first Hugs released as open source.  Jones gave up
maintainership in January 2000; development continued at Yale, at OGI
and among a group of volunteers, chiefly Ross Paterson, Sigbjorn
Finne and Alastair Reid.

## Hugs 98, 1999 to 2006

The releases were named by month:

- **January 1999** — Hugs 98 beta.  Merged the module system and
  dynamic linking of Hugs 1.4 with the type-system extensions of Hugs
  1.3c: multi-parameter classes, TREX records, rank-2 polymorphism,
  existentials.
- **May 1999** — Largely Haskell 98 conformant.  Import chasing, a
  Windows GUI, a first foreign interface.
- **November 1999** — BSD licence.  `:browse` and `:version`,
  functional dependencies, better overlapping instances.  Bug-fix
  releases in February and July 2000.
- **February 2001** — A foreign function interface modelled on GHC's,
  the `observe` debugger, parallel list comprehensions, recursive
  `mdo`, classic Mac OS support.
- **December 2001** — Hierarchical module names, the missing standard
  libraries, most of `hslibs`, macOS X.
- **November 2002** — The FFI brought close to the specification, a
  large part of GHC's hierarchical libraries, a complete module
  system.
- **November 2003** — Internal restructuring.  `Double` becomes double
  precision, the libraries shared with GHC and nhc98 are adopted
  wholesale, imprecise exceptions, .NET support on Windows.  The
  beginnings of a user's guide.
- **March 2005** — Unicode `Char` and locale-based I/O, the `-X`
  option, the new `ffihugs` syntax, Cabal support, and a Cabal-based
  library build.
- **May 2006** — WinHugs rewritten by Neil Mitchell, the `:main`
  command, the empty `Hugs` module as the default scope.
- **September 2006** — A bug-fix release with libraries matching GHC
  6.6.

September 2006 was the last release.  By then GHC had an interactive
mode of its own, the shared libraries had moved on, and the three-stage
bootstrap that turned the shared sources into something Hugs could
load had become the main cost of keeping up.  Hugs remained in
distributions for some years, then stopped building on current
compilers.

## This tree

This tree starts from the September 2006 release with one aim: a
simpler, understandable Haskell implementation.  It is not an attempt
to keep up with GHC, but it is meant to run the Haskell people write.
The language is Haskell 2010 plus the parts of modern Haskell that
real code needs and one type checker can hold; everything from 2006
that did not serve the aim has been removed rather than kept behind
flags.  In order:

- the sources were made standard C23 and the K&R definitions converted
- autoconf was replaced by meson, with the platform facts written down
  rather than probed
- the dead platforms (DOS, classic Mac OS, Symbian, Win16, the old
  Unixes) were deleted
- the flag-guarded extensions and n+k patterns were deleted; pattern
  guards were added
- the base library was taken over as plain Haskell and the
  Cabal/cpphs/hsc2hs bootstrap removed
- the packaging, the DocBook user's guide and the release notes were
  replaced with this documentation
- the `-98` switch was dropped, so the extensions are always on
- kind signatures, GADT syntax and equality constraints were added to
  the one constraint solver, and literate scripts were dropped
- the fork was named Cupid, after the day Hugs was first released,
  and everything that carried the Hugs name was renamed; the
  `haskell98` flat-module package, the .NET remnants and the unbuilt
  2006 `lib/` and `include/` trees were dropped, and the three
  modules the demos need from `lib/` moved into `demos/`
- the libraries took the class hierarchy and API of GHC's base, and
  the language gained what packages written for GHC use most: type
  operators, tuple sections, polymorphic and constraint synonyms, and
  GHC's deriving strategies with `Generic` and `Data`
- Windows became a supported platform, built natively without a POSIX
  layer, and releases are built as archives for Linux, macOS and
  Windows

[The design of Cupid](design.md) explains the choices that came out of
these changes, and why they were made.

## See also

- [How Cupid runs a program](how-cupid-runs-a-program.md)
- [The language Cupid accepts](../reference/language.md)
