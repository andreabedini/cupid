# How Hugs runs a program

Hugs is an interpreter in the old sense: it reads source text, compiles
it to an internal code, and runs that code in the same process, with
no files written in between.  This page follows a module from file to
result and says where each stage lives in `src/`.

## From text to code

**Reading.**  `input.c` turns a file into tokens.  Literate scripts are
unwrapped here, and the layout rule is applied here too: the lexer
inserts the braces and semicolons that indentation implies, so the
parser never sees layout.

**Parsing.**  `parser.y` is a bison grammar.  Its output is not an
abstract syntax tree in the usual sense but heap cells of the same
kind the evaluator uses; the whole interpreter works on one
representation.  Some desugaring happens in the parser itself.
Pattern guards, for example, are turned into a hidden `Maybe` binding
and a boolean guard before the rest of the system sees them.

**Static analysis.**  `static.c` resolves names, checks that
constructors and classes are used consistently, sorts bindings into
dependency groups, and handles the `foreign` declarations.  `module.c`
implements the module system: what a module exports, what an import
brings into scope.

**Type checking.**  `type.c` infers types by Hindley-Milner inference
with type classes, one binding group at a time in dependency order.
`preds.c` solves class constraints and builds dictionaries;
`subst.c` is the substitution machinery.  This is where the `-98`
extensions take effect: multi-parameter classes and functional
dependencies change what `preds.c` accepts, rank-2 types change what
`type.c` will unify.

**Compiling.**  `compiler.c` compiles each binding to the code of an
abstract machine: pattern matching is compiled to case trees, local
functions are lambda-lifted to supercombinators, and the result is a
sequence of instructions for `machine.c` to run.  `scc.c` does the
strongly-connected-components analysis the compiler needs.

## Running the code

`machine.c` is the evaluator, a graph-reduction machine driven by the
compiled instructions.  Values live in a heap of two-word **cells**
managed by `storage.c`.  The heap is a fixed size, set by `-h` when
the interpreter starts, and is collected by a mark-and-sweep
collector.

The collector is conservative about the C stack: any word on it that
looks like a cell pointer keeps that cell alive.  That is why five of
the source files that hold cell pointers in C variables across
allocations (`compiler.c`, `module.c`, `subst.c`, `static.c`, `type.c`)
are compiled without optimisation.  An optimiser could keep only a
derived pointer, which the scan would not recognise.

`builtin.c` holds the primitive functions the `Prelude` is built on:
arithmetic, characters, arrays, the pieces of the FFI.  `iomonad.c`
implements the `IO` monad and exceptions, `bignums.c` the `Integer`
type, `char.c` Unicode classification and the encodings described in
[Character encoding](character-encoding.md).

## The front ends

Three programs share all of the above.

`hugs.c` is the interactive interpreter: the prompt, the commands, and
the loop that reads a line, decides whether it is a command or an
expression, and acts.  `edit.c` adds line editing through readline or
libedit when one was found at build time.

`runhugs.c` is a small C program written against the server API of
`server.c` (see [The server API](../reference/server-api.md)): it
loads one file, looks up `Main.main`, runs it and exits.

`ffihugs` is `runhugs.c` compiled with `FFI_COMPILER` defined.  In
that mode, loading a module with `foreign` declarations makes `ffi.c`
write a C file of stubs and run the C compiler on it instead of
looking for an object to load.  `script.c` keeps the list of loaded
files that all three share, and `opts.c` reads the options.

## Where the platform comes in

There is no configure-time probing.  `sysdeps.h` states the facts
about the supported targets, which are constant across them: the
sizes of `int` and pointers, the direction of the C stack, the shape
of signal handlers.  meson writes the operating system and
architecture names into `platform.h`, and the build options and
install paths into `config.h`.  `machdep.c` holds the code that
differs between Windows and POSIX: finding the library directory,
running the editor and the shell, dynamic loading of FFI objects,
timers and signals.

The libraries have one platform-dependent piece too, the widths of C
types and the values of `errno` and signal constants.  These are asked
of the C compiler at build time by `libraries/tools/mksysconfig.c` and
written into the module `Hugs.SysConfig`.

## What this design gives up

Everything is compiled every time.  There is no object code for
modules, so starting Hugs recompiles the `Prelude`, and `:reload`
recompiles every loaded module, not just the changed one.  Programs
run perhaps a hundred times slower than compiled code.  In exchange
the whole implementation is about fifty thousand lines of C that one
person can read, and the turnaround from editing a file to seeing the
result is a `:reload`.

## See also

- [Modules, files and the search path](modules-and-the-search-path.md)
- [Build options and install layout](../reference/build-options.md)
