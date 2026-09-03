# Hugs

Hugs is a small interpreter for Haskell, written in C.  This tree is a
revival of the September 2006 release of Hugs 98 with one aim: a simpler,
understandable Haskell implementation.  The language is Haskell 2010 plus
the type-system extensions the libraries need (multi-parameter classes,
functional dependencies, existential and rank-n types), always enabled.
Everything else from 2006 that did not serve that aim has been removed:
the autoconf build, the flag-guarded extensions, the shared GHC/nhc98
library sources and their three-stage bootstrap.

The code is standard C23 and builds with meson on 64-bit Linux, macOS and
the BSDs.  Windows is supported by design but not yet verified; see
[Build Hugs on Windows](docs/how-to/build-on-windows.md).

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

This compiles `hugs`, `runhugs` and `ffihugs` into `build/src`, then
assembles the Haskell libraries into `build/libraries/hugsdir`.  The
whole thing takes about a minute.  The source tree is never written to.

meson takes the compiler from `$CC`.  The same command is also baked
into `ffihugs`, which runs it during the library step, and again at run
time, to compile the C stubs of modules with foreign imports.  A compiler
wrapper such as `sccache cc` can therefore break the library step even
when the C sources compiled fine; the symptom is `ffihugs` reporting
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
`HUGSDIR` at the built library directory:

```sh
HUGSDIR=build/libraries/hugsdir build/src/hugs
Hugs> [1..10]
Hugs> :quit
```

or pass a search path to `runhugs` for a script:

```sh
build/src/runhugs -P'build/libraries/hugsdir/packages/*' Main.hs
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
`lib/hugs` (or `lib64/hugs` where the distribution uses it), the manual
page in `share/man`.  Set the prefix with `meson setup build
--prefix=DIR` and move the library directory with `-Dhugsdir=DIR`.
`DESTDIR` is honoured for staged installs.  An installed `hugs` finds
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
[Build and install Hugs](docs/how-to/build-and-install.md) and
[Build options and install layout](docs/reference/build-options.md).

## Layout

- `src/` — the interpreter: lexer, parser (`parser.y`), type checker,
  compiler to bytecode, evaluator, storage, and the `hugs`, `runhugs`
  and `ffihugs` front ends
- `libraries/` — the Haskell libraries Hugs ships: `hugsbase` (the
  Prelude and Hugs-specific modules), `base` and `haskell98`, all plain
  Haskell; `tools/` holds the build script and the generator for the
  platform-dependent module `Hugs.SysConfig`
- `docs/` — the documentation: a tutorial, how-to guides, reference
  and explanation pages, and the manual page
- `tests/` — the regression tests and the script that runs them
- `demos/` — example programs, installed alongside the libraries

## Documentation

- [docs/README.md](docs/README.md) — the index of the documentation
- [Getting started with Hugs](docs/tutorials/getting-started.md) — a
  first session, from build to running a program
- [Interpreter commands](docs/reference/commands.md) and
  [Options](docs/reference/options.md)
- [Call C functions from Haskell](docs/how-to/call-c-functions.md) —
  the foreign function interface and `ffihugs`

## License

Hugs 98 is Copyright (c) Mark P Jones, Alastair Reid, the Yale Haskell
Group, and the OGI School of Science & Engineering at OHSU, 1994-2005.
It is distributed under the BSD-style license in [License](License).
The contributors are listed in [Credits](Credits).
