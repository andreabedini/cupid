# Cupid documentation

Cupid is a small interpreter for Haskell, written in C.  It reads Haskell
2010 source, with a few type-system extensions, and evaluates expressions
at an interactive prompt or runs a program's `main` from a script.

The documentation follows the [Diátaxis](https://diataxis.fr/) scheme:
tutorials teach, how-to guides solve a task, reference pages describe,
and explanations discuss.

## Start here

New to Cupid?  Work through
[Getting started with Cupid](tutorials/getting-started.md).  It builds
the interpreter, evaluates a few expressions, loads a module and runs a
program with `runcupid`.

## Tutorials

- [Getting started with Cupid](tutorials/getting-started.md) — build the
  interpreter, use the prompt, load a module, run a program.

## How-to guides

- [Install a release](how-to/install-a-release.md)
- [Build and install Cupid](how-to/build-and-install.md)
- [Run Cupid from the build tree](how-to/run-from-the-build-tree.md)
- [Build Cupid on Windows](how-to/build-on-windows.md)
- [Write a script that runs with runcupid](how-to/write-a-script.md)
- [Call C functions from Haskell](how-to/call-c-functions.md)
- [Make `:edit` open your editor](how-to/configure-the-editor.md)
- [Preprocess source files before loading](how-to/preprocess-sources.md)
- [Add a library to the search path](how-to/add-a-library.md)
- [Measure time and memory use](how-to/measure-time-and-memory.md)
- [Run the static analysis](how-to/run-clang-tidy.md)
- [Make a release](how-to/make-a-release.md)

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
  beyond Haskell 2010
- [The libraries](reference/libraries.md) — packages and modules shipped

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

## Manual page

`cupid.1.in` is the source of the `cupid(1)` manual page, which meson
fills in with the install paths and installs alongside the binaries.
