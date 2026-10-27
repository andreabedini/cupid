# Cupid documentation

Cupid is a small interpreter for Haskell, written in C.  It reads
Haskell 2010 with the parts of modern Haskell that real code needs, its
libraries follow GHC's base, and it evaluates expressions at an
interactive prompt or runs a program's `main` from a script.

The documentation follows the [Diátaxis](https://diataxis.fr/) scheme:
tutorials teach, how-to guides solve a task, explanations discuss,
and reference pages describe.  Before them comes Install, and after them
Contributing, for the people who change Cupid rather than use it.

## Start here

New to Cupid?  [Install a release](install/install-a-release.md), then
work through [Getting started with Cupid](tutorials/getting-started.md):
it evaluates a few expressions, loads a module and runs a program with
`runcupid`.

## Install

- [Install a release](install/install-a-release.md) — one command, or
  the same steps by hand
- [Build Cupid on Linux and macOS](install/build-on-linux-and-macos.md)
  — from source, with meson; installed or run from the build tree
- [Build Cupid on Windows](install/build-on-windows.md) — the native
  MinGW-w64 build

## Tutorials

- [Getting started with Cupid](tutorials/getting-started.md) — use the
  prompt, load a module, run a program.

## How-to guides

- [Write a script that runs with runcupid](how-to/write-a-script.md)
- [Call C functions from Haskell](how-to/call-c-functions.md)
- [Make `:edit` open your editor](how-to/configure-the-editor.md)
- [Preprocess source files before loading](how-to/preprocess-sources.md)
- [Add a library to the search path](how-to/add-a-library.md)
- [Measure time and memory use](how-to/measure-time-and-memory.md)

## Explanation

- [The design of Cupid](explanation/design.md) — the aim, and the choices
  in the language, the build and the C code that follow from it
- [How Cupid runs a program](explanation/how-cupid-runs-a-program.md) —
  the stages from source to result, and where the pieces live
- [Cells, text and tables](explanation/cells-text-and-tables.md) — the
  one data type everything is built from, the heap, the collector and
  the checkpoints that reloading rests on
- [How source is read](explanation/how-source-is-read.md) — the lexer,
  the grammar's shortcuts, and the static analysis that turns equations
  into binding groups
- [How types are inferred](explanation/how-types-are-inferred.md) — the
  substitution, structure sharing, generalisation and the in-place
  rewriting that inserts dictionaries
- [How constraints are solved](explanation/how-constraints-are-solved.md) —
  predicates, evidence, entailment, improvement and the loop that runs
  them at each binding group
- [How code is compiled and run](explanation/how-code-is-compiled-and-run.md) —
  pattern-matching compilation, lambda lifting, the abstract machine and
  its instructions
- [Modules, files and the search path](explanation/modules-and-the-search-path.md)
- [Character encoding](explanation/character-encoding.md)
- [Foreign import wrappers](explanation/foreign-import-wrappers.md) —
  how callbacks into Haskell were implemented, and why they are not
  available on current targets
- [History](explanation/history.md) — from Gofer to this tree

## Reference

- [Interpreter commands](reference/commands.md) — `:load`, `:type`,
  `:set` and the rest
- [Options](reference/options.md) — toggles and parameters, and where
  they are read from
- [Environment variables](reference/environment-variables.md)
- [runcupid](reference/runcupid.md)
- [fficupid](reference/fficupid.md)
- [Build options and install layout](reference/build-options.md)
- [The language Cupid accepts](reference/language.md) — Haskell 2010
  conformance, known deviations and bugs
- [Haskell 2010 conformance](reference/haskell-2010.md) — the Report
  chapter by chapter, language and libraries
- [Language extensions](reference/extensions.md) — what Cupid accepts
  beyond Haskell 2010, and what it leaves out
- [The libraries](reference/libraries.md) — packages and modules shipped

## Contributing

- [Check every build configuration](contributing/check-every-configuration.md)
- [Run the static analysis](contributing/run-clang-tidy.md)
- [Make a release](contributing/make-a-release.md)

## Manual page

`cupid.1.in` is the source of the `cupid(1)` manual page, which meson
fills in with the install paths and installs alongside the binaries.
