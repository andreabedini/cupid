# Add a library to the search path

Make a directory of Haskell modules importable from any Cuddles session.

## Prerequisites

- a directory tree in which module `A.B.C` is the file `A/B/C.hs` or
  `A/B/C.lhs`

## Steps

### 1. Put the modules in a package directory

The default search path already includes every immediate
subdirectory of two package directories:

```
{Home}/lib/cuddles/packages/*
/usr/local/lib/cuddles/packages/*
```

`{Home}` is your home directory.  So the simplest way to add a library
called `mylib` for yourself is:

```sh
mkdir -p ~/lib/cuddles/packages
cp -r mylib ~/lib/cuddles/packages/
```

Its modules can now be imported.  If a module has foreign imports,
compile it there with `fficuddles` so that its `.so` file sits next to it.

### 2. Or extend the search path

To use a directory anywhere else, add it to the search path with `-P`.
An empty entry stands for the previous path, so this appends:

```sh
cuddles -P:/path/to/mylib
```

An entry ending in `/*` means every immediate subdirectory:

```sh
cuddles -P:/path/to/packages/*
```

To make it permanent, put the option in `CUDDLESFLAGS`.

## Verify it worked

```
Cuddles> :load My.Module
My.Module>
```

`:set` prints the search path in force on its `Search path` line.

## Troubleshooting

### `unable to locate Prelude along new path`

**Cause**: a `-P` argument without a leading `:` replaced the whole
path, and the new path does not reach the Cuddles libraries.

**Fix**: start the argument with `:` to append, or include
`{Cuddles}/packages/*` in the new path.

### Removing a library

Delete its directory.  There is no package database.

## See also

- [Options](../reference/options.md#-pstr) for the syntax of `-P`
- [Modules, files and the search path](../explanation/modules-and-the-search-path.md)
