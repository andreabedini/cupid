# The language Cupid accepts

What to expect first, with links to the details.

## What it is

Cupid accepts Haskell 2010: the Haskell 98 language with hierarchical
module names, the foreign function interface, pattern guards, empty
data declarations and `then` and `else` at do-block indentation.  The
n+k patterns that Haskell 2010 removed are not accepted.

`LANGUAGE` pragmas are read as comments.  The [extensions](extensions.md)
Cupid has are always on, except overlapping instances, which need `+o`.  Two
of them are visible in programs that use no extension at all: `forall` is a
keyword, so it cannot be used as a variable name, and a type error in `f $ x`
is reported against `f x`.

Type families, generalised algebraic data types with refined result types
(the syntax is accepted, the refinement is not), Template Haskell and the
other features in [Not accepted](extensions.md#not-accepted) are not in Cupid.

The libraries are a subset of GHC's base, roughly version 4.18; see [The
libraries](libraries.md).

## What a program is most likely to meet

**The numbers.**  `Int` and `Word` are 32 bits wide on every target, and
`Float` and `Double` do not recognise NaN or infinity, so `isNaN` is always
`False`.  `show` on a floating point number prints 15 significant digits (7
for `Float`), so `show (0.1 + 0.2)` is `0.3`.  See [chapter
6](haskell-2010.md#6-predefined-types-and-classes).

**The classes are mostly GHC's, not the Report's.**  `Applicative` is a
superclass of `Monad`, `fail` belongs to `MonadFail` and `Semigroup` is a
superclass of `Monoid`, so a Haskell 2010 program that defines `Monad` alone, or
`fail` in a `Monad` instance, is rejected, as it is by GHC.  The Prelude's list
functions are those of `Foldable` and `Traversable`, and it doesn't export
`catch`.  Unlike GHC's, `Num` still has `Eq` and `Show` as superclasses.  See
[chapter 6](haskell-2010.md#6-predefined-types-and-classes) and [chapter
9](haskell-2010.md#9-standard-prelude).

**Tuple instances stop at five components,** and `Bounded` has none, where the Report asks for at least fifteen.  See [chapter
6](haskell-2010.md#6-predefined-types-and-classes).

**Modules.**  Mutually recursive modules are not supported, and a missing
module header means `module Main where`, exporting everything.  See [chapter
5](haskell-2010.md#5-modules) and [Modules, files and the search
path](../inside/modules-and-the-search-path.md).

**Source files.**  A literate script is read in the Bird-track form only (see
[chapter 10](haskell-2010.md#10-syntax-reference)), and a `#!` first line is read
as a blank line, so a script can be made executable.  Operator symbols are
limited to Latin-1, though identifiers may use any Unicode letter (see [chapter
2](haskell-2010.md#2-lexical-structure)).  A few legal or illegal forms are read
differently from the Report, because fixities are resolved after parsing (see
[chapter 3](haskell-2010.md#3-expressions)).

**Bindings.**  A binding caught by the monomorphism restriction, such as `n = 3`,
is defaulted when its own group is inferred, so a later use cannot fix its type;
a type signature avoids this.  See [chapter
4](haskell-2010.md#4-declarations-and-bindings).

**Foreign functions.**  A module with foreign declarations must be compiled with
[`cupid --ffi`](cupid.md) before it can be loaded.  `foreign import "wrapper"`
fails at run time and `foreign export` is not implemented.  See [chapter
8](haskell-2010.md#8-foreign-function-interface) and [Foreign
functions](../inside/foreign-functions.md).

**Missing library names.**  A few names are missing, such as `withFile`.  See
[Part II](haskell-2010.md#part-ii-the-libraries).

## Defaulting at the prompt

An expression typed at the prompt is defaulted more liberally than a
top-level binding: an ambiguous type variable is defaulted when all
its constraints are Haskell 2010 classes (`Eq`, `Ord`, `Show`, `Read`,
`Enum`, `Bounded`, `Ix` or a numeric class) and at least one is numeric
or is `Show`, `Eq` or `Ord`.  See [Interpreter commands](commands.md#expr).

## Known bugs

**Deep or unbounded recursion usually ends in an error, but not always.**
While a program is evaluating, an unbounded computation ends in `Control stack
overflow` or `Garbage collection fails to reclaim sufficient space`, or runs for
ever, and none we have tried crashes, under C stack limits from 128KB to unlimited.
At limits that small, loading fails with `Control stack overflow`: below about 128KB the
`Prelude` itself doesn't load, and at 128KB a module with larger imports may not.

While a program is being loaded, some recursion in C is unchecked, so a pathological
source file can overflow the C stack, which on Unix is a segmentation fault reported
as `Unexpected signal`.  The known cases are a type applied to about 70,000 arguments
and a list comprehension with 30,000 qualifiers, at the default 8MB limit, and a chain
of 30,000 definitions each using the next under a 256KB limit (which loads at 8MB).

**Top-level bindings leak.**  This runs in constant space at the prompt:

```haskell
mapM_ putStrLn (repeat "y")
```

but the same expression as the definition of `main` does not, because
`main` takes no arguments, so its value is kept, and that value is the
ever-growing expansion.  Putting the expansion under a lambda, so that
it is rebuilt rather than updated, avoids the leak:

```haskell
main = return () >>= \_ -> mapM_ putStrLn (repeat "y")
```

## Where the rest is

[Language extensions](extensions.md) has each feature beyond Haskell 2010 and what is
not accepted.  [Haskell 2010 conformance](haskell-2010.md) goes through the Report chapter
by chapter, libraries included, with the full statement of each deviation.  Together
with this page they are the authority on the language.  The extensions of Hugs's 2006
release that were guarded by compile-time flags are gone; [Differences from Hugs
98](differences-from-hugs.md#language) lists them.  For why the language is drawn where it
is, see [Why the language is what it is](../inside/language-design.md).
