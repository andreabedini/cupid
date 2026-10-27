# Add a library to the search path

Make a directory of Haskell modules importable from any Cupid session.

## Prerequisites

- a directory tree in which module `A.B.C` is the file `A/B/C.hs`

## Steps

### 1. Put the modules in a package directory

The default search path already includes every immediate
subdirectory of two package directories:

```
{Home}/lib/cupid/packages/*
/usr/local/lib/cupid/packages/*
```

`{Home}` is your home directory.  So the simplest way to add a library
called `mylib` for yourself is:

```sh
mkdir -p ~/lib/cupid/packages
cp -r mylib ~/lib/cupid/packages/
```

Its modules can now be imported.  If a module has foreign imports,
compile it there with `cupid --ffi` so that its `.so` file sits next to it.

### 2. Or extend the search path

To use a directory anywhere else, add it to the search path with `-P`.
An empty entry stands for the previous path, so this appends:

```sh
cupid -P:/path/to/mylib
```

An entry ending in `/*` means every immediate subdirectory.  Quote it, so
that the shell does not expand it:

```sh
cupid -P':/path/to/packages/*'
```

To make it permanent, put the option in `CUPIDFLAGS`.

## Verify it worked

```
Cupid> :load My.Module
My.Module>
```

`:set` prints the search path in force on its `Search path` line.

## Troubleshooting

### `unable to locate Prelude along new path`

**Cause**: a `-P` argument without a leading `:` replaced the whole
path, and the new path does not reach the Cupid libraries.

**Fix**: start the argument with `:` to append, or include
`{Cupid}/packages/*` in the new path.

### Removing a library

Delete its directory.  There is no package database.

## See also

- [Options](../reference/options.md#-pstr) for the syntax of `-P`
- [Modules, files and the search path](../explanation/modules-and-the-search-path.md)
