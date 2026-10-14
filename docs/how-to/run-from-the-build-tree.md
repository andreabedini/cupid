# Run Cupid from the build tree

Use a freshly built Cupid without installing it.

## Prerequisites

- a completed `ninja -C build` (see
  [Build and install Cupid](build-and-install.md))

## Steps

The interpreter needs to know where the libraries are.  The build
puts them in `build/libraries/cupiddir`, and the binaries in
`build/src`.

### Interactive session

Point `CUPIDDIR` at the built library directory:

```sh
CUPIDDIR=build/libraries/cupiddir build/src/cupid
```

### A script

Either set `CUPIDDIR` in the same way, or pass the library search path
with `-P`:

```sh
build/src/runcupid -P'build/libraries/cupiddir/packages/*' Main.hs
```

### For the rest of a shell session

Export the two settings once:

```sh
export PATH="$PWD/build/src:$PATH"
export CUPIDDIR="$PWD/build/libraries/cupiddir"
```

After this `cupid`, `runcupid` and `fficupid` work as if installed.

## Verify it worked

```sh
CUPIDDIR=build/libraries/cupiddir build/src/cupid
```

```
Cupid> :load Data.Map
Data.Map> fromList [(1,"one")]
fromList [(1,"one")]
Data.Map> :quit
```

If the libraries cannot be found the interpreter stops before the
prompt with a message that the `Prelude` cannot be located.

## See also

- [Environment variables](../reference/environment-variables.md) for
  `CUPIDDIR`
- [Options](../reference/options.md) for `-P`
