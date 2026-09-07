# Environment variables

Variables that `cuddles`, `runcuddles` and `fficuddles` consult.

### `CUDDLESFLAGS`

Options, in the syntax of [Options](options.md), read before the
command line.  Spaces inside an option need a backslash or double
quotes:

```sh
CUDDLESFLAGS='+k -E"vi +%d %s"'
export CUDDLESFLAGS
```

### `CUDDLESDIR`

The library directory.  Default: the directory chosen at build time,
`lib/cuddles` under the prefix.  It is substituted for `{Cuddles}` in the
search path and must contain `packages/`, and for `fficuddles` also
`include/HsFFI.h`.  Set it to `build/libraries/cuddlesdir` to run from
the build tree.

### `HOME`

The home directory.  Substituted for `{Home}` in the search path and
for `~` in the argument of `:cd`.

### `EDITOR`

The editor run by `:edit` and `:find` when no `-E` option is given.

### `SHELL`

The shell run by `:!`.  Default: `/bin/sh`.

### `EMACS`

If set, Cuddles assumes it is running in an Emacs shell buffer and reads
its input line by line without stripping carriage returns.

### `CC`

Read by meson at configuration time, not by Cuddles.  The compiler it
names is also the one `fficuddles` runs.  See
[Build and install Cuddles](../how-to/build-and-install.md).

## See also

- [Options](options.md)
