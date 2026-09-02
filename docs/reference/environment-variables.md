# Environment variables

Variables that `hugs`, `runhugs` and `ffihugs` consult.

### `HUGSFLAGS`

Options, in the syntax of [Options](options.md), read before the
command line.  Spaces inside an option need a backslash or double
quotes:

```sh
HUGSFLAGS='+k -E"vi +%d %s"'
export HUGSFLAGS
```

### `HUGSDIR`

The library directory.  Default: the directory chosen at build time,
`lib/hugs` under the prefix.  It is substituted for `{Hugs}` in the
search path and must contain `packages/`, and for `ffihugs` also
`include/HsFFI.h`.  Set it to `build/libraries/hugsdir` to run from
the build tree.

### `HOME`

The home directory.  Substituted for `{Home}` in the search path and
for `~` in the argument of `:cd`.

### `EDITOR`

The editor run by `:edit` and `:find` when no `-E` option is given.

### `SHELL`

The shell run by `:!`.  Default: `/bin/sh`.

### `EMACS`

If set, Hugs assumes it is running in an Emacs shell buffer and reads
its input line by line without stripping carriage returns.

### `CC`

Read by meson at configuration time, not by Hugs.  The compiler it
names is also the one `ffihugs` runs.  See
[Build and install Hugs](../how-to/build-and-install.md).

## See also

- [Options](options.md)
