# Preprocess source files before loading

Run every source file through a filter, such as the C preprocessor,
before Cupid reads it.

## Prerequisites

- a filter program that takes a file name as its last argument and
  writes the result to standard output

## Steps

### 1. Set the filter

The `-F` option names the command.  Cupid runs it with the source file
name appended and reads its standard output instead of the file.  To use
the C preprocessor:

```
Cupid> :set -F"cpp -P -traditional"
```

or on the command line:

```sh
cupid -F"cpp -P -traditional" -i Main.hs
```

`-P` stops cpp from emitting line markers, which Cupid would take for
Haskell.  `-traditional` keeps it from mangling Haskell's `'` and
`--`.

### 2. Load as usual

```
Cupid> :load Main.hs
```

### 3. Turn it off

An empty argument clears the filter:

```
Cupid> :set -F
```

## Load a package written for GHC

Packages written for GHC test the version of `base` with
`MIN_VERSION_base(a,b,c)`, a macro that Cabal defines and Cupid does
not.  Define it on the cpp command line, true for every version up to
4.18, the base that Cupid's libraries follow:

```
Cupid> :set -F"cpp -P -traditional -DMIN_VERSION_base(a,b,c)=((a)<4||((a)==4&&(b)<=18))"
```

The macro has no spaces in it, because Cupid splits the command on
whitespace.  Leave `__GLASGOW_HASKELL__` undefined, so that the package
takes the code it keeps for compilers other than GHC.  Then put the
package's source directory on the search path (see
[Add a library to the search path](add-a-library.md)) and load its
modules as usual.  This is how the transformers package, 0.6.1.2, loads
from its unmodified source.

## Notes

- The filter is slower than reading files directly, and because of the
  way imports are chased it runs twice on a file that imports a module
  not yet loaded.
- Cupid does not notice `#include`d files.  After changing one, use
  `:load` rather than `:reload`.
- Cupid ignores `{-# LINE #-}` pragmas, so error messages give line
  numbers in the preprocessed text.
- The filter is given the file name, so it can decide what to do by the
  suffix.  Combine `-F` with `-S` to have Cupid look for files with an
  extra suffix that only the filter understands.

## See also

- [Options](../reference/options.md) for `-F` and `-S`
