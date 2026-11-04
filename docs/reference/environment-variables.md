# Environment variables

Variables that `cupid` consults.

### `CUPIDFLAGS`

Options, in the syntax of [Options](options.md), read before the
command line.  Spaces inside an option need a backslash or double
quotes:

```sh
CUPIDFLAGS='+k -E"vi +%d %s"'
export CUPIDFLAGS
```

### `CUPIDDIR`

The library directory.  It is substituted for `{Cupid}` in the search
path and must contain `packages/`, and with `--ffi` also
`include/HsFFI.h`.  Set it to `build/libraries/cupiddir` to run from the
build tree.

Default: on Linux, macOS and Windows, `../libdir/cupid` relative to the
running executable, if that has a `packages/` directory, so that a moved
installation such as an unpacked release archive still finds its
libraries.  Otherwise, and always when the build set `-Dcupiddir`, the
directory chosen at build time, by default `libdir/cupid` under the
prefix.

### `HOME`

The home directory.  Substituted for `{Home}` in the search path and
for a leading `~` in the arguments of `:cd`, `:load`, `:also` and
`:edit`.

### `EDITOR`

The editor run by `:edit` and `:find` when no `-E` option is given.

### `SHELL`

On Unix, the shell that `:!` starts when it is given no command.
Default: `/bin/sh`.  A command given to `:!` is run with `/bin/sh -c`
whatever this says.  Not read on Windows.

### `CC`

Read by meson at configuration time, not by Cupid.  The compiler it
names is also the one `cupid --ffi` runs.  See
[Build Cupid on Linux and macOS](../install/build-on-linux-and-macos.md).

### `LC_ALL`, `LC_CTYPE`, `LANG`

Choose the encoding of character I/O, only when Cupid was built with
`-Dchar-encoding=locale`.

## See also

- [Options](options.md)
