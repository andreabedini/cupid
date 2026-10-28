# Further reading

Documents about the language Cupid accepts, the system it descends
from and the ideas its implementation is built on.

## The language

- Simon Marlow (ed.), [*Haskell 2010 Language
  Report*](https://www.haskell.org/onlinereport/haskell2010/) (2010).
  The definition Cupid follows; [Haskell 2010
  conformance](haskell-2010.md) goes through it chapter by chapter.
- Simon Peyton Jones (ed.), [*Haskell 98 Language and Libraries: the
  Revised Report*](https://www.haskell.org/onlinereport/) (Cambridge
  University Press, 2003).  The language Hugs implemented.
- [The documentation of `base`
  4.18](https://hackage.haskell.org/package/base-4.18.0.0), the version
  of GHC's base that Cupid's libraries follow.  Cupid ships a subset;
  [The libraries](libraries.md) lists what is there.

## Learning Haskell

- Paul Hudak, John Peterson and Joseph Fasel, [*A Gentle Introduction
  to Haskell*](https://www.haskell.org/tutorial/), version 98.  A short
  tour of the language for readers who have programmed before.  It
  predates `Applicative`, so a `Monad` instance written as it shows
  needs `Functor` and `Applicative` instances as well.
- [haskell.org](https://www.haskell.org/) lists current books, courses
  and tutorials.

## Hugs and Gofer

Hugs is no longer developed, but its documentation is still online.
It describes Hugs 98, not Cupid: [Differences from Hugs
98](differences-from-hugs.md) says what changed.

- [The Hugs home page](https://www.haskell.org/hugs/).
- [*The Hugs 98 User's
  Guide*](https://www.haskell.org/hugs/pages/users_guide/index.html),
  the manual of the later releases, with the deviations from Haskell
  98 and the extensions Hugs had.
- [*The Hugs 98 User
  Manual*](https://www.haskell.org/hugs/pages/hugsman/index.html), the
  older manual, with a tutorial for beginners and a description of the
  Hugs-only extensions.
- Mark P. Jones, [*The implementation of the Gofer functional
  programming system*](http://web.cecs.pdx.edu/~mpj/pubs/RR-1030.pdf),
  Research Report YALEU/DCS/RR-1030, Yale University (1994).  The
  architecture Cupid still has; [History](../explanation/history.md#gofer-and-hugs)
  says what it covers.

## Types and classes

- Mark P. Jones, *Qualified Types: Theory and Practice* (Cambridge
  University Press, 1994).  The theory behind the type checker's
  predicates and evidence; see [How types are
  inferred](../explanation/how-types-are-inferred.md).
- Mark P. Jones, *A system of constructor classes: overloading and
  implicit higher-order polymorphism* (FPCA 1993; Journal of
  Functional Programming, 1995).  Classes over type constructors,
  first implemented in Gofer, which `Functor` and `Monad` rest on.
- Mark P. Jones, *Simplifying and improving qualified types* (FPCA
  1995).  Improvement, which the solver uses for functional
  dependencies.
- Simon Peyton Jones, Mark Jones and Erik Meijer, *Type classes: an
  exploration of the design space* (Haskell Workshop, 1997).  The
  choices behind multi-parameter classes and flexible contexts and
  instances.
- Mark P. Jones, *Type Classes with Functional Dependencies* (ESOP
  2000).  Functional dependencies as Cupid implements them; see [How
  constraints are solved](../explanation/how-constraints-are-solved.md).
- Mark P. Jones, *Typing Haskell in Haskell* (Haskell Workshop, 1999).
  A Haskell 98 type checker written as a Haskell program, by the author
  of the one in Cupid.  Shorter than the C, and a useful companion to
  it.

## Compiling and running

- Simon L. Peyton Jones, *The Implementation of Functional Programming
  Languages* (Prentice Hall, 1987).  The G-machine, pattern-matching
  compilation and lambda lifting that [How code is compiled and
  run](../explanation/how-code-is-compiled-and-run.md) follows.
