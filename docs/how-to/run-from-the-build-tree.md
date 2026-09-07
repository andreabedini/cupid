# Run Cuddles from the build tree

Use a freshly built Cuddles without installing it.

## Prerequisites

- a completed `ninja -C build` (see
  [Build and install Cuddles](build-and-install.md))

## Steps

The interpreter needs to know where the libraries are.  The build
puts them in `build/libraries/cuddlesdir`, and the binaries in
`build/src`.

### Interactive session

Point `CUDDLESDIR` at the built library directory:

```sh
CUDDLESDIR=build/libraries/cuddlesdir build/src/cuddles
```

### A script

Either set `CUDDLESDIR` in the same way, or pass the library search path
with `-P`:

```sh
build/src/runcuddles -P'build/libraries/cuddlesdir/packages/*' Main.hs
```

### For the rest of a shell session

Export the two settings once:

```sh
export PATH="$PWD/build/src:$PATH"
export CUDDLESDIR="$PWD/build/libraries/cuddlesdir"
```

After this `cuddles`, `runcuddles` and `fficuddles` work as if installed.

## Verify it worked

```sh
CUDDLESDIR=build/libraries/cuddlesdir build/src/cuddles
```

```
Cuddles> :load Data.Map
Data.Map> fromList [(1,"one")]
fromList [(1,"one")]
Data.Map> :quit
```

If the libraries cannot be found the interpreter stops before the
prompt with a message that the `Prelude` cannot be located.

## See also

- [Environment variables](../reference/environment-variables.md) for
  `CUDDLESDIR`
- [Options](../reference/options.md) for `-P`
