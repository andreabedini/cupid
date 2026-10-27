# Haskell 2010 conformance

How Cupid measures up against the Haskell 2010 Report, chapter by
chapter.  [The language Cupid accepts](language.md) is the short
account; this page is the long one, and it is the page to update when
a deviation is fixed or found.

Every statement here was checked by running a program against the
interpreter built from this tree on 2026-10-16, on x86-64 Linux, unless
it says otherwise.  Section numbers are the Report's.  Where the Report
leaves something to the implementation, the page says what Cupid does
and marks it *unspecified*.  What Cupid accepts beyond the Report, the
extensions, is in [Language extensions](extensions.md) and is mentioned
here only where it makes a program the Report rejects acceptable.

## Summary

Cupid implements Haskell 2010: the language of Part I, the Prelude of
chapter 9 and the libraries of Part II.  Most programs written to the
Report load and run.  These are the exceptions a program is likely to
meet, each explained in its chapter below:

- Literate scripts are not read.
- `Int` and `Word` are 32 bits wide, on every target.
- The Prelude's classes are those of GHC's base, not the Report's:
  `Applicative` sits between `Functor` and `Monad`, `fail` belongs to
  `MonadFail`, `Semigroup` is a superclass of `Monoid`, the list
  functions are `Foldable`'s and `Traversable`'s, and `catch` is not
  exported.
- The derived and built-in instances for tuples stop at five
  components, and there are no `Bounded` instances for tuples at all;
  the Report asks for at least fifteen.
- `Float` and `Double` do not know NaN or infinity: `isNaN`,
  `isInfinite` and the other `RealFloat` predicates are always `False`,
  and `show` prints `inf` and `-nan`.  `show` on a floating point number
  prints 15 significant digits, not the shortest representation that
  reads back, and conversions near the ends of the range of `Double`
  are inexact or fail.
- Fixity is resolved after parsing, which lets a few illegal forms
  through: prefix negation after an operator, as in `a + - b`, and a
  right section whose operand is an infix expression, as in `(* 3 + 4)`.
- A few names are missing from the libraries: `withFile`,
  `hSetFileSize` and `hShow` in `System.IO`, `catch` and `try` in
  `System.IO.Error`, `finalizeForeignPtr`, `unsafeLocalState`, the
  `throwErrnoPath` family and the `CUChar`/`CSChar` casts of
  `Foreign.C.String`.
- `foreign export` is not implemented, `foreign import "wrapper"` fails
  at run time, and `stdcall` is accepted only on Windows.
- The monomorphism restriction defaults a binding when its own group is
  inferred, not at the end of the module, so a later use cannot fix the
  type.

## Part I: the language

### 1 Introduction

Conforms.  The namespaces of 1.4 are kept: a module that declares both
`class T` and `data T` is rejected, and a type and a class of one name
imported from different modules make the name ambiguous (chapter 5).

### 2 Lexical structure

Conforms, with the Unicode limits below.  The reserved identifiers and
operators are all reserved, `as`, `qualified`, `hiding`, `safe`,
`unsafe`, `ccall` and `stdcall` are ordinary identifiers, nested
comments nest, `-->` and `|--` are operators and not comments, every
escape of 2.6 is read as the Report says (`\SOH`, `\SO\&H`, `\^@`, the
ASCII names, numeric escapes up to `\1114111`, string gaps), and the
numeric literals of 2.5 lex as the Report says: `1e` is `1 e`, `.5` is
`. 5`, `1.` is `1 .`, `0xy` is `0 xy`, and `-1` is negation applied to
`1`.  Qualified names lex by maximal munch, so `[False..]` is `[` then
the qualified operator `False..` and a syntax error, as in the Report;
write `[False ..]`.

**Identifiers take any Unicode letter; symbols, spaces and digits do
not.**  `λ`, `π`, `日本` and `Λόγος` are identifiers, as 2.4 says.  But
the Report's `uniSymbol`, `uniWhite` and `uniDigit` stop at Latin-1 in
Cupid: `∘` or `→` cannot form an operator, `¬`, `·` and `×` can; an
EN SPACE (U+2003) or IDEOGRAPHIC SPACE (U+3000) between tokens is
`Unrecognised character`, a NO-BREAK SPACE (U+00A0) is fine; and `١٢`
is not a numeric literal, though `x١` is an identifier.  A byte order
mark at the start of a file is rejected, as the Report's grammar has
it.

**Limited lookahead.**  `Just.if` is mis-lexed (the Report reads
`Just . if`), and `9e+y` and `9.0e+y` fail with `Missing digits in
exponent` where the Report reads `9 e + y`.  `0xy` and `0oy` now lex
correctly.

**`forall` is a keyword** and cannot be a variable, for the sake of the
extensions.

**A reserved word after a qualifier** is lexed as a qualified variable:
`Prelude.where` is `Undefined qualified variable`, where the Report
would lex `Prelude . where`.  Both are errors.

### 3 Expressions

Conforms, with the exceptions below, which are all consequences of
resolving fixity after parsing (3.4, 3.5) or edge cases.  The
translations of 3.2 refer to the Prelude's `fromInteger`, `negate`,
`enumFromTo`, `concatMap`, `>>=` and `==` whatever is in scope; the
Report's examples of 3.17.2 all give the answers it lists, including
the irrefutable, newtype and `seq` cases; `case`, `do`, `let`, list
comprehensions, arithmetic sequences, field labels and expression
signatures behave as described, and the error messages are the
compile-time ones the Report asks for where it asks for them.  `C {}`
is legal for any constructor without strict fields, labelled or not,
and an error that names the omitted field when one is strict (3.15.2).

**Prefix negation is accepted as the right operand of an operator of
precedence 6 or higher** (3.4, 10.6).  `1 + - 1`, `a - - b`, `2 * - 3`
and `(+ - 3)` are accepted and mean `1 + (-1)` and so on; the Report
makes each a syntax error.  `1 == - 1` and `[- 1, - 2]` are legal and
work.

**Right sections are not checked against fixity** (3.5).  `(* 3 + 4) 2`
is accepted and evaluates to `2 * (3 + 4)`, as are `(+ 1 + 2)` and
`` (`div` 2 + 1) ``; the Report allows `(op e)` only when `x op e`
parses as `x op (e)`.  Left sections are checked: `(1 + 2 *)` is
rejected with `Operator "(*)" in a left section does not apply to the
whole operand`.

**A guard in a `case` alternative that ends in an unparenthesised type
signature is rejected** (3.13).  The Report's `| let b = not a in b ::
Bool -> a` is a syntax error there; the Report notes that a
limited-lookahead parser may reject it.  Parenthesise the signature.
In a function's guard it is accepted.

**A failed pattern bind in `IO` carries no location.**  `Just x <-
return Nothing` fails with `user error (match fails)`; the Report
prefers a message that names the position (3.14).

**`fail` is used only for refutable patterns** (3.14), and comes from
`MonadFail`: `(x, y) <- m` and `~(Just x) <- m` need `Monad` alone,
`Just x <- m` needs `MonadFail`.  A Haskell 2010 `Monad` instance that
defines `fail` is rejected (see chapter 6).

**Tuples** of any size can be built and matched, but have class
instances only up to five components (see chapter 6).

**n+k patterns** are not accepted (3.17), as Haskell 2010 removed them,
and neither is anything else Haskell 2010 does not have: `f !x = x`
defines the operator `!`, as the Report says it should.

Unspecified: `error` and `undefined` print `Program error: message`
and `Program error: Prelude.undefined` and exit with status 1; they
can be caught with `Control.Exception`, which the Report does not
provide.

### 4 Declarations and bindings

Conforms, with the deviations below.  Kind inference (4.6) defaults an
unconstrained kind to `*` and rejects `type FunnyTree = Tree []`; a
synonym must be fully applied; a datatype context constrains the
constructor's type as 4.2.1 says; a newtype behaves exactly as the
`d1`/`d2`/`s`/`n` example of 4.2.3; class declarations obey 4.3.1
(a method signature must mention the class variable, type signatures
and fixity declarations are not permitted in instances, a cyclic
superclass relation is an error, a missing method is bottom); instance
declarations obey 4.3.2 (the context is never inferred, a missing
superclass instance is an error, a second instance is an error); the
defaulting rules of 4.3.4 are applied exactly (`show (read "1")` and
`print []` are ambiguous in a module, `default (Bool)` is rejected, a
class outside the standard libraries blocks defaulting); fixity
declarations follow 4.4.2, in `let` and `where` as well as at top
level; a pattern binding that binds no variables, such as `1 = 2`, is
accepted and has no effect (4.4.3.2); and the dependency analysis and
generalisation of 4.5 give the types the Report's examples give.

**Record construction and update ignore strictness flags** (4.2.1).
For `data R = R { a :: !Int, b :: Int }`, `R { a = undefined, b = 1 }`
and `r { a = undefined }` are values; only the positional `R undefined
1` is bottom.  The Report says the strict field is evaluated in every
form of construction.

**Fixity is not resolved on the left-hand side of an infix binding**
(4.4.3.1).  The Report's example `a ## b : xs = e`, with `##` at a
precedence below 5, "would be illegal"; Cupid accepts it, with `b : xs`
as the second argument pattern whatever the fixity of `##`.

**Deriving infers contexts the Report calls a static error** (4.5.3).
`data Apply a b = App (a b) deriving Show` is accepted and gets the
context `Show (a b)`, which Haskell 2010 does not allow in a context.
This is the flexible-contexts extension at work.

**Polymorphic recursion is slightly more permissive** (4.5.1).  In a
group of mutually recursive bindings where one function has a
signature, the others depend only on that signature, so

```haskell
data BalancedTree a = Zero a | Succ (BalancedTree (a,a))

zig :: BalancedTree a -> a
zig (Zero a) = a
zig (Succ t) = fst (zag t)

zag (Zero a) = a
zag (Succ t) = snd (zig t)
```

is accepted without a signature for `zag`, which the Report requires.

**A class method may constrain the class variable** (4.3.1).
`class Foo a where op :: Num a => a -> a -> a` is accepted; the Report
forbids a constraint on `a` there.

**The monomorphism restriction is applied per binding group** (4.5.5).
Rule 2 of the Report defers defaulting of a restricted binding to the
end of the module, so that a later use can fix its type.  Cupid
defaults when the binding's own group is inferred.  The Report's
example

```haskell
module M where
import Data.List
len1 = genericLength "Hello"
len2 = (2*len1) :: Rational
```

is rejected: `len1` is defaulted to `Integer` and the use at `Rational`
is a type error.  So are `x = 3; y = x + 0.5` and the `[(n,s)] =
reads t` example of 4.5.5, with `Outstanding context: Fractional
Integer`.  A restricted binding whose constraint cannot be defaulted
fails even if a later use would fix it: `total = sum` reports
`Unresolved top-level overloading` with `Foldable` outstanding.  A
type signature avoids all of these.

**Accepted though the Report rejects them**, because the extensions are
always on (overlapping instances excepted; they need `+o`): a context on
a type that is not a type variable (`Eq Int => ...`), an instance for a
synonym, a type with an operator name, an instance whose head is not a
constructor applied to distinct variables.  See [Language
extensions](extensions.md).

### 5 Modules

Conforms, with the deviations below.  Every export form of 5.2 and
every import form of 5.3 does what the Report says, including the
subtle ones: `module M` exports only names in scope both unqualified
and as `M.x`; an export of two different entities under one name, or
of a lone data constructor, is an error; `hiding (C)` hides a data
constructor as well as a type; the 5.5.2 example with `sin` is an
error only at an unqualified use; types and classes share a namespace,
so a type `T` and a class `T` imported from different modules make an
unqualified `T` ambiguous, in a type, a context, an instance head or an
export list alike; a module may export a function whose
type it cannot name (5.5.3); and the Prelude rules of 5.6 hold, down to
literals using `Prelude.fromInteger` whatever is in scope and the
Prelude's instances being impossible to hide.

**`import qualified M hiding (...)` imports nothing.**  The two rows of
the table in 5.3.4 that combine `qualified` and `hiding` should bring
`M.x` and `M.y`, or `M.y`, into scope; in Cupid they bring nothing,
and `M.y` is `Undefined qualified variable`.  The other thirteen rows
are right.

**`Main` need not export `main`** (5.1).  `module Main () where` and
`module Main (helper) where` run.  The Report requires `main` to be
exported.

**`main` without a signature is not fixed to `IO`.**  `main = return ()`
with no signature is rejected with `Unresolved top-level overloading`,
since the monomorphism restriction leaves `Monad m` unresolved and the
rule that `main` has type `IO t` is not used to resolve it.  Write
`main :: IO ()`.

**`cupid` runs `main >> return ()` in `Main`'s scope.**  A `Main`
module that imports the Prelude qualified, hides `>>` or `return`, or
imports only some Prelude names, loads and evaluates `main` at the
prompt but fails when `cupid` runs the file, with
`Undefined variable ">>"`.  The
Report's `import qualified Prelude` style of 5.3.2 is therefore
unusable in `Main`.

**`module M` in an export list, when `M` is imported only under an
alias**, is `Unknown module`.  The Report allows it and has it export
nothing.

**A missing module header means `module Main where`,** exporting
everything, rather than `module Main (main) where`, since small test
files usually have no header.

**A missing export list is treated as `(module M)`** for the current
module `M`.  This differs from exporting every defined name only when
an imported module is aliased as `M`.

**Type synonyms may be written `T(..)`** in export lists and `T(..)` or
`T()` in import lists.

**Hiding a name that the module does not export is not an error,** as
in GHC, though the Report makes it one: `import Prelude hiding (catch)`
is accepted.

**Mutually recursive modules are not supported:** `Recursive import
dependency` is reported.

**`(:)` is exported by the Prelude as if it were an identifier,** so
Cupid wrongly rejects

```haskell
module Foo where
import Prelude ()
cs = 'a':cs
```

with `Undefined data constructor ":"`, in patterns as well as
expressions.  The Report makes `:` special syntax that is always in
scope.

Unspecified: a module is found by its name, `A.B.C` in `A/B/C.hs` on
the search path, so a file whose header names a different module
cannot be imported; `cupid` run on such a file reports that it
cannot find a `Main` module.

### 6 Predefined types and classes

The types of 6.1 have the instances 6.1 requires, apart from tuples;
`seq` and `$!` follow the equations of 6.2 exactly; the default methods
of every class in 6.3 behave as the Report's (an `Eq` instance needs
only one of `==` and `/=`, `max` returns its second argument on a tie,
`Integral` with only `quotRem` and `toInteger` gets the right `div` and
`mod`); `Enum` sequences stop at the bounds of `Int` and follow
`numericEnumFromThenTo` for `Float` and `Double`; `Integer` is
unbounded; `div`, `mod`, `quot` and `rem` have the Report's signs; `^`
with a negative exponent, division by zero and `gcd 0 0` raise errors;
`round` rounds halves to even; and the `RealFloat` methods other than
the predicates (`decodeFloat`, `encodeFloat`, `floatRange`,
`significand`, `scaleFloat`) are right for finite values.

**The class hierarchy is that of GHC's base** (6.3).  `Applicative` is
a superclass of `Monad`, and `Functor` of `Applicative`, so a `Monad`
instance needs the other two, and `return` defaults to `pure`.  `fail`
is the method of its own class, `MonadFail`.  `Semigroup` is a
superclass of `Monoid`.  The list functions are those of `Foldable` and
`Traversable` (see chapter 9).  A Haskell 2010 program that defines
`Monad` without `Applicative`, or `fail` in a `Monad` instance, is
rejected, as it is by GHC.  `Functor` has the extra method `(<$)`.
`Num` keeps the Report's superclasses `Eq` and `Show`, which GHC has
since dropped.

**`Num`, `Integral` and `Fractional` have hidden methods,** `fromInt`,
`toInt` and `fromDouble`.  They are not exported, but an instance may
define them, which the Report rejects.

**`Int` and `Word` are 32 bits** (6.4).  The Report asks for at least
30.  Arithmetic wraps on overflow, `maxBound + 1 == minBound`, which
the Report leaves to the implementation; `div`, `quot`, `divMod` and
`quotRem` of `minBound` by `-1` raise `arithmetic overflow`.

**Tuple instances stop at five components, and `Bounded` has none.**
6.1.4 says every tuple type is an instance of `Eq`, `Ord`, `Bounded`,
`Read` and `Show`, and the Report asks for at least size 15.  Cupid
has `Eq`, `Ord`, `Read`, `Show` and `Ix` for sizes 2 to 5 only, and
`minBound :: (Bool, Bool)` is `Unresolved overloading`.  A tuple of any
size can still be built and matched.  `read "((1,2))"` also fails,
though the Report's tuple `Read` accepts redundant parentheses.

**`Float` and `Double` do not recognise NaN or infinity.**  `isNaN`,
`isInfinite`, `isDenormalized`, `isNegativeZero` and `isIEEE` always
return `False`.  As a result the Report's `atan2` gives the wrong
answer in every case that depends on negative zero: `atan2 (-0.0) (-1)`
is `pi`, not `-pi`.  `decodeFloat` on an infinity returns a corrupt
`Integer`, which prints as `(./,),(-*,(,-53)`, so `toRational`,
`truncate`, `floor` and `significand` on an infinity are wrong too,
and `realToFrac (1/0 :: Double) :: Float` never returns.  NaN compares
`EQ` with every number, so `compare (0/0) 1` is `EQ` and `[0/0 .. 1]`
is infinite.

**`show` on `Float` and `Double` is not the Report's.**  The Report's
`show` is `showFloat`: the shortest digits that read back to the same
number, in fixed notation for 0.1 ≤ x < 10^7 and as `1.0e-2` otherwise.
Cupid's prints 15 significant digits for `Double` and 7 for `Float`, in
C's style, so `show (0.1 + 0.2)` is `0.3`, `show (1/3 :: Float)` is
`0.3333333`, and `read (show x) == x` fails for such values.  Fixed
notation is used from 10^-4 up to 10^15 for `Double` and to 10^7 for
`Float`, and the exponent is written with a sign and two digits:
`1.0e-05`, `1.0e+22`.  Infinity and NaN show as `inf`, `-inf`, `nan`
and `-nan`, which `read` does not accept; it accepts `Infinity` and
`NaN`.  `Numeric.showFloat` gives the Report's output.

**Negative numbers are parenthesised at precedence 6.**  `showsPrec 6
(-1) ""` is `(-1)`; the Report's `showSigned` adds parentheses only
above 6.  `-0.0` is never parenthesised: `show (Just (-0.0))` is
`Just -0.0`.

**Large and tiny values do not convert exactly** (6.4.6).
`fromInteger` to `Double` is inexact near the top of the range, and
`fromInteger (2^1024)` gives the largest finite double, not infinity.
`fromRational` and `read` at `Double` turn denormal results into `0.0`,
and raise `arithmetic overflow` when the result is out of range or
even near it: `read "2.2250738585072014e-308" :: Double` fails.
Literals take another path and are correctly rounded.

**Rational literals pass through `Double`.**  `1.234e-5 :: Rational`
is exact, but a literal with more digits than a double holds loses
them, `3.14159265358979323846 :: Rational` is `3141592653589793 %
1000000000000000`, and one outside the range of a double becomes
`0 % 1`: `1e400 :: Rational` is zero.

**`**`, `log` and `sqrt` raise errors outside their domain.**
`(-2) ** 2`, `0 ** 0`, `log 0` and `sqrt (-1)` raise `argument out of
range`, where the Report's default `x ** y = exp (log x * y)` would give
a NaN or an infinity; the Report leaves such cases to the
implementation.  `tanh 1000` is `-nan` rather than `1.0`.

**Instances the Report does not have:** `Functor`, `Applicative` and
`Monad` for `Either e`, `(->) r` and `(,) a`; `Foldable` and
`Traversable` for `Maybe`, `Either e` and pairs; `Semigroup` and
`Monoid` for `Ordering`, `Maybe`, `IO`, functions and tuples.

Unspecified: `fromInteger` to `Int` reduces modulo 2^32, so
`read "9223372036854775808" :: Int` is `0`; `decodeFloat 0` is
`(0,-53)`.

### 7 Basic input/output

Conforms.  The examples of 7.1 to 7.3 run as printed, with
`catchIOError` for the Report's `catch` (see chapter 42).  `getContents`
and `interact` are lazy; `readIO` and `readLn` fail with `user error
(PreludeIO.readIO: no parse)`; `fail` in `IO` is `ioError . userError`;
`getLine` at end of file raises an error that `isEOFError` recognises;
`putStr` output is flushed at exit; an uncaught `IOError` or `error`
prints `Program error:` and the error and exits with status 1.

**No file locking.**  Cupid does not enforce the single-writer,
multiple-reader rule of 41.1.  A program that opens for writing a file
another handle is reading succeeds where the Report requires
`isAlreadyInUseError`, and a lazily read file that is then overwritten
yields the new contents.

### 8 Foreign function interface

The FFI of chapter 8 is implemented for imports, with the limits
below.  A module with `foreign` declarations must be compiled with
[`cupid --ffi`](cupid.md) before it can be loaded.  Every form of
`foreign import` in 8.4 is accepted and works: `static` with and
without a header, `&` address imports, `dynamic`, `safe` and `unsafe`,
an omitted entity string, and the marshallable types of 8.6 including
newtypes of them and `IO ()`; a non-marshallable type such as `String`
or `Integer` is `Illegal foreign type`, a static error as the Report
requires.  The 8.4.3 example of an import named `sin` behaves as
described.

**`foreign export` is not implemented.**  A declaration parses, and
`cupid --ffi` writes its C stub, but loading the module fails with
`Foreign export not implemented yet.`  The entity string also cannot
be omitted in an export, which the Report allows.

**`foreign import "wrapper"` fails at run time** on every target this
tree builds for, with `Foreign import wrapper is not supported on this
architecture`.  The 2006 release had the machine code it needs only
for 32-bit x86, PowerPC and SPARC; see
[Foreign import wrappers](../explanation/foreign-import-wrappers.md).
`freeHaskellFunPtr`, which only such a pointer could be passed to, is
a segmentation fault for any argument, `nullFunPtr` included.

**Calling conventions.**  `ccall` is supported everywhere, `stdcall` on
Windows only; elsewhere it and `cplusplus`, `jvm` and `dotnet` are
rejected with `calling convention "stdcall" not supported`, a static
error, which the Report permits.

**`Word` is not a foreign type.**  It is not in the list of 8.6, so
Cupid's rejection follows the letter of the Report; GHC accepts it.

**A newtype exported without its constructor is accepted as a foreign
type** in another module.  The Report requires the constructor to be
in scope where the type is used in a foreign declaration (8.4.2).

**The Report's `"errno.h &errno"` example does not compile:** the
generated stub undefines the `errno` macro before taking its address,
and glibc's `errno` is a macro.

**`HsFFI.h` is incomplete** (8.7, read from the installed header):
`HS_FLOAT_ROUND` is spelled `HS_FLOAT_ROUNDS`, and `hs_init`, `hs_exit`
and `hs_set_argv` are not declared, since nothing can be exported for
C to call.

Unspecified: `HsInt` is `int`, matching the 32-bit `Int`.

### 9 Standard Prelude

The Prelude exports every name of the Report except `catch`.  Every
function the Report defines in code was checked against that code and
agrees with it, from `lines "a\n\nb"` and `words "  a  b "` to the
laziness of `foldr`, `take 0 undefined` and `splitAt` on a partial
list, the precedences of the fixity table, and the messages of
`Prelude.!!: negative index`, `Prelude.foldl1: empty list`,
`Prelude.cycle: empty list` and the rest.  `lex` gives the Report's
results, including the limitations the Report itself lists.

**These functions take any `Foldable` or `Traversable` structure,** as
in GHC's base, rather than a list: `all`, `any`, `and`, `or`, `concat`,
`concatMap`, `elem`, `notElem`, `foldl`, `foldr`, `foldl1`, `foldr1`,
`length`, `null`, `maximum`, `minimum`, `sum`, `product`, `mapM_` and
`sequence_` (`Foldable`), and `mapM` and `sequence` (`Traversable`).
They work on `Maybe`, `Either e`, pairs, arrays and the containers as
well as lists.  `Data.List`, `Data.Foldable`, `Data.Traversable` and
`Control.Monad` export the same functions, so a module may import all
of them unqualified.  A program in which the type is not fixed may
become ambiguous, as `total = sum` does (chapter 4).

**`catch` is not exported,** as GHC's Prelude has not since 7.6.
`Control.Exception.catch` catches any exception and
`System.IO.Error.catchIOError` only an `IOError`.

**The Prelude also exports** the classes `Applicative`, `MonadFail`,
`Semigroup`, `Monoid`, `Foldable` and `Traversable` with their
methods, `(<$>)`, and the type `Word`.

**A few messages differ from the Report's code.**  `head []` and its
kin report `pattern match failure: head []`, where the Report's code
says `Prelude.head: empty list`; `readIO` says `PreludeIO.readIO: no
parse`.  A pattern-match failure in `succ ()` or a local function
names an internal variable, such as `v49_v2098`.

**`lex` differs from the Report's code in two small ways.**  `lex "_x"`
is `[("_x","")]`, where the Report's `isSingle` makes `_` a lexeme of
its own; and a string gap is removed from a string lexeme rather than
turned into `\&`.

**The control stack is small.**  `foldl (+) 0 [1..100000]` and
`foldr (+) 0 [1..100000]` end with `Control stack overflow`, which
cannot be caught; `sum` and `length` over a million elements work.

Unspecified: `error s` prints `Program error: s` and exits with status
1; lazy output and an error interleave, so `print [1, error "x"]`
prints `[1,` first.

### 10 Syntax reference

**Layout** (10.3) is implemented as the algorithm in the Report,
including the parse-error rule that closes an implicit block, so the
one-line `let x = 1 in x`, `(case x of 1 -> 2)` and `f x = case x of y
| y > 0 -> 1; _ -> 0` all parse; tabs are 8-column stops; a Unicode
character has width 1; `then` and `else` may sit at the indentation of
the `do` statements.  One departure:

**A `do` block may start at the indentation of the enclosing context.**
`main = do` with `print 1` in column 1, or `id $ do` followed by a
statement at the outer statements' column, is accepted; the Report's
algorithm inserts `{}` and the empty `do` is a parse error.  This is
GHC's `NondecreasingIndentation`, applied only to `do`; `of` and `let`
at the same indentation are rejected as the Report says.

**Literate comments** (10.4) are not supported.  Every file is read as
ordinary Haskell source, so a literate script, with `>` lines or
`\begin{code}` blocks, is rejected, and `.lhs` is not on the suffix
list.  A `#!` first line is read as a blank line, so an
executable script needs no literate wrapper.

**Fixity resolution** (10.6) is done after parsing, on the tree the
parser builds, and is right for every legal program: `- x ^ 2` is
`-(x ^ 2)`, undeclared operators are `infixl 9`, mixing operators of
equal precedence and different associativity is `Ambiguous use of
operator`, and so is a chain of two non-associative ones.  The two
illegal forms it lets through, prefix negation after an operator of
precedence 6 or higher and a right section with an infix operand, are
in chapter 3; the left-hand side of an infix binding is in chapter 4.

### 11 Specification of derived instances

Conforms, with the `Show` and `Read` corners below.  Derived `Eq`,
`Ord`, `Enum`, `Bounded`, `Ix`, `Show` and `Read` give the results the
Report's examples give: the `Tree` of figure 11.1 shows and reads as
printed, `infixr 4 :$` shows `1 :$ (2 :$ NT)` and `read` does not
accept `1 :$ 2 :$ NT`, both operands of an infix constructor are shown
at precedence one above the constructor's whatever its associativity,
records show as `R {f = 1, g = True}` with fields at precedence 0 and
read back with any spacing, `showsPrec 11 (Just 1) ""` is `(Just 1)`,
negative fields are parenthesised.  `Ix` is derivable for enumerations
and single-constructor types, as chapter 19 requires.

**An operator constructor declared with record syntax shows as a bare
operator.**  `data I = (:+:) { lf, rf :: Int } deriving (Show, Read)`
shows `1 :+: 2` as `:+: {lf = 1, rf = 2}`, which is not a Haskell
expression, and the derived `Read` accepts that and rejects the legal
`(:+:) {lf = 1, rf = 2}`.

**A backquoted infix constructor with a fixity declaration cannot be
read back.**  `data T = Int \`Pair\` Int deriving (Show, Read)` with
`infixl 5 \`Pair\`` shows `` 1 `Pair` 2 ``, as the Report says, but the
derived `Read` parses neither that nor `Pair 1 2`.  Without a fixity
declaration the constructor shows and reads prefix.

**Negative numbers are parenthesised at precedence 6.**  `showsPrec 6
(-1) ""` is `(-1)`; the Report's `showSigned` adds parentheses only
above 6.  So a derived `Show` for a constructor of precedence 5 gives
`(-1) :$ NT` where the Report's gives `-1 :$ NT`.  `read` accepts
both.

**A record construction is not parenthesised at precedence 11.**
`show (Just (R 1 True))` is `Just R {f = 1, g = True}`.  This follows
the Report's words, since a record construction is an atomic
expression, and the derived `Read` reads it, but GHC parenthesises.

Unspecified: `succ` and `pred` past the ends of a derived `Enum`
report `toEnum: out of range`.

### 12 Compiler pragmas

`INLINE`, `NOINLINE`, `SPECIALIZE` (and `SPECIALISE`), `LANGUAGE`,
`OPTIONS_GHC` and any pragma Cupid does not know are read as comments.

**An unsupported `LANGUAGE` feature does not fail.**  12.3 says a
program that asks for a feature the implementation does not support
must fail with an error; `{-# LANGUAGE TemplateHaskell #-}` is
accepted and ignored.

**`LINE` pragmas are ignored.**  They are not part of chapter 12, but
`{-# LINE 100 "foo.hs" #-}` has no effect on the position in an error
message.  See
[Preprocess source files before loading](../how-to/preprocess-sources.md).

## Part II: the libraries

Each module of Part II exists, with every name the Report lists except
those in the first table, with the Report's type except where the
second table says.  What Cupid's modules export beyond the Report,
which is most of what GHC's base has added since, is listed in [The
libraries](libraries.md).

| Module | Missing names |
|---|---|
| `System.IO` (41) | `withFile`, `hSetFileSize`, `hShow` |
| `System.IO.Error` (42) | `catch`, `try`: use `catchIOError` and `tryIOError` |
| `Foreign.Marshal` (30) | `unsafeLocalState` |
| `Foreign.ForeignPtr` (29) | `finalizeForeignPtr` |
| `Foreign.C.String` (27) | `castCharToCUChar`, `castCUCharToChar`, `castCharToCSChar`, `castCSCharToChar` |
| `Foreign.C.Error` (26) | `throwErrnoPath`, `throwErrnoPathIf`, `throwErrnoPathIf_`, `throwErrnoPathIfNull`, `throwErrnoPathIfMinus1`, `throwErrnoPathIfMinus1_` |

| Module | Types that differ |
|---|---|
| `Control.Monad` (13) | `mapM`, `forM`, `sequence` take any `Traversable`; `mapM_`, `forM_`, `sequence_`, `msum` any `Foldable`; `forever`, `filterM`, `zipWithM`, `replicateM`, `when`, `unless` and their kin need only `Applicative`; `guard` needs `Alternative`; `fail` is `MonadFail`'s; `MonadPlus` has `Alternative` as a superclass |
| `Data.List` (20) | `null`, `length`, the folds, `concat`, `concatMap`, `and`, `or`, `any`, `all`, `sum`, `product`, `maximum`, `minimum`, `elem`, `notElem`, `find`, `maximumBy`, `minimumBy` are `Foldable`'s; `mapAccumL` and `mapAccumR` are `Traversable`'s; `genericLength` needs `Num a`, not `Integral a` |
| `Data.Ix` (19) | `Ix` has an extra method, `unsafeIndex` |
| `Data.Bits` (15) | `Bits` has the methods GHC's base added, and a subclass `FiniteBits` |
| `Data.Array` (14) | `Array` has `Foldable` and `Traversable` instances |

The rest of this part lists what the modules do differently from the
Report's text.  A chapter not mentioned conforms in every probe.

### 15 Data.Bits

Conforms.  `bitSize` of `Int` and `Word` is 32.  Unspecified: a
negative shift count, which the Report forbids, shifts the other way;
`popCount (-1 :: Integer)` is `-1`.

### 16 Data.Char

**`isSpace` knows only Latin-1.**  It is `True` for the ASCII spaces
and `\xa0`, and `False` for the other Unicode spaces (`\x2000` to
`\x200a`, `\x3000` and so on) though `generalCategory` classifies them
as `Space`.  The Report says any Unicode space character.

**The Unicode table is Unicode 4.0.1.**  Characters assigned since are
`NotAssigned`, so `isAlpha` and `isPrint` are `False` for them; the
Report names no Unicode version.  Everything else in the chapter
conforms: `toUpper`, `toLower` and `toTitle` on non-ASCII letters, the
predicates, `generalCategory`, `showLitChar`, `lexLitChar` and
`readLitChar` give the Report's answers.

### 18 Data.Int and 23 Data.Word

Arithmetic on the sized types is modulo 2^n as the chapters say, and
`Int` and `Word` are 32 bits.

**`toEnum` and `fromEnum` do not fail when the value is out of range.**
`toEnum 200 :: Int8` is `-56`, `fromEnum (maxBound :: Word64)` is `-1`;
6.3.4 asks for a run-time error.  `succ maxBound` and `pred minBound`
do fail.

### 19 Data.Ix

**Tuple instances stop at five components.**  The Report asks for `Ix`
up to at least size 15; `Ix (a,b,c,d,e,f)` does not exist.

### 26 Foreign.C.Error

**`errnoToIOError` builds a user error.**  Its result satisfies
`isUserError`, not the predicate for the `errno` value, so
`isDoesNotExistError` is `False` for `eNOENT`, and the file name is
folded into the message rather than attached.  Everything thrown by
the `throwErrno` family is therefore a user error.  The hundred `e*`
constants, `getErrno`, `resetErrno` and the `throwErrnoIf` family are
all there and work.

### 27 Foreign.C.String

Conforms to the letter of 27.1.1, where `CString` conversion is
"identical to" the `CAString` one: a character is stored as its code
point modulo 256, so `"日本"` becomes two wrong bytes and a character
above `\255` is not representable (`charIsRepresentable` says so).
GHC uses the locale encoding.  `CWString` is UTF-32 and round-trips
any string.  Cupid's own I/O is UTF-8; see
[Character encoding](../explanation/character-encoding.md).

### 28 Foreign.C.Types

All 26 types exist with exactly the instances the chapter lists, and
`sizeOf` and `alignment` agree with the C compiler for every one.

**`pi` at `CFloat` and `CDouble` does not terminate:** the `Floating`
instances define it as itself.

**`isNaN` and `isInfinite` are `False`** at `CFloat` and `CDouble`, as
at `Float` and `Double` (chapter 6).

### 31 Foreign.Marshal.Alloc

**Allocation failure is an `IOError`,** `resource exhausted (out of
memory)`, where the chapter says `nullPtr` is returned.

### 36 Foreign.StablePtr

**There is no `Eq (StablePtr a)` instance,** which the chapter
requires.  Unspecified: `deRefStablePtr` after `freeStablePtr` returns
garbage and then fails with an internal error; a second
`freeStablePtr` is a no-op.

### Other Foreign modules

`Foreign`, `Foreign.C`, `Foreign.ForeignPtr` (finalizers run in
reverse order of addition, at collection and at exit),
`Foreign.Marshal.Array`, `Foreign.Marshal.Error` (with `void`),
`Foreign.Marshal.Utils`, `Foreign.Ptr` (with `IntPtr` and `WordPtr`)
and `Foreign.Storable` (with the Report's default methods) conform in
every probe, apart from the missing names in the table above.
`Foreign.Marshal` and `Foreign` also export `Foreign.Marshal.Pool`.

### 38 Numeric

Conforms for finite values, down to `showFFloat (Just 0) 2.5 ""` being
`"3"` and `showFloat (-0.0) ""` being `"0.0"` as the reference code
has them.  `showFloat` gives the shortest digits that read back, which
the Prelude's `show` does not (chapter 6).

**Non-finite values are mishandled.**  `showFloat (1/0) ""` is
`"0.0e0"`, and `showFloat (0/0) ""` and `readFloat "1e400"` raise
`arithmetic overflow`, because `Double` does not recognise NaN or
infinity (chapter 6).

### 40 System.Exit

Conforms.  `exitWith` cannot be caught by `catchIOError`; `exitWith
(ExitFailure 0)` fails with a pattern-match error rather than an
`IOError`, which the Report leaves open.

### 41 System.IO

Conforms in the handle semantics of 41.2 to 41.5, including
semi-closed handles, the errors on closed and wrong-mode handles, the
three open modes, seeking and positions, buffering, `fixIO` and the
end-of-file conditions, with these exceptions and the missing names
above.

**`hClose` on a standard handle does nothing.**  After `hClose stdout`
the handle is still open and still writes.

**`hWaitForInput` and `hReady` are `True` at end of file,** where the
Report has them fail with an end-of-file error.

**`hSeek` past the end of a file succeeds,** where the Report says it
is not possible; `hTell` then reports the position asked for.

**`hTell` on a pipe returns `-1`** instead of failing; `hSeek` and
`hFileSize` on it do fail with `isIllegalOperation`.

**`hGetBuffering` reports `BlockBuffering (Just 8192)`** after
`hSetBuffering h (BlockBuffering Nothing)`.

**No file locking**, as in chapter 7.

Unspecified: `show` of a handle is `<handle>`, `<stdin>`, `<stdout>`
or `<stderr>`; text handles are UTF-8, and a byte sequence that is not
UTF-8 raises an `IOError` reading `protocol error`; binary mode writes
the low eight bits of each character; opening a directory for reading
succeeds and the first read fails.  See [Character
encoding](../explanation/character-encoding.md).

### 42 System.IO.Error

Conforms, apart from the names: the predicates, `mkIOError`,
`annotateIOError`, `ioeGet*` and nested handlers all behave as
described.  Unspecified: an error shows as `file: location:
description (detail)`, for example `missing.txt: IO.openFile: does not
exist (file does not exist)`; `ioeGetErrorString` of an error that is
not a user error returns the description.

## See also

- [The language Cupid accepts](language.md)
- [Language extensions](extensions.md)
- [The libraries](libraries.md)
- The Report: <https://www.haskell.org/onlinereport/haskell2010/>
