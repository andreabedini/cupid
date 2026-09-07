# Modules, files and the search path

How Cuddles turns a module name into a file, what the current module is,
and why the default search path looks the way it does.

## One module per file

Cuddles keeps each module in its own file and does not support mutually
recursive modules.  A file may be loaded by its path, in which case it
may contain a module of any name, or by its module name, in which case
Cuddles has to find it.

To find module `M`, Cuddles tries `dir/M.hs` and `dir/M.lhs` for each
directory `dir` on the search path, in order.  A hierarchical name is
mapped to a path: `A.B.C` becomes `dir/A/B/C.hs`.  The suffix list is
the `-S` option and the search path the `-P` option.  Files ending in
`.lhs` are literate scripts; with `+l`, so is any file whose name ends
in neither suffix.

## Imports are chased

Loading a module loads the modules it imports, recursively.  For these,
one extra directory is tried before the search path: the directory in
which the importing module was found, when that module was itself
loaded by file name or found relative to that directory.  So

```
Cuddles> :load /path/to/project/Main.hs
```

resolves `import A.B.C` to `/path/to/project/A/B/C.hs` without the
project directory being on the search path, and an `import D` in that
module to `/path/to/project/D.hs`.  Modules found on the search path
proper do not get this extra directory; their imports are resolved from
the search path alone.

## The default search path

```
.:{Home}/lib/cuddles/packages/*:/usr/local/lib/cuddles/packages/*:{Cuddles}/packages/*
```

The current directory comes first, so a project's own modules shadow
everything.  `{Cuddles}` is the library directory and its `packages/`
holds the three shipped packages; `dir/*` means every immediate
subdirectory of `dir`, so each package's root is a search directory
and a module `Data.Map` is found as `packages/base/Data/Map.hs`.

The two other `packages/*` entries exist for libraries you add.  One is
under your home directory, the other is the system-wide location.  A
library is a directory of modules copied under either; there is no
package database and nothing to register.  This is what remains of the
Cabal support of the 2006 release, which installed packages into these
directories.

Cuddles refuses a search path or suffix list from which the `Prelude`
cannot be found, so `:set -P` cannot lock you out of the interpreter.

## The current module

Expressions at the prompt are evaluated in the scope of one module,
the current module, which the prompt names.  At start it is `Cuddles`, an
empty module that exists only for this purpose.  `:load` and `:also`
make the last module loaded current; `:module` changes it.

The scope is the module's whole top level, not its export list, so
names the module does not export are usable at the prompt.  This is
deliberate: when experimenting, being able to poke at the internals
of the module you are writing is worth more than a faithful
simulation of what importers see.  It does mean that an abstract type
is not abstract at the prompt of its own module.

Only one module is current at a time.  To have several in scope, write
a module that imports them and make that one current.

## Why `Cuddles.Prelude`

Haskell defines the `Prelude` and the standard libraries as mutually
recursive, which Cuddles does not support.  Instead almost everything is
defined in `Cuddles.Prelude`, which the `Prelude` and the library modules
import.  This is why `:names` shows `Cuddles.Prelude.map` rather than
`Prelude.map`, and why an error message may mention `Cuddles.Prelude`.

## See also

- [Options](../reference/options.md) for `-P`, `-S` and `+l`
- [Add a library to the search path](../how-to/add-a-library.md)
- [Interpreter commands](../reference/commands.md) for `:load`,
  `:also` and `:module`
