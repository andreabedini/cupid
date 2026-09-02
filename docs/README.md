# Hugs documentation

Hugs is a small interpreter for Haskell, written in C.  It reads Haskell
2010 source, with a few type-system extensions, and evaluates expressions
at an interactive prompt or runs a program's `main` from a script.

The documentation follows the [Diátaxis](https://diataxis.fr/) scheme:
tutorials teach, how-to guides solve a task, reference pages describe,
and explanations discuss.

## Start here

New to Hugs?  Work through
[Getting started with Hugs](tutorials/getting-started.md).  It builds
the interpreter, evaluates a few expressions, loads a module and runs a
program with `runhugs`.

## Tutorials

- [Getting started with Hugs](tutorials/getting-started.md) — build the
  interpreter, use the prompt, load a module, run a program.

## How-to guides

- [Build and install Hugs](how-to/build-and-install.md)
- [Run Hugs from the build tree](how-to/run-from-the-build-tree.md)
- [Build Hugs on Windows](how-to/build-on-windows.md)
- [Write a script that runs with runhugs](how-to/write-a-script.md)
- [Call C functions from Haskell](how-to/call-c-functions.md)
- [Make `:edit` open your editor](how-to/configure-the-editor.md)
- [Preprocess source files before loading](how-to/preprocess-sources.md)
- [Add a library to the search path](how-to/add-a-library.md)
- [Measure time and memory use](how-to/measure-time-and-memory.md)
- [Embed the interpreter in a C program](how-to/embed-hugs.md)
- [Run the static analysis](how-to/run-clang-tidy.md)

## Reference

- [Interpreter commands](reference/commands.md) — `:load`, `:type`,
  `:set` and the rest
- [Options](reference/options.md) — toggles and parameters, and where
  they are read from
- [Environment variables](reference/environment-variables.md)
- [runhugs](reference/runhugs.md)
- [ffihugs](reference/ffihugs.md)
- [Build options and install layout](reference/build-options.md)
- [The language Hugs accepts](reference/language.md) — Haskell 2010
  conformance, known deviations and bugs
- [Language extensions](reference/extensions.md) — what Hugs accepts
  beyond Haskell 2010
- [The libraries](reference/libraries.md) — packages and modules shipped
- [The server API](reference/server-api.md) — the C interface used by
  `runhugs` and by embedding programs

## Explanation

- [How Hugs runs a program](explanation/how-hugs-runs-a-program.md) —
  the stages from source to result, and where the pieces live
- [Modules, files and the search path](explanation/modules-and-the-search-path.md)
- [Character encoding](explanation/character-encoding.md)
- [Foreign import wrappers](explanation/foreign-import-wrappers.md) —
  how callbacks into Haskell were implemented, and why they are not
  available on current targets
- [History](explanation/history.md) — from Gofer to this tree

## Manual page

`hugs.1.in` is the source of the `hugs(1)` manual page, which meson
fills in with the install paths and installs alongside the binaries.
