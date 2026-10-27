# Build Cupid on Linux and macOS

Build the interpreter and its libraries from source with meson, and
either install them under a prefix or run them from the build tree.
The steps are the same on Linux and macOS; only the package manager and
one detail about bison differ.  The BSDs build the same way with their
own packages, but are not tested on every change.

For Windows, see [Build Cupid on Windows](build-on-windows.md).  If you
only want to run Cupid, [Install a release](install-a-release.md) is
quicker.

## Prerequisites

- a C compiler: gcc 13 or later, or clang (the sources are standard C23)
- meson 1.1 or later, and ninja
- python3 (meson needs it anyway; the library build script uses it too)
- bison, which generates the parser from `src/parser.y` at build time
- optionally readline or libedit, for command-line editing at the prompt
- optionally hp2ps, to view heap profiles from a profiling build
- git, to get the sources

On Fedora:

```sh
sudo dnf install gcc meson ninja-build python3 bison git
```

On Debian 13 or Ubuntu 24.04 and later:

```sh
sudo apt install gcc meson ninja-build python3 bison git
```

Older releases ship a meson older than 1.1; install a newer one with
`python3 -m pip install --user meson`.  They also ship a gcc older than
13, which cannot compile Cupid's C23 sources (Ubuntu 22.04 has gcc 11).
Install gcc 13 or later and name it in `CC` below.

On macOS with Homebrew:

```sh
brew install meson ninja bison
export PATH="$(brew --prefix bison)/bin:$PATH"
```

The bison that macOS ships is too old for Cupid's grammar, so the
Homebrew one has to come first on the path.  The C compiler is the one
Xcode's command line tools install; `CC=cc` below picks it up.

## Steps

### 1. Get the sources

```sh
git clone https://github.com/andreabedini/cupid.git
cd cupid
```

To build a released version instead of the current `main`, download
`cupid-VERSION.tar.gz` from the
[releases page](https://github.com/andreabedini/cupid/releases) and
unpack it; it holds the same tree.  Everything below is run from the
top of that tree.

### 2. Configure

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

### 3. Build

```sh
ninja -C build
```

This compiles `cupid`, `runcupid` and `fficupid` into `build/src`, then
assembles the Haskell libraries into `build/libraries/cupiddir`.  The
source tree is not written to.  The whole thing takes a minute or two,
and the last lines of output look like this:

```
[52/53] Generating libraries/cupid-libraries with a custom command
fficupid cupid/Cupid/Storable.hs
...
fficupid base/Data/ByteString/Base.hs
```

To run the regression tests on what you built:

```sh
meson test -C build
```

### 4. Install

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
variable.  It still finds them after the whole installation tree has
been moved, because it looks next to its own executable first.

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

## Run it from the build tree

You do not have to install to use what you built.  The interpreter
only needs to know where the libraries are: the build puts them in
`build/libraries/cupiddir`, and the binaries in `build/src`.

For an interactive session, point `CUPIDDIR` at the built library
directory:

```sh
CUPIDDIR=build/libraries/cupiddir build/src/cupid
```

For a script, either set `CUPIDDIR` in the same way, or pass the
library search path with `-P`:

```sh
build/src/runcupid -P'build/libraries/cupiddir/packages/*' Main.hs
```

For the rest of a shell session, export the two settings once:

```sh
export PATH="$PWD/build/src:$PATH"
export CUPIDDIR="$PWD/build/libraries/cupiddir"
```

After this `cupid`, `runcupid` and `fficupid` work as if installed.  If
the libraries cannot be found, the interpreter stops before the prompt
with a message that the `Prelude` cannot be located.
[Environment variables](../reference/environment-variables.md)
describes `CUPIDDIR`, and [Options](../reference/options.md) describes
`-P`.

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

### The build stops in `parser.y` with a bison error

**Cause**: the bison on the path is too old for the grammar.  On macOS
this is the one the system ships.

**Fix**: install bison from Homebrew and put it first on the path, as
in the prerequisites.

## See also

- [Install a release](install-a-release.md), for a build that is ready
  to unpack
- [Build Cupid on Windows](build-on-windows.md)
- [Build options and install layout](../reference/build-options.md)
- [Run the static analysis](../contributing/run-clang-tidy.md)
