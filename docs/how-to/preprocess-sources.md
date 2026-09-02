# Preprocess source files before loading

Run every source file through a filter, such as the C preprocessor,
before Hugs reads it.

## Prerequisites

- a filter program that takes a file name as its last argument and
  writes the result to standard output

## Steps

### 1. Set the filter

The `-F` option names the command.  Hugs runs it with the source file
name appended and reads its standard output instead of the file.  To use
the C preprocessor:

```
Hugs> :set -F"cpp -P -traditional"
```

or on the command line:

```sh
hugs -F"cpp -P -traditional" Main.hs
```

`-P` stops cpp from emitting line markers, which Hugs would take for
Haskell.  `-traditional` keeps it from mangling Haskell's `'` and
`--`.

### 2. Load as usual

```
Hugs> :load Main.hs
```

### 3. Turn it off

An empty argument clears the filter:

```
Hugs> :set -F
```

## Notes

- The filter is slower than reading files directly, and because of the
  way imports are chased it runs twice on a file that imports a module
  not yet loaded.
- Hugs does not notice `#include`d files.  After changing one, use
  `:load` rather than `:reload`.
- Hugs ignores `{-# LINE #-}` pragmas, so error messages give line
  numbers in the preprocessed text.
- The filter is given the file name, so it can decide what to do by the
  suffix.  Combine `-F` with `-S` to have Hugs look for files with an
  extra suffix that only the filter understands.

## See also

- [Options](../reference/options.md) for `-F` and `-S`
