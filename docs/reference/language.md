# The language Cupid accepts

What Cupid implements of Haskell 2010, where it deviates, and what it
adds.

## Summary

Cupid accepts Haskell 2010: the Haskell 98 language with hierarchical
module names, the foreign function interface, pattern guards, empty
data declarations, `then` and `else` at do-block indentation and
`LINE` pragmas.  The n+k patterns that Haskell 2010 removed are not
accepted.

It also accepts, without any option, the extensions in
[Language extensions](extensions.md).  Two of them are visible in
programs that use no extension at all: `forall` is a keyword, so it
cannot be used as a variable name, and a type synonym may appear in an
instance head.  Everything else in this page holds whether or not the
extensions are used.

The extensions of the 2006 release that were guarded by compile-time
flags are gone: TREX extensible records, implicit parameters, the
`observe` debugger, recursive do (`mdo`), parallel list comprehensions,
multi-instance resolution, here documents and restricted type synonyms
(see [the design](../explanation/design.md#deleting-instead-of-switching-off)).

## Known deviations from the Report

Listed by the section of the Haskell 2010 Report they concern.

### Lexical structure

**Identifiers are limited to Latin-1.**  Strings and comments may
contain any Unicode character, but an identifier may only use
characters from ISO 8859-1.

**Limited lookahead.**  The lexer is confused by `Just.if`, `0xy`,
`0oy`, `9e+y` and `9.0e+y`, because it does not look far enough ahead.

### Expressions

**Fixities are resolved after parsing.**  Cupid does not use operator
fixities during parsing, so it rejects legal but odd expressions such
as

```haskell
let x = True in x == x == True
```

### Declarations and bindings

**Polymorphic recursion is slightly more permissive.**  In a group of
mutually recursive bindings where one function has a signature, Cupid
treats the others as depending only on that signature, and so can
infer polymorphic types where the Report requires an explicit
signature:

```haskell
data BalancedTree a = Zero a | Succ (BalancedTree (a,a))

zig :: BalancedTree a -> a
zig (Zero a) = a
zig (Succ t) = fst (zag t)

zag (Zero a) = a
zag (Succ t) = snd (zig t)
```

Haskell 2010 rejects this without a signature for `zag`; Cupid accepts
it.

**Class methods may constrain the class variable.**  Contrary to
section 4.3.1 of the Report, Cupid allows

```haskell
class Foo a where
    op :: Num a => a -> a -> a
```

**The monomorphism restriction is applied per binding group.**  Cupid
rejects this example from section 4.5.5 of the Report:

```haskell
module M where
import Data.List
len1 = genericLength "Hello"
len2 = (2*len1) :: Rational
```

`len1` is defaulted to `Integer` as soon as its own group is inferred,
so the use at `Rational` in `len2` fails.  The Report defers defaulting
to the end of the module.  For the same reason a restricted binding
whose constraint cannot be defaulted is rejected even when a later use
fixes its type: `total = sum`, with no signature, reports
`Unresolved top-level overloading` with `Foldable` outstanding, because
the Prelude's `sum` works on any `Foldable` structure.  A type
signature avoids both.

### Modules

**A missing module header means `module Main where`,** exporting
everything, rather than `module Main (main) where`, since small test
files usually have no header.

**A missing export list is treated as `(module M)`** for the current
module `M`.  This differs from exporting every defined name only when
an imported module is aliased as `M`.

**Type synonyms may be written `T(..)`** in export lists and `T(..)` or
`T()` in import lists.

**Hiding a name that the module does not export is not an error,** as
in GHC, though the Report makes it one: `import Prelude hiding (catch)`,
in code written for a Prelude that exported `catch`, is accepted.

**Mutually recursive modules are not supported.**

**`(:)` is exported by the Prelude as if it were an identifier,** so
Cupid wrongly rejects

```haskell
module Foo where
import Prelude ()
cs = 'a':cs
```

### Predefined types and classes

**The monad classes are those of GHC's base, not of the Report.**
`Applicative` is a superclass of `Monad`, and `Functor` of
`Applicative`, so a `Monad` instance needs the other two; `return`
defaults to `pure`.  `fail` is the method of a class of its own,
`MonadFail`, and a `do` block uses it only for a generator whose
pattern can fail to match: `Just x <- m` needs `MonadFail`, while
`(x, y) <- m` and `~(Just x) <- m` need only `Monad`.  The Prelude
exports `Functor(fmap, (<$))`, `(<$>)`,
`Applicative(pure, (<*>), (*>), (<*), liftA2)`,
`Monad((>>=), (>>), return)` and `MonadFail(fail)`.  `MonadPlus` has
`Alternative` as its superclass, and `Control.Arrow`'s `Arrow` has
`Category`.  A Haskell 98 program that defines `Monad` without
`Applicative`, or `fail` in a `Monad` instance, is rejected, as it is
by GHC.

**The Prelude's list folds are those of `Foldable`,** and its `mapM`
and `sequence` those of `Traversable`, as in GHC's base: `length`,
`sum`, `elem`, `foldr`, `concat`, `mapM_` and the rest work on any
structure with an instance, such as `Maybe`, `Either e`, pairs, arrays
and the containers.  The Prelude exports the classes as
`Foldable(elem, foldMap, foldr, foldl, foldr1, foldl1, maximum, minimum,
product, sum, null, length)` and
`Traversable(traverse, sequenceA, mapM, sequence)`.  `Data.List`,
`Data.Foldable`, `Data.Traversable` and `Control.Monad` export the same
functions, so a module may import all of them unqualified.

**The Prelude does not export `catch`,** as GHC's has not since 7.6:
`Control.Exception.catch` catches any exception and
`System.IO.Error.catchIOError` only an `IOError`.

**`Monoid` has `Semigroup` as its superclass,** and `mappend`
defaults to `(<>)`; the Prelude exports `Semigroup((<>))` and
`Monoid(mempty, mappend, mconcat)`.  A `Monoid` instance therefore
needs a `Semigroup` instance for the same type.  `Text.PrettyPrint`
keeps its own, left-associative `(<>)` on documents, as the `pretty`
package does, so a module that imports it unqualified and uses `(<>)`
must hide one of the two.

**Rational literals may lose precision.**  A literal such as
`1.234e-5` is held as a double before conversion, so at type `Rational`
it may not be exact.

**Floating point values print differently.**  `show` on floating point
numbers uses an internal routine rather than `Numeric.showFloat`, and
the output can differ slightly.

**Tuple instances stop at size 7.**  `Eq`, `Ord`, `Bounded`, `Read` and
`Show` are defined for tuples of up to seven components; the Report
requires every tuple size.

**No file locking.**  Cupid does not enforce the single-writer,
multiple-reader rule on files.  A program that reads and writes the
same file at once may see inconsistent data instead of failing.

### Syntax reference

**Literate comments are not supported.**  Every file is read as
ordinary Haskell source, so a literate script (section 10.4), with `>`
lines or `\begin{code}` blocks, is rejected, and `.lhs` is not on the
default suffix list.  A line beginning with `#!` is read as a blank
line, so an executable script needs no literate wrapper.

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
ever-growing expansion.  Making the binding a function, so that it is
not updated, avoids the leak:

```haskell
main = return () >>= \_ -> mapM_ putStrLn (repeat "y")
```

## Defaulting at the prompt

An expression typed at the prompt is defaulted more liberally than a
top-level binding: an ambiguous type variable is defaulted when all
its constraints are standard classes and at least one is numeric or is
`Show`, `Eq` or `Ord`.  See [Interpreter commands](commands.md#expr).

## Foreign function interface

The FFI of Haskell 2010 is implemented with these limits:

- Calling conventions: `ccall` and `stdcall`.  Any other is an error.
- `foreign import` in its `static` form, and `foreign import
  "dynamic"` (calling through a `FunPtr`), work on all targets.
- `foreign import "wrapper"`, which turns a Haskell function into a C
  function pointer, needs architecture-specific machine code that the
  2006 release had only for 32-bit x86, PowerPC and SPARC; none of the
  targets this tree builds for are among them, and that code has been
  removed, so it fails at run time everywhere.
  See [Foreign import wrappers](../explanation/foreign-import-wrappers.md).
- `foreign export` declarations are accepted and `fficupid` generates C
  for them, but nothing in the tree exercises them and they should be
  considered untested.

A module containing foreign declarations must be compiled with
[fficupid](fficupid.md) before it can be loaded.

## Hierarchical modules

A module name may contain dots.  To find `A.B.C`, Cupid looks for
`A/B/C.hs` in each directory of the search path.  See
[Modules, files and the search path](../explanation/modules-and-the-search-path.md).

## See also

- [Language extensions](extensions.md)
- [The libraries](libraries.md)
