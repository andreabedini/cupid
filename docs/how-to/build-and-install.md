# Build and install Cuddles

Build the interpreter and its libraries from source with meson and
install them under a prefix.

## Prerequisites

- a C compiler: gcc or clang (the sources are standard C23)
- meson 1.1 or later, and ninja
- python3 (meson needs it anyway; the library build script uses it too)
- optionally readline or libedit, for command-line editing at the prompt
- bison, which generates the parser from `src/parser.y` at build time
- optionally hp2ps, to view heap profiles from a profiling build

Cuddles builds on x86-64 and ARM64 machines running a current Linux, macOS
or BSD.  Windows is supported by design but not yet verified; see
[Build Cuddles on Windows](build-on-windows.md).

## Steps

### 1. Configure

```sh
CC=gcc meson setup build --prefix=/usr/local
```

meson takes the compiler from `CC`.  Name the real compiler, not a
wrapper such as `sccache cc`: the same command is baked into `fficuddles`,
which runs it during the library build and again at run time to compile
the C stubs of modules with foreign imports.  A wrapper that does not
work as a plain command breaks the library step even though the C
sources compiled fine.

meson remembers the compiler it found.  To change it in an existing
build directory, wipe the configuration:

```sh
CC=clang meson setup --wipe build
```

A plain `--reconfigure` keeps the old compiler.

Build options are set with `-Dname=value` on this command line, or
later with `meson configure build`.  They are listed in
[Build options and install layout](../reference/build-options.md).

### 2. Build

```sh
ninja -C build
```

This compiles `cuddles`, `runcuddles` and `fficuddles` into `build/src`, then
assembles the Haskell libraries into `build/libraries/cuddlesdir`.  The
source tree is not written to.  The whole thing takes about a minute.

### 3. Install

```sh
ninja -C build install
```

With the default prefix this puts the binaries in `/usr/local/bin`, the
libraries in `/usr/local/lib/cuddles` (or `lib64/cuddles` where the
distribution uses it) and the manual page in `/usr/local/share/man`.
`DESTDIR` is honoured for staged installs:

```sh
DESTDIR=/tmp/stage ninja -C build install
```

An installed `cuddles` finds its libraries without any environment
variable.

## Verify it worked

```sh
/usr/local/bin/cuddles
```

At the prompt:

```
Cuddles> [1..10]
[1,2,3,4,5,6,7,8,9,10]
Cuddles> :quit
```

Then run one of the installed demos:

```sh
cuddles /usr/local/lib/cuddles/demos/Say.hs
```

```
Say> putStr (say "Cuddles")
```

## Troubleshooting

### `fficuddles` reports "Error while running compilation command"

**Cause**: `CC` named a compiler wrapper that cannot be run as a plain
command in the library build.

**Fix**: reconfigure with the real compiler:

```sh
CC=gcc meson setup --wipe build
```

### `readline was requested but neither readline nor libedit was found`

**Cause**: `-Dreadline=enabled` was given but no line-editing library is
installed.

**Fix**: install the readline or libedit development package, or drop
the option; the default `auto` uses a library if one is found and does
without otherwise.

## See also

- [Run Cuddles from the build tree](run-from-the-build-tree.md) if you do
  not want to install
- [Build options and install layout](../reference/build-options.md)
- [Run the static analysis](run-clang-tidy.md)
