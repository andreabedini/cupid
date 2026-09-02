# Run Hugs from the build tree

Use a freshly built Hugs without installing it.

## Prerequisites

- a completed `ninja -C build` (see
  [Build and install Hugs](build-and-install.md))

## Steps

The interpreter needs to know where the libraries are.  The build
puts them in `build/libraries/hugsdir`, and the binaries in
`build/src`.

### Interactive session

Point `HUGSDIR` at the built library directory:

```sh
HUGSDIR=build/libraries/hugsdir build/src/hugs
```

### A script

Either set `HUGSDIR` in the same way, or pass the library search path
with `-P`:

```sh
build/src/runhugs -P'build/libraries/hugsdir/packages/*' Main.hs
```

### For the rest of a shell session

Export the two settings once:

```sh
export PATH="$PWD/build/src:$PATH"
export HUGSDIR="$PWD/build/libraries/hugsdir"
```

After this `hugs`, `runhugs` and `ffihugs` work as if installed.

## Verify it worked

```sh
HUGSDIR=build/libraries/hugsdir build/src/hugs
```

```
Hugs> :load Data.Map
Data.Map> fromList [(1,"one")]
fromList [(1,"one")]
Data.Map> :quit
```

If the libraries cannot be found the interpreter stops before the
prompt with a message that the `Prelude` cannot be located.

## See also

- [Environment variables](../reference/environment-variables.md) for
  `HUGSDIR`
- [Options](../reference/options.md) for `-P`
