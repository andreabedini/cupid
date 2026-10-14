<p align="center">
  <img src="assets/logo-wordmark.svg" alt="Cupid" width="360">
</p>

<p align="center">
  <strong>A small Haskell interpreter, written in C.</strong><br>
  <em>Hugs 98, taken apart and put back together so you can read it.</em>
</p>

---

Cupid is a fork of Hugs 98 — the interpreter Mark P Jones started in
1994 and a group of volunteers kept alive until 2006.  It picks up from
the September 2006 release, the last one that shipped, and takes it
somewhere Hugs itself was not going.

The aim is one sentence: **a Haskell implementation you can sit down and
read.**  Not the fastest one, not the most complete one.  The one where
you can follow an expression from the characters you typed to the value
that comes back, and never lose the thread.

Three things follow from that, and they don't change:

- **It is an interpreter written in C.**  Source text is compiled to the
  code of a small abstract machine and run in the same process.  No
  native code generation, no linker, no separate compilation step.
- **It does not self-host.**  Nothing in the implementation is written in
  Haskell except the libraries it ships.  You need a C compiler and
  nothing else, and the thing you are reading is the thing that runs.
- **It stays small.**  Haskell 2010, plus the type-system extensions the
  libraries actually need — multi-parameter classes, functional
  dependencies, existentials, rank-n types — always on.  Anything that
  doesn't serve the aim gets deleted rather than parked behind a flag.

That third one does most of the work.  A flag is a promise to keep two
code paths working forever, and a tree full of them is exactly the tree
you can't read.  So when something has to go, it goes.

## Where it's at

A fair amount of the 2006 tree has already gone: the autoconf build, the
flag-guarded extensions, the GHC/nhc98 shared library sources and the
three-stage bootstrap that turned them into something Hugs could load,
the platforms nobody has run this century.  None of that was deleted on
a whim: [The design of Cupid](docs/explanation/design.md) says what
stayed and why.  If you want the longer story of where the code came
from, that's [History](docs/explanation/history.md).

It builds and runs.  The code is standard C23 and builds with meson on
64-bit Linux, macOS and the BSDs.  Windows is supported by design, but I
haven't actually verified it — see
[Build on Windows](docs/how-to/build-on-windows.md) if you'd like to.

## Quick start

You need a C compiler, meson 1.1 or later, ninja and python3.  Then:

```sh
CC=gcc meson setup build
ninja -C build
CUPIDDIR=build/libraries/cupiddir build/src/cupid
```

```
Cupid> map (*2) [1..10]
[2,4,6,8,10,12,14,16,18,20]
Cupid> :quit
```

The whole build takes about a minute and never writes to the source
tree.  Two things worth knowing up front:

- Name a **real** compiler in `$CC`.  meson bakes the command into
  `fficupid`, which runs it at library-build time and again at run time
  to compile the C stubs of modules with foreign imports.  A wrapper like
  `sccache cc` breaks that step even when the C sources compiled fine;
  the symptom is `fficupid` reporting "Error while running compilation
  command".
- meson remembers the compiler it found, so changing it in an existing
  build directory needs `meson setup --wipe build`.  A plain
  `--reconfigure` keeps the old one.

Tests are `meson test -C build`, and installing is `ninja -C build
install`.  [Build and install](docs/how-to/build-and-install.md) has the
detail, and [Build options and install layout](docs/reference/build-options.md)
lists every `-Dname=value` you can set.

## Documentation

The docs follow [Diátaxis](https://diataxis.fr/), so where a page lives
tells you what it's for.  [docs/README.md](docs/README.md) is the index;
the useful starting points are:

- [Getting started](docs/tutorials/getting-started.md) — a first session,
  from build to running a program.  About 20 minutes, most of it the build.
- [How Cupid runs a program](docs/explanation/how-cupid-runs-a-program.md)
  — source text to abstract machine, and where each piece lives.  Read
  this before the source.
- [Interpreter commands](docs/reference/commands.md) and
  [Options](docs/reference/options.md) — what you can type at the prompt
  and what you can set.
- [The language Cupid accepts](docs/reference/language.md) — Haskell 2010
  conformance, and the deviations and bugs that are known.
- [Call C functions from Haskell](docs/how-to/call-c-functions.md) — the
  FFI and `fficupid`.
- [The design of Cupid](docs/explanation/design.md) — why the tree
  looks the way it does.

## Layout

- `src/` — the interpreter: lexer, parser (`parser.y`), type checker,
  bytecode compiler, evaluator, storage, and the `cupid`, `runcupid`
  and `fficupid` front ends
- `libraries/` — the Haskell it ships: `cupid` (the Prelude and the
  interpreter-specific modules) and `base`, all plain Haskell.  `tools/`
  holds the build script and the generator for `Cupid.SysConfig`
- `docs/` — tutorial, how-to guides, reference, explanation and the
  manual page
- `tests/` — the regression tests and the script that runs them
- `demos/` — example programs, installed alongside the libraries
- `www/` — the website, a [Zola](https://www.getzola.org/) site that
  builds from `docs/`

Add a test for every bug you fix.  Each `tests/NAME.hs` is a program
whose output is compared with `NAME.stdout`, or an interpreter session
when `NAME.stdin` exists; the comment at the top of `tests/run-tests`
describes the rest.

## About the name

It's a fork of Hugs, so it needed a name in the same family.  Hugs was
first released on Valentine's Day, 14 February 1995, which made Cupid
the obvious choice.  The grin on the logo is his: Cupid has always been
a bit of a troublemaker.

## Origin and license

Cupid is derived from Hugs 98, which is Copyright (c) Mark P Jones,
Alastair Reid, the Yale Haskell Group, and the OGI School of Science &
Engineering at OHSU, 1994-2005, and distributed under the BSD-style
license in [License](License).  Cupid is distributed under the same
license.

The people who built Hugs 98 are listed in [Credits](Credits).  The type
checker, the abstract machine and the libraries are their design, and
this tree would not exist without their work.  What's changed here is
mostly subtraction.
