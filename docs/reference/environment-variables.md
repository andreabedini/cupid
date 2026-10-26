# Environment variables

Variables that `cupid`, `runcupid` and `fficupid` consult.

### `CUPIDFLAGS`

Options, in the syntax of [Options](options.md), read before the
command line.  Spaces inside an option need a backslash or double
quotes:

```sh
CUPIDFLAGS='+k -E"vi +%d %s"'
export CUPIDFLAGS
```

### `CUPIDDIR`

The library directory.  Default: on Linux, macOS and Windows, the
library directory where the install put it relative to the running
executable (`../lib/cupid` from `bin`, `..\lib\cupid` on Windows), if
it has a `packages/` directory, so an installation that has been moved,
such as an unpacked release archive, still finds its libraries;
otherwise the directory chosen at build time, `libdir/cupid` under the
prefix (`lib/cupid` or `lib64/cupid`).  It is substituted
for `{Cupid}` in the search path and must contain `packages/`, and for
`fficupid` also `include/HsFFI.h`.  Set it to `build/libraries/cupiddir`
to run from the build tree.

### `HOME`

The home directory.  Substituted for `{Home}` in the search path and
for `~` in the argument of `:cd`.

### `EDITOR`

The editor run by `:edit` and `:find` when no `-E` option is given.

### `SHELL`

The shell that `:!` starts when it is given no command.  Default:
`/bin/sh`.  A command given to `:!` is always run with `/bin/sh -c`.

### `CC`

Read by meson at configuration time, not by Cupid.  The compiler it
names is also the one `fficupid` runs.  See
[Build and install Cupid](../how-to/build-and-install.md).

## See also

- [Options](options.md)
