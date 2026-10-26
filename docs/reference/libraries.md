# The libraries

The Haskell modules that ship with Cupid.

## Packages

The libraries are plain Haskell that Cupid owns (see
[the design](../explanation/design.md#the-libraries)).  They live in two
directories under `libraries/` in the source tree and are installed as
subdirectories of `packages/` in the library directory, where the
default search path finds them.

| Package | Contents |
|---|---|
| `cupid` | `Cupid.Prelude`, where most of the `Prelude` is defined, and the `Cupid.*` modules that bind the interpreter's primitives |
| `base` | The `Prelude` and `base`, following GHC's base, roughly version 4.18: `Control.*`, `Data.*`, `Foreign.*`, `System.*`, `Text.*`, with GHC's class hierarchy (see [the language](language.md#predefined-types-and-classes)), and the modules added since 2006, such as `Control.Category` and `GHC.Generics` |

## Modules beyond Haskell 2010

Much of `base` is not Haskell 2010.  The `ST` monad's `runST` has a
rank-2 type, and `Control.Monad.ST`, `Data.STRef`, the mutable and
unboxed array modules, `Data.HashTable` and `Data.Graph` are built on
it; other modules use multi-parameter classes or existential types.  All
of them load without any option, since the extensions are always enabled
(overlapping instances excepted; they need `+o`); see [Language
extensions](extensions.md).

## Modules added since 2006

`base` has the modules of GHC's base that later library code imports
most: `Control.Category`, `Control.Monad.Fail`,
`Control.Monad.IO.Class`, `Control.Monad.Zip`, `Data.Bifoldable`,
`Data.Bifunctor`, `Data.Bitraversable`, `Data.Data`, `Data.Foldable1`,
`Data.Function`, `Data.Functor` and its submodules (among them
`.Classes` and `.Contravariant`), `Data.List.NonEmpty`, `Data.Proxy`,
`Data.Semigroup`, `Data.String`, `Data.Void`, `GHC.Generics`,
`GHC.Stack` and `Numeric.Natural`.  With them the transformers package
loads from its unmodified source, once it has been through the C
preprocessor (see [Load a package written for
GHC](../how-to/preprocess-sources.md#load-a-package-written-for-ghc)).
The older modules have the functions added to them since, such as
`Data.List.intercalate`, `Data.Ord.Down` and `Text.Read.readMaybe`.  Six
differ from GHC's:

- `Proxy` stands for a type of kind `Type` only, since kinds are not
  polymorphic: `Proxy :: Proxy Int` but not `Proxy :: Proxy Maybe`.
- `IsString` has no overloaded string literals behind it, so
  `fromString` must be called.
- A `Natural` that would be negative is an `error` call,
  `arithmetic underflow`, rather than an `ArithException`.
- `Data`'s `dataCast1` and `dataCast2` take `Typeable1` and `Typeable2`
  for the type constructor, as `Typeable` is for types of kind `Type`
  only.
- `HasCallStack` is a class with no parameters that always holds, so
  code with it in its types loads and runs, but no call stack is
  recorded: `callStack` is always empty, and an `error` call reports
  only its message.
- `Read1` and `Read2` have only their `ReadS` methods, since the `Read`
  class has no `readPrec`, and `Eq1` and its kin have no quantified
  superclasses, so `Eq1 f` does not give `Eq (f a)`.

## Exceptions

`Control.Exception` has the `Exception` class of GHC's base, with
`SomeException`, `throw`, `throwIO`, `catch`, `catches`, `try`,
`bracket`, `finally`, `onException` and the exception types the
interpreter raises: `ErrorCall` for `error`, `ArithException`,
`PatternMatchFail` and the rest.  Since every type is `Typeable` (see
[the extensions](extensions.md#typeable-for-every-type)), an empty
instance makes a type an exception:

```haskell
data BankError = InsufficientFunds Int deriving Show
instance Exception BankError
```

One that nothing catches is reported with its `displayException`, after
`Program error:`, and the program exits with status 1.  Four things
differ from GHC's:

- Cupid's threads cannot be interrupted, so there are no asynchronous
  exceptions to mask: `mask` and its kin only run the action, and there
  is no `throwTo`.
- `assert` does not check its condition.
- `ErrorCall` is a single constructor, with no source location.
- `catchDyn` and `throwDyn`, from the 2006 library, remain.

## Modules with foreign imports

Nine modules call C through the FFI.  Their stubs are compiled by
`fficupid` when the libraries are built, and the objects are installed
next to them:

- `Cupid.Storable`
- `Foreign.C.Error`, `Foreign.Marshal.Alloc`, `Foreign.Marshal.Utils`,
  `Foreign.Ptr`
- `System.Posix.Internals`, `System.Posix.Signals`,
  `System.Process.Internals`, which are POSIX only
- `Data.ByteString.Base`

## Windows

On Windows the libraries leave out the three POSIX-only modules,
`System.Posix.Internals`, `System.Posix.Signals` and
`System.Process.Internals`, and `System.Posix.Types` has only the types
Windows has: not `CGid`, `CNlink`, `CUid`, `CCc`, `CSpeed`, `CTcflag`,
`CRLim`, `LinkCount`, `UserID` or `GroupID`.  The Windows version of a
module is under `libraries/windows`.

## The generated module

`Cupid.SysConfig` is not in the source tree.  It is generated at build
time by `libraries/tools/mksysconfig.c` and holds the mapping from C
types to Haskell types (`HsType_int`, `HsType_mode_t`, ...), the
`errno` and signal constants (`const_E2BIG`, `const_SIGINT`, ...), and
`hostOS` and `hostArch`.  The C-side equivalent is `HsBaseConfig.h`.

## Module list

### cupid

`Cupid`, and under `Cupid.`: `Array`, `Base`, `Bits`,
`ByteArray`, `Char`, `ConcBase`, `Directory`, `Exception`,
`ForeignPtr`, `Int`, `IO`, `IOArray`, `IOExts`, `IORef`, `LazyST`,
`Memo`, `Numeric`, `Prelude`, `Ptr`, `ST`, `Stable`, `StablePtr`,
`Storable`, `SysConfig` (generated), `System`, `Time`, `Weak`, `Word`.

### base

`Control.Applicative`, `Control.Arrow`, `Control.Category`,
`Control.Concurrent` (and `.Chan`, `.MVar`, `.QSem`, `.QSemN`,
`.SampleVar`), `Control.Exception`, `Control.Monad` (and `.Fail`,
`.Fix`, `.Instances`, `.IO.Class`, `.ST`, `.ST.Lazy`, `.ST.Strict`,
`.Zip`), `Control.Parallel`, `Data.Array` (and `.Base`, `.Diff`,
`.IArray`, `.IO`, `.IO.Internals`, `.MArray`, `.ST`, `.Storable`,
`.Unboxed`), `Data.Bifoldable`, `Data.Bifunctor`,
`Data.Bitraversable`, `Data.Bits`, `Data.Bool`, `Data.ByteString` (and
`.Base`, `.Char8`, `.Fusion`, `.Lazy`, `.Lazy.Char8`), `Data.Char`,
`Data.Complex`, `Data.Data`, `Data.Dynamic`, `Data.Either`, `Data.Eq`,
`Data.Fixed`, `Data.Foldable`, `Data.Foldable1`, `Data.Function`,
`Data.Functor` (and `.Classes`, `.Compose`, `.Const`,
`.Contravariant`, `.Identity`, `.Product`, `.Sum`), `Data.FunctorM`,
`Data.Graph`, `Data.HashTable`, `Data.Int`, `Data.IntMap`,
`Data.IntSet`, `Data.IORef`, `Data.Ix`, `Data.Kind`, `Data.List` (and
`.NonEmpty`), `Data.Map`, `Data.Maybe`, `Data.Monoid`, `Data.Ord`,
`Data.PackedString`, `Data.Proxy`, `Data.Queue`, `Data.Ratio`,
`Data.Semigroup`, `Data.Sequence`, `Data.Set`, `Data.STRef` (and
`.Lazy`, `.Strict`), `Data.String`, `Data.Traversable`, `Data.Tree`,
`Data.Tuple`, `Data.Typeable`, `Data.Unique`, `Data.Version`,
`Data.Void`, `Data.Word`, `Debug.Trace`, `Foreign`, `Foreign.C` (and
`.Error`, `.String`, `.Types`), `Foreign.ForeignPtr`,
`Foreign.Marshal` (and `.Alloc`, `.Array`, `.Error`, `.Pool`,
`.Utils`), `Foreign.Ptr`, `Foreign.StablePtr`, `Foreign.Storable`,
`GHC.Generics`, `GHC.Stack` (and `.Types`), `Numeric`,
`Numeric.Natural`, `Prelude`, `System.Cmd`, `System.Console.GetOpt`,
`System.CPUTime`, `System.Directory` (and `.Internals`),
`System.Environment`, `System.Exit`, `System.Info`,
`System.IO` (and `.Error`, `.Unsafe`), `System.Locale`, `System.Mem`
(and `.StableName`, `.Weak`), `System.Posix.Internals` (not on
Windows), `System.Posix.Signals` (not on Windows), `System.Posix.Types`,
`System.Process.Internals` (not on Windows), `System.Random`,
`System.Time`,
`Text.ParserCombinators.ReadP`, `Text.ParserCombinators.ReadPrec`,
`Text.PrettyPrint` (and `.HughesPJ`), `Text.Printf`, `Text.Read` (and
`.Lex`), `Text.Show` (and `.Functions`).

## Demos

`demos/` in the source tree, installed under the library directory,
holds example programs from the original distribution: a banner
printer (`Say`), a Prolog interpreter (`prolog/`), N queens, a
calendar, Eliza and others.  `Demos.hs` imports most of them so that
one `:load` brings them all in.

## See also

- [Add a library to the search path](../how-to/add-a-library.md)
- [Build options and install layout](build-options.md)
