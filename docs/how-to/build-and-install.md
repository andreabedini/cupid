# Build and install Cupid

Build the interpreter and its libraries from source with meson and
install them under a prefix.

## Prerequisites

- a C compiler: gcc 13 or later, or clang (the sources are standard C23)
- meson 1.1 or later, and ninja
- python3 (meson needs it anyway; the library build script uses it too)
- optionally readline or libedit, for command-line editing at the prompt
- bison, which generates the parser from `src/parser.y` at build time;
  on Windows, WinFlexBison's `win_bison` will do
- optionally hp2ps, to view heap profiles from a profiling build

Cupid builds on x86-64 and ARM64 machines running a current Linux, macOS
or BSD, and on x86-64 Windows; for Windows, see
[Build Cupid on Windows](build-on-windows.md).

## Steps

### 1. Configure

```sh
CC=gcc meson setup build --prefix=/usr/local
```

meson takes the compiler from `CC`.  Name the real compiler, not a
wrapper such as `sccache cc`: the same command is baked into `fficupid`,
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

This compiles `cupid`, `runcupid` and `fficupid` into `build/src`, then
assembles the Haskell libraries into `build/libraries/cupiddir`.  The
source tree is not written to.  The whole thing takes a minute or two.

### 3. Install

```sh
ninja -C build install
```

With the default prefix this puts the binaries in `/usr/local/bin`, the
libraries in `/usr/local/lib/cupid` (or `lib64/cupid` where the
distribution uses it) and the manual page in `/usr/local/share/man`.
`DESTDIR` is honoured for staged installs:

```sh
DESTDIR=/tmp/stage ninja -C build install
```

An installed `cupid` finds its libraries without any environment
variable.  On Linux, macOS and Windows it still finds them after the whole
installation tree has been moved, because it looks next to its own
executable first.

## Verify it worked

```sh
/usr/local/bin/cupid
```

At the prompt:

```
Cupid> [1..10]
[1,2,3,4,5,6,7,8,9,10]
Cupid> :quit
```

Then run one of the installed demos.  Where the distribution uses
`lib64`, the path is `/usr/local/lib64/cupid/demos/Say.hs`:

```sh
cupid /usr/local/lib/cupid/demos/Say.hs
```

```
Say> putStr (say "Cupid")

 CCCC  U   U  PPPP   IIIII  DDDD 
C      U   U  P   P    I    D   D
C      U   U  PPPP     I    D   D
C      U   U  P        I    D   D
 CCCC   UUU   P      IIIII  DDDD 
```

## Troubleshooting

### `fficupid` reports "Error while running compilation command"

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

- [Run Cupid from the build tree](run-from-the-build-tree.md) if you do
  not want to install
- [Build options and install layout](../reference/build-options.md)
- [Run the static analysis](run-clang-tidy.md)
