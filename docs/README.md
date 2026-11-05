# Cupid documentation

Cupid is a small Haskell interpreter, written in C and descended from Hugs.
It implements Haskell 2010 with documented deviations and a fixed set of
extensions, [the language reference](reference/language.md) says which, its
libraries are a subset of GHC's base, and it evaluates expressions at an
interactive prompt or runs a program's `main` from a script.  It's also small
enough to read, and these pages are for both kinds of reader.

## Start here

New to Cupid?  [Install a release](start/install-a-release.md), then work
through [Getting started with Cupid](start/getting-started.md): it evaluates
a few expressions, loads a module and runs a program with `cupid`.  To build
from source instead, see [Build Cupid on Linux and
macOS](start/build-on-linux-and-macos.md) or [Build Cupid on
Windows](start/build-on-windows.md).

## Using Cupid

Guides to a particular task:

- [Write a script that runs with cupid](how-to/write-a-script.md)
- [Call C functions from Haskell](how-to/call-c-functions.md)
- [Make `:edit` open your editor](how-to/configure-the-editor.md)
- [Preprocess source files before loading](how-to/preprocess-sources.md)
- [Add a library to the search path](how-to/add-a-library.md)
- [Measure time and memory use](how-to/measure-time-and-memory.md)

## Inside Cupid

[A guided tour of the source](inside/README.md).  Begin with [Reading Cupid
in a day](inside/reading-cupid-in-a-day.md), which gives a route through the
main stages for a first day, and the order for the whole tour after that.

## Reference

Running it:

- [Running cupid](reference/cupid.md) — the command: prompt, run a file,
  load with `-i`, compile stubs with `--ffi`
- [Interpreter commands](reference/commands.md) — `:load`, `:type`, `:set`
  and the rest
- [Options](reference/options.md) — toggles and parameters, and where they
  are read from
- [Environment variables](reference/environment-variables.md)
- [Build options and install layout](reference/build-options.md)

The language and libraries:

- [The language Cupid accepts](reference/language.md) — what the language
  is, the deviations a program is most likely to meet, and known bugs
- [Language extensions](reference/extensions.md) — each feature beyond
  Haskell 2010, and what is not accepted
- [Haskell 2010 conformance](reference/haskell-2010.md) — the Report chapter
  by chapter, language and libraries
- [The libraries](reference/libraries.md) — packages and modules shipped

Background:

- [Differences from Hugs 98](reference/differences-from-hugs.md) — the
  programs, options, variables, modules and extensions that changed
- [Further reading](reference/further-reading.md) — the Report, the Hugs
  manuals and the papers the implementation follows

## Contributing

- [Check every build configuration](contributing/check-every-configuration.md)
- [Run the static analysis](contributing/run-clang-tidy.md)
- [Make a release](contributing/make-a-release.md)

## Manual page

`cupid.1.in` is the source of the `cupid(1)` manual page, which meson
fills in with the install paths and installs under `share/man`.
