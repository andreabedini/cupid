# The running interpreter

The pipeline in the earlier chapters is the interesting part of Cupid, but
it runs inside a process that has to start, take commands, survive
interrupts and clean up after errors.  This chapter is about that process:
what `cupid.c`, `run.c`, `evaluator.c` and `script.c` do around the stages.

## One program, three modes

There is no C API for embedding Cupid in another program.  `cupid` is the
only program, and it picks its mode from its arguments, as python does.
`chooseMode` in `cupid.c` picks one of three: with no file, or with `-i`,
it shows the prompt; with a file it runs the program; with `--ffi` it
compiles a module's foreign-import stubs.  `-i` and `--ffi` count only
before the first file and stay out of `CUPIDFLAGS` and `:set`, where they
would apply to every file.

`interpreter` in `cupid.c` is the prompt: the loop that reads a line,
decides whether it is a command or an expression, and acts.  `edit.c`
starts the external editor for `:edit` and `:find`; line editing through
readline or libedit, when one was found at build time, is in `input.c`.

`run.c` holds `runFile`, the same interpreter without the prompt: it
starts the evaluator, loads the Prelude and the file it is given, and
calls `runProgram`, which is the expression pipeline the prompt uses,
applied to `main >> return () :: IO ()` in module `Main`.  The exit code
is whatever the program asked for.

With `--ffi`, `runFile` sets `generateFFI`, and loading the named
module, if it has `foreign` declarations, makes `ffi.c` write a C file
of stubs and run the C compiler on it instead of looking for an object
to load; `main` is not run.  `script.c` keeps the list of loaded files
that all three modes share, and `opts.c` reads the options.

## Subsystems are a table of hooks

Each module that has state to install, reset, mark, clean up at exit or
tidy after an interrupt contributes one `Subsystem` row, with named
functions such as `resetTypeChecker` and `markStorage`.  The `subsystems[]`
array in `evaluator.c` fixes the install order, and five functions walk it,
one for each phase.  Primitive modules and the stubs `cupid --ffi`
generates carry the same hooks.

## Interrupts are taken at polls only

Ctrl-C only sets a flag.  The flag is acted on at `allowBreak()` polls,
which sit in the evaluator, the bignum loops, the lexer, the type walkers
and the driver loops of the type checker, dependency analysis and compiler.
The garbage collector is not a poll: allocation happens everywhere, so a
poll there would make every allocation an interrupt point.  After an
interrupt the interpreter is in the same state as after an error: it jumps
back to the prompt and resets the subsystems.

## Fatal signals are re-raised

The handler for a crash makes only async-signal-safe calls: it restores the
terminal, writes one line, and re-raises the signal.  The process then ends
with the signal, so the shell sees it and a core dump is still written.
`main` in `src/cupid.c` installs the handler.

## Output goes through a sink

Almost all interpreter output goes through `Printf` and its siblings to an
`OutputSink`, or to the standard streams when none is set.  When `cupid`
runs a file, it collects output into a growable buffer and prints it on
error, so long messages are printed in full.

## Commands run without a shell

The `-F` preprocessor, the `-E` editor and the C compiler that `cupid
--ffi` uses are each built as an argument vector and started directly (fork
and `execvp`, or `_spawnvp` on Windows).  A file name arrives as one
argument, byte for byte, so there is nothing to quote and nothing to
inject.  A configured command is split on whitespace only; if you need a
pipe or a redirect, write a wrapper script.  The `:!` command and
`System.Cmd.system` are the only places a shell is used, because running a
shell is what they are for.

## Option strings have their own tokeniser

`CUPIDFLAGS` and `:set` are split into words by `opts.c` itself, so reading
options never disturbs the lexer's input.
