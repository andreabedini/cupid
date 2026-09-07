# Cuddles

Cuddles is a small interpreter for Haskell, written in C.  It is a fork
of Hugs 98, the Haskell interpreter written by Mark P Jones and
developed at Yale, OGI and by a group of volunteers between 1994 and
2006, and it starts from the September 2006 release, the last one made.

Cuddles has one aim: a simpler, understandable Haskell implementation.
Three things follow from it and will not change:

- **It is an interpreter written in C.**  Source text is compiled to
  the code of a small abstract machine and run in the same process.
- **It does not self-host.**  No part of the implementation is written
  in Haskell except the libraries it ships.
- **It stays small.**  The language is Haskell 2010 plus the
  type-system extensions the libraries need (multi-parameter classes,
  functional dependencies, existential and rank-n types), always
  enabled.  Anything that does not serve the aim is deleted rather
  than kept behind a flag.

Much of the 2006 tree has already gone: the autoconf build, the
flag-guarded extensions, the shared GHC/nhc98 library sources and their
three-stage bootstrap, the dead platforms.
[History](docs/explanation/history.md) tells where the code came from.

The code is standard C23 and builds with meson on 64-bit Linux, macOS and
the BSDs.  Windows is supported by design but not yet verified; see
[Build on Windows](docs/how-to/build-on-windows.md).

## Building

You need:

- a C compiler: gcc or clang
- meson 1.1 or later, and ninja
- python3 (meson needs it anyway; the library build script uses it too)
- optionally readline or libedit, for command-line editing at the prompt

Then:

```sh
meson setup build
ninja -C build
```

This compiles `cuddles`, `runcuddles` and `fficuddles` into `build/src`, then
assembles the Haskell libraries into `build/libraries/cuddlesdir`.  The
whole thing takes about a minute.  The source tree is never written to.

meson takes the compiler from `$CC`.  The same command is also baked
into `fficuddles`, which runs it during the library step, and again at run
time, to compile the C stubs of modules with foreign imports.  A compiler
wrapper such as `sccache cc` can therefore break the library step even
when the C sources compiled fine; the symptom is `fficuddles` reporting
"Error while running compilation command".  Name the real compiler
instead:

```sh
CC=gcc meson setup build
```

meson remembers the compiler it found, so to change it in an existing
build directory use `CC=gcc meson setup --wipe build`.  A plain
`--reconfigure` keeps the old one.

### Running from the build tree

The interpreter needs to know where the libraries are.  Either point
`CUDDLESDIR` at the built library directory:

```sh
CUDDLESDIR=build/libraries/cuddlesdir build/src/cuddles
Cuddles> [1..10]
Cuddles> :quit
```

or pass a search path to `runcuddles` for a script:

```sh
build/src/runcuddles -P'build/libraries/cuddlesdir/packages/*' Main.hs
```

### Testing

```sh
meson test -C build
```

runs the regression tests under `tests/`.  Each `NAME.hs` there is a
program whose output is compared with `NAME.stdout`, or a session with
the interpreter when `NAME.stdin` exists; the comment at the top of
`tests/run-tests` describes the other files a test may have.  Add a
test for every bug you fix.

### Installing

```sh
ninja -C build install
```

The default prefix is `/usr/local`: binaries in `bin`, libraries in
`lib/cuddles` (or `lib64/cuddles` where the distribution uses it), the manual
page in `share/man`.  Set the prefix with `meson setup build
--prefix=DIR` and move the library directory with `-Dcuddlesdir=DIR`.
`DESTDIR` is honoured for staged installs.  An installed `cuddles` finds
its libraries without any environment variable.

### Options

Options are set with `-Dname=value` at setup time or later with
`meson configure build`, which also lists them.  The useful ones:

| Option | Default | Meaning |
|---|---|---|
| `readline` | `auto` | Command-line editing with readline or libedit |
| `char-encoding` | `utf8` | Encoding for character I/O: `utf8`, `locale` or `none` |
| `libraries` | `true` | Build the Haskell libraries |
| `profiling` | `false` | Heap profiler; view `profile.hp` with hp2ps |
| `debug-code` | `false` | Assertions and bytecode debugging |
| `small-banner` | `false` | One-line startup banner |

The full list, and more detail on every step, is in
[Build and install](docs/how-to/build-and-install.md) and
[Build options and install layout](docs/reference/build-options.md).

## Layout

- `src/` — the interpreter: lexer, parser (`parser.y`), type checker,
  compiler to bytecode, evaluator, storage, and the `cuddles`, `runcuddles`
  and `fficuddles` front ends
- `libraries/` — the Haskell libraries Cuddles ships: `cuddles` (the
  Prelude and interpreter-specific modules) and `base`,
  all plain Haskell; `tools/` holds the build script and the generator
  for the platform-dependent module `Cuddles.SysConfig`
- `docs/` — the documentation: a tutorial, how-to guides, reference
  and explanation pages, and the manual page
- `tests/` — the regression tests and the script that runs them
- `demos/` — example programs, installed alongside the libraries

## Documentation

- [docs/README.md](docs/README.md) — the index of the documentation
- [Getting started](docs/tutorials/getting-started.md) — a first
  session, from build to running a program
- [How a program is run](docs/explanation/how-cuddles-runs-a-program.md)
  — the pipeline from source text to the abstract machine
- [Interpreter commands](docs/reference/commands.md) and
  [Options](docs/reference/options.md)
- [Call C functions from Haskell](docs/how-to/call-c-functions.md) —
  the foreign function interface and `fficuddles`

## Origin and license

Cuddles is derived from Hugs 98.  Hugs 98 is Copyright (c) Mark P Jones,
Alastair Reid, the Yale Haskell Group, and the OGI School of Science &
Engineering at OHSU, 1994-2005, and is distributed under the BSD-style
license in [License](License).  Cuddles is distributed under the same
license.  The people who built Hugs 98 are listed in
[Credits](Credits); the design of the type checker, the abstract
machine and the libraries is theirs, and this tree would not exist
without their work.
