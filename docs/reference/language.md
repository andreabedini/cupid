# The language Cupid accepts

What Cupid implements of Haskell 2010, where it deviates, and what it
adds.  This page names the deviations a program is most likely to
meet; [Haskell 2010 conformance](haskell-2010.md) states them in full,
going through the Report chapter by chapter, libraries included.

## Summary

Cupid accepts Haskell 2010: the Haskell 98 language with hierarchical
module names, the foreign function interface, pattern guards, empty
data declarations and `then` and `else` at do-block indentation.  The
n+k patterns that Haskell 2010 removed are not accepted.  `LINE`
pragmas are read as comments.

It also accepts, without any option, the extensions in
[Language extensions](extensions.md), except overlapping instances,
which need `+o`.  Two of them are visible in programs that use no
extension at all: `forall` is a keyword, so it cannot be used as a
variable name, and a type error in `f $ x` is reported against `f x`.
Everything else in this page holds whether or not the extensions are
used.

The extensions of the 2006 release that were guarded by compile-time
flags are gone: TREX extensible records, implicit parameters, the
`observe` debugger, recursive do (`mdo`), parallel list comprehensions,
multi-instance resolution and here documents (see
[the design](../explanation/design.md#deleting-instead-of-switching-off)).

## Known deviations from the Report

Listed by the section of the Haskell 2010 Report they concern.

### Lexical structure

[Chapter 2 of the conformance page](haskell-2010.md#2-lexical-structure)
has these in full.

**Operator symbols, spaces and digits are limited to Latin-1,** so `∘`
is `Unrecognised character`, though identifiers may use any Unicode
letter and strings and comments any Unicode character.

**Limited lookahead.**  `Just.if`, `9e+y` and `9.0e+y` are errors, where
the Report reads `Just . if`, `9 e + y` and `9.0 e + y`, because the
lexer does not look far enough ahead.

### Expressions

[Chapter 3 of the conformance page](haskell-2010.md#3-expressions) has
these in full.

**Fixities are resolved after parsing,** so two illegal forms get
through: prefix negation after an operator of precedence 6 or more,
`a + - b`, and a right section whose operand is an infix expression,
`(* 3 + 4)`, which is read as `(* (3 + 4))`.

### Declarations and bindings

[Chapter 4 of the conformance
page](haskell-2010.md#4-declarations-and-bindings) has these in full.

**Polymorphic recursion is slightly more permissive:** where one
function in a mutually recursive group has a signature, Cupid can infer
polymorphic types for the others where the Report requires a signature.

**Class methods may constrain the class variable,** contrary to section
4.3.1 of the Report: `class Foo a where op :: Num a => a -> a -> a`.

**The monomorphism restriction is applied per binding group:** a
restricted binding is defaulted when its own group is inferred, not at
the end of the module, so a later use cannot fix its type; a type
signature avoids this.

### Modules

[Chapter 5 of the conformance page](haskell-2010.md#5-modules) has
these in full.

**A missing module header means `module Main where`,** exporting
everything, rather than `module Main (main) where`.

**A missing export list is treated as `(module M)`** for the current
module `M`, which differs from exporting every defined name only when
an imported module is aliased as `M`.

**Type synonyms may be written `T(..)`** in export lists and `T(..)` or
`T()` in import lists.

**Hiding a name that the module does not export is not an error,** as
in GHC, though the Report makes it one.

**Mutually recursive modules are not supported.**

**`(:)` is exported by the Prelude as if it were an identifier,** so a
module that imports `Prelude ()` cannot use `:`.

### Predefined types and classes

[Chapter 6 of the conformance
page](haskell-2010.md#6-predefined-types-and-classes) has these in
full, except where an entry links elsewhere.

**The monad classes are those of GHC's base, not of the Report:**
`Applicative` is a superclass of `Monad`, `Functor` of `Applicative`,
and `fail` is the method of a class of its own, `MonadFail`, so a
Haskell 2010 program that defines `Monad` without `Applicative`, or
`fail` in a `Monad` instance, is rejected, as it is by GHC.

**The Prelude's list folds are those of `Foldable`,** and its `mapM`
and `sequence` those of `Traversable`, as in GHC's base, so `length`,
`sum`, `elem` and the rest work on any structure with an instance (see
[chapter 9](haskell-2010.md#9-standard-prelude)).

**The Prelude does not export `catch`,** as GHC's has not since 7.6;
use `Control.Exception.catch` or `System.IO.Error.catchIOError` (see
[chapter 9](haskell-2010.md#9-standard-prelude)).

**`Monoid` has `Semigroup` as its superclass,** so a `Monoid` instance
needs a `Semigroup` instance for the same type.

**Rational literals may lose precision,** since a literal is held as a
double before conversion: `1e400 :: Rational` is `0 % 1`.

**Floating point values print differently:** `show` uses C's `%.15g`
(`%.7g` for `Float`) rather than `Numeric.showFloat`, so
`show (0.1 + 0.2)` is `0.3` and the output does not always read back to
the same number.

**`Float` and `Double` do not recognise NaN or infinity:** `isNaN`,
`isInfinite` and the other `RealFloat` predicates always return `False`.

**Tuple instances stop at size 5,** and `Bounded` has none, where the
Report requires them up to at least fifteen.

**No file locking.**  Cupid does not enforce the single-writer,
multiple-reader rule on files (see
[chapter 7](haskell-2010.md#7-basic-inputoutput)).

### Syntax reference

[Chapter 10 of the conformance
page](haskell-2010.md#10-syntax-reference) has this in full.

**Literate scripts are read in the Bird-track form only:** in a file
whose name ends in `.lhs` the program is the lines that begin with `>`,
and a script that puts it between `\begin{code}` and `\end{code}` is
rejected.

## Other known bugs

**Infinite computations can crash.**  An unbounded computation usually
ends with `Garbage collection fails to reclaim sufficient space` or
`Control stack overflow`, or runs for ever.  Depending on the sizes of
the heap, the Cupid stack and the C stack, it can instead overflow the C
stack, which on Unix is a segmentation fault.

**Top-level bindings leak.**  This runs in constant space at the
prompt:

```haskell
mapM_ putStrLn (repeat "y")
```

but the same expression as the definition of `main` does not, because
`main` is a constant applicative form and is updated with its
ever-growing expansion.  Putting the expansion under a lambda, so that
it is rebuilt rather than updated, avoids the leak:

```haskell
main = return () >>= \_ -> mapM_ putStrLn (repeat "y")
```

## Defaulting at the prompt

An expression typed at the prompt is defaulted more liberally than a
top-level binding: an ambiguous type variable is defaulted when all
its constraints are Haskell 2010 classes (`Eq`, `Ord`, `Show`, `Read`,
`Enum`, `Bounded`, `Ix` or a numeric class) and at least one is numeric
or is `Show`, `Eq` or `Ord`.  See [Interpreter commands](commands.md#expr).

## Foreign function interface

The FFI of Haskell 2010 is implemented with these limits, which
[chapter 8 of the conformance
page](haskell-2010.md#8-foreign-function-interface) has in full:

- Calling conventions: `ccall`, and `stdcall` on Windows only.  Any
  other is an error.
- `foreign import` in its `static` form, and `foreign import
  "dynamic"` (calling through a `FunPtr`), work on all targets.
- `foreign import "wrapper"`, which turns a Haskell function into a C
  function pointer, fails at run time on every target this tree builds
  for.  See [Foreign import wrappers](../explanation/foreign-import-wrappers.md).
- `foreign export` declarations parse, and `cupid --ffi` writes and
  compiles C for them, but it and loading the module both fail with
  `Foreign export not implemented yet.`

A module containing foreign declarations must be compiled with
[`cupid --ffi`](cupid.md) before it can be loaded.

## Hierarchical modules

A module name may contain dots.  To find `A.B.C`, Cupid looks for
`A/B/C.hs` or `A/B/C.lhs` in each directory of the search path.  See
[Modules, files and the search path](../explanation/modules-and-the-search-path.md).

## See also

- [Haskell 2010 conformance](haskell-2010.md)
- [Language extensions](extensions.md)
- [The libraries](libraries.md)
- [Differences from Hugs 98](differences-from-hugs.md)
