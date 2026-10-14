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
| `cupid` | The `Prelude`, `Cupid.Prelude` (where most of it is defined) and the `Cupid.*` modules that bind the interpreter's primitives |
| `base` | The `base` library of 2006: `Control.*`, `Data.*`, `Foreign.*`, `System.*`, `Text.*` |

## Modules beyond Haskell 2010

Much of `base` is not Haskell 2010.  The `ST` monad's `runST` has a
rank-2 type, and `Control.Monad.ST`, `Data.STRef`, the mutable and
unboxed array modules, `Data.HashTable` and `Data.Graph` are built on
it; other modules use multi-parameter classes or existential types.
All of them load without any option, since the extensions are always
enabled; see [Language extensions](extensions.md).

## Modules with foreign imports

Nine modules call C through the FFI.  Their stubs are compiled by
`fficupid` when the libraries are built, and the objects are installed
next to them:

- `Cupid.Storable`
- `Foreign.C.Error`, `Foreign.Marshal.Alloc`, `Foreign.Marshal.Utils`,
  `Foreign.Ptr`
- `System.Posix.Internals`, `System.Posix.Signals`,
  `System.Process.Internals`
- `Data.ByteString.Base`

## The generated module

`Cupid.SysConfig` is not in the source tree.  It is generated at build
time by `libraries/tools/mksysconfig.c` and holds the mapping from C
types to Haskell types (`HsType_int`, `HsType_mode_t`, ...), the
`errno` and signal constants (`const_E2BIG`, `const_SIGINT`, ...), and
`hostOS` and `hostArch`.  The C-side equivalent is `HsBaseConfig.h`.

## Module list

### cupid

`Cupid`, `Prelude`, and under `Cupid.`: `Array`, `Base`, `Bits`,
`ByteArray`, `Char`, `ConcBase`, `Directory`, `Exception`,
`ForeignPtr`, `Int`, `IO`, `IOArray`, `IOExts`, `IORef`, `LazyST`,
`Memo`, `Numeric`, `Prelude`, `Ptr`, `ST`, `Stable`, `StablePtr`,
`Storable`, `SysConfig` (generated), `System`, `Time`, `Weak`, `Word`.

### base

`Control.Applicative`, `Control.Arrow`, `Control.Concurrent` (and
`.Chan`, `.MVar`, `.QSem`, `.QSemN`, `.SampleVar`),
`Control.Exception`, `Control.Monad` (and `.Fix`, `.Instances`, `.ST`,
`.ST.Lazy`, `.ST.Strict`), `Control.Parallel`,
`Data.Array` (and `.Base`, `.Diff`, `.IArray`, `.IO`, `.IO.Internals`,
`.MArray`, `.ST`, `.Storable`, `.Unboxed`), `Data.Bits`, `Data.Bool`,
`Data.ByteString` (and `.Base`, `.Char8`, `.Fusion`, `.Lazy`,
`.Lazy.Char8`), `Data.Char`, `Data.Complex`, `Data.Dynamic`,
`Data.Either`, `Data.Eq`, `Data.Fixed`, `Data.Foldable`,
`Data.FunctorM`, `Data.Graph`, `Data.HashTable`, `Data.Int`,
`Data.IntMap`, `Data.IntSet`, `Data.IORef`, `Data.Ix`, `Data.Kind`,
`Data.List`, `Data.Map`, `Data.Maybe`, `Data.Monoid`, `Data.Ord`,
`Data.PackedString`, `Data.Queue`, `Data.Ratio`, `Data.Sequence`,
`Data.Set`, `Data.STRef` (and `.Lazy`, `.Strict`), `Data.Traversable`,
`Data.Tree`, `Data.Tuple`, `Data.Typeable`, `Data.Unique`,
`Data.Version`, `Data.Word`, `Debug.Trace`,
`Foreign`, `Foreign.C` (and `.Error`, `.String`, `.Types`),
`Foreign.ForeignPtr`, `Foreign.Marshal` (and `.Alloc`, `.Array`,
`.Error`, `.Pool`, `.Utils`), `Foreign.Ptr`, `Foreign.StablePtr`,
`Foreign.Storable`, `Numeric`, `System.Cmd`,
`System.Console.GetOpt`, `System.CPUTime`, `System.Directory` (and
`.Internals`), `System.Environment`, `System.Exit`, `System.Info`,
`System.IO` (and `.Error`, `.Unsafe`), `System.Locale`, `System.Mem`
(and `.StableName`, `.Weak`), `System.Posix.Internals`,
`System.Posix.Signals`, `System.Posix.Types`,
`System.Process.Internals`, `System.Random`, `System.Time`,
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
