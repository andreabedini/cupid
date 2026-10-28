# Differences from Hugs 98

What changed for someone who used Hugs, the September 2006 release
that Cupid starts from.  Each table gives the Hugs name on the left and
what to use in Cupid on the right.  The commands at the prompt are
Hugs's, and so are most options; anything this page does not mention
works as it did.

## Programs

| Hugs | Cupid |
|---|---|
| `hugs` | `cupid` shows the prompt |
| `hugs File.hs` | `cupid -i File.hs`.  Without `-i`, `cupid File.hs` runs the program |
| `runhugs File.hs args` | `cupid File.hs args` |
| `ffihugs File.hs cc-args` | `cupid --ffi File.hs cc-args` |
| `ffihugs '-i<math.h>' File.hs` | No `-i` option: name the header in each import, `foreign import ccall "math.h sin"` |
| WinHugs | None.  The Windows build is the same console program as on other systems |

`cupid` decides what to do from its arguments; [Running cupid](cupid.md)
has the details.  A script for `runhugs` had to be a literate script
so that its `#!` line was not read as Haskell.  Cupid reads a `#!`
first line as a blank line, so a script is an ordinary `.hs` file; see
[Write a script that runs with cupid](../how-to/write-a-script.md).

## Environment variables and files

| Hugs | Cupid |
|---|---|
| `HUGSFLAGS` | `CUPIDFLAGS` |
| `HUGSDIR` | `CUPIDDIR` |
| `{Hugs}` in the search path | `{Cupid}` |
| `lib/hugs/packages` | `lib/cupid/packages` |
| `LC_CTYPE` chooses the encoding of `Char` | UTF-8 everywhere, unless Cupid was built with `-Dchar-encoding=locale`; see [Character encoding](../explanation/character-encoding.md) |
| Options in the Windows registry, the WinHugs settings or the Mac "Hugs Preferences" file | Read only from `CUPIDFLAGS` and the command line |
| `EMACS`, `IGNORE_USER_REGISTRY` | Not read |

[Environment variables](environment-variables.md) lists what Cupid reads.

## At the prompt

| Hugs | Cupid |
|---|---|
| The empty module `Hugs` and the prompt `Hugs>` | The empty module `Cupid` and the prompt `Cupid>` |
| `:reload` reads every loaded module again | `:reload` reads the first module whose file changed and those loaded after it, and nothing if no file changed |

The other commands are unchanged; [Interpreter commands](commands.md)
lists them.

## Options

| Hugs | Cupid |
|---|---|
| `+98` and `-98` switch the extensions off and on | The extensions are always on.  Both options are ignored with a message |
| `+l`, and `.lhs` in the `-S` default | Literate scripts are not read.  `-S` defaults to `.hs`; `+l` is an unknown toggle |
| `+H` (here documents) | Gone with here documents |
| `+R` (root optimisation) | Always on; `+R` is an unknown toggle |
| `-h` defaults to 250k cells | `-h` defaults to 1000000 cells |
| `-e`, `-f`, `-N`, `-W`, `-G`, `+L` | Ignored with a message |

[Options](options.md) describes every option Cupid has.

## Language

Cupid accepts Haskell 2010 rather than Haskell 98.  For a Hugs program
the changes that matter are these:

- **n+k patterns are not accepted**, as in Haskell 2010.
- **Pattern guards are.**
- **The extensions are always on**, except overlapping instances,
  which still need `+o` or `+O`.  So `forall` is always a keyword.
- **These extensions are gone:** TREX extensible records, implicit
  parameters, the `observe` debugger (HugsHood), recursive `mdo`,
  parallel list comprehensions, here documents and multi-instance
  resolution.  [The design](../explanation/design.md#deleting-instead-of-switching-off)
  says why.
- **Others are new:** kind signatures, GADT syntax, equality
  constraints, type operators, tuple sections, polymorphic and
  constraint synonyms, and GHC's deriving strategies with `Generic`
  and `Data`.  See [Language extensions](extensions.md).
- **Restricted type synonyms are kept**, the one Hugs-only extension
  that is.

[The language Cupid accepts](language.md) lists the deviations from
the Report; most of those in Hugs's list are still there.

## Libraries

The libraries follow GHC's base, roughly version 4.18, rather than the
Haskell 98 libraries.

**The monad classes are those of base.**  `Applicative` is a superclass
of `Monad` and `fail` belongs to `MonadFail`, so a Hugs program that
defines a `Monad` instance on its own needs `Functor` and
`Applicative` instances as well.  `Semigroup` is a superclass of
`Monoid`, and the Prelude exports `Foldable` and `Traversable`.
[The language Cupid accepts](language.md#predefined-types-and-classes) has the full list.

**The Haskell 98 module names are gone.**  Hugs shipped the flat names
of the Haskell 98 Report in its `haskell98` package; in Cupid,
`import Char` fails with `Can't find imported module "Char"`.  Use the
hierarchical names:

| Haskell 98 | Cupid |
|---|---|
| `Array` | `Data.Array` |
| `Char` | `Data.Char` |
| `Complex` | `Data.Complex` |
| `CPUTime` | `System.CPUTime` |
| `Directory` | `System.Directory` |
| `IO` | `System.IO` |
| `Ix` | `Data.Ix` |
| `List` | `Data.List` |
| `Locale` | `System.Locale` |
| `Maybe` | `Data.Maybe` |
| `Monad` | `Control.Monad` |
| `Random` | `System.Random` |
| `Ratio` | `Data.Ratio` |
| `System` | `System.Exit`, `System.Environment` and `System.Cmd` |
| `Time` | `System.Time` |

`Numeric` keeps its name.

**The interpreter's own modules are `Cupid.*`.**  What was
`Hugs.Prelude`, `Hugs.IOExts` and so on is `Cupid.Prelude`,
`Cupid.IOExts` and so on.  `Hugs.Trex` and `Hugs.Observe` went with
their extensions.  [The libraries](libraries.md) lists the modules that
ship.

## See also

- [History](../explanation/history.md) for the changes in order
- [The design of Cupid](../explanation/design.md) for why they were made
