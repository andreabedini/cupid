# The design of Cupid

Cupid has one aim: to be a Haskell implementation you can sit down and
read.  Three properties follow from it and do not change: Cupid is an
interpreter written in C, it does not self-host, and it stays small.
This page explains the choices that the aim leads to: in the language,
the libraries, the build, the C code and the running interpreter.  For
each one it says why the choice was made, so that the next change can
be checked against the same reasons.

## Deleting instead of switching off

The rule behind most of what follows is that a feature which doesn't
serve the aim is deleted, not left in the tree behind a flag.  A flag
is a promise to keep two code paths working forever, and every such
promise makes the code harder to read.  So Cupid has no
language-extension flags and no compatibility modes.  The extensions it
has are always on, and the extensions Hugs had that it doesn't keep
are gone from the source.

Compatibility with code written for other systems is not a goal.
Programs written for GHC, nhc98 or Hugs in 2006 load if they are
Haskell 2010 plus the extensions described below.  Programs that rely
on anything else don't, and Cupid doesn't try to make them.

## The language

**Haskell 2010, plus what the libraries need.**  The accepted language
is Haskell 2010 with multi-parameter type classes, functional
dependencies, flexible contexts and instances, existential types,
rank-n types, kind signatures, GADT syntax and equality constraints, all
always on.  `forall` is always a keyword.  Literate scripts are not
read.  A `#!` first line is read as a blank line, so an executable
script is an ordinary Haskell file.  The details are in
[The language Cupid accepts](../reference/language.md) and
[Language extensions](../reference/extensions.md).

**How a type-system feature gets in.**  The type system grows one
feature at a time.  Each feature is held to the following rules, which
exist to keep the type checker one pipeline that a reader can follow:

- *Always on.*  There is no `-X` flag.  A feature is good enough for
  every program, or it is not in the tree.
- *One solver.*  A feature extends the existing constraint solver; it
  never adds a second one.  Separate engines are how the extensions
  Hugs used to have were written, and they are why those are gone.
- *Restrict rather than infer.*  Where inference is the hard part, a
  type signature is required.  Where termination is the hard part, the
  existing `-c` depth bound comes before any new termination check.
- *GHC's syntax, Cupid's rules.*  The syntax is GHC's, so programs
  read the same.  Where the rules differ, the difference is listed in
  [Language extensions](../reference/extensions.md).
- *Small and local.*  A feature is a few named functions in the file
  that owns its stage of the pipeline.
- *Nothing below the type checker changes.*  Evidence is erased before
  code generation, a GADT match compiles as an existential match, and
  a feature that needs evidence at run time is not a candidate.

Equality constraints show the rules at work.  `a ~ b` is a predicate
with no evidence.  A given equality is unified at the point it is
assumed, and a wanted equality is solved by unification where it
arises, so it never waits in the solver.  Nothing could make a stuck
equality hold later, because there are no type families.
[How constraints are solved](how-constraints-are-solved.md) follows
the solver in detail.

**Desugar early.**  Where a construct can be expressed in terms the
rest of the system already handles, the parser rewrites it, and no
later stage knows it existed.  Pattern guards become a hidden `Maybe`
binding and a boolean guard.  GADT syntax becomes ordinary
constructors with existential variables.  The alternative, a new node
in the syntax tree, would have to be taught to every stage after the
parser.

**Context reduction is Haskell 2010's.**  Contexts are reduced to head
normal form.  The one exception is under overlapping instances (`+o`,
`+O`), where reducing through an instance is unsound because a more
specific instance may apply later.

**The grammar's `a`/`b` split stays.**  `parser.y` has paired rules
such as `infixExpa` and `infixExpb`.  They look like duplication, but
the split confines a lambda, `let`, `if` or `do` to the last operand
of an operator chain, and bison cannot parameterise a rule.

## The libraries

Cupid owns its libraries.  `libraries/base` and `libraries/cupid` are
plain Haskell, written for Cupid, with no conditional code for other
compilers.  The parts that depend on the platform, such as the C types
behind `CInt` and `CSize` and the values of `errno` constants, are
generated at build time by `libraries/tools/mksysconfig.c` into the
module `Cupid.SysConfig` and the header `HsBaseConfig.h`.  A small C
program answers those questions by asking the compiler, so the build
needs no Haskell tools at all: a C compiler, meson, ninja, python3 and
bison are the whole list.

Shared library sources and the cpp, cpphs and hsc2hs bootstrap they
needed used to be the largest single source of complexity in the tree.
Nothing in it targets GHC now, so they serve no purpose.
[The libraries](../reference/libraries.md) lists what is shipped.

## Platforms and the build

**Modern 64-bit systems only.**  Cupid targets 64-bit Linux, macOS,
the BSDs and Windows on x86-64 and ARM64.  On these, the questions
autoconf used to ask all have the same answer: `int` is four bytes,
pointers eight, floating point is IEEE, the stack grows down, and every
C99 header is present.  `src/sysdeps.h` writes those answers down, and
`static_assert`s in `prelude.h` check them.  The only real differences
left are Windows against POSIX, and sometimes macOS.  Windows is
supported by design but has not been tested; see
[Build Cupid on Windows](../how-to/build-on-windows.md).

**meson, with generated files kept in the build tree.**  The build is
meson only.  Two kinds of file are generated:

- `platform.h` is generated from `platform.h.in`, because the Haskell
  libraries are preprocessed outside the C compiler and can't see
  macros like `__linux__`.
- `parser.c` and `parser.h` are generated by bison into the build
  directory, and no copies are kept in `src/`.  A hand-regenerated
  file goes stale.  A stale `parser.h` next to `input.c` would also
  win over the generated one for a quoted `#include`, and the build
  would then use the wrong token numbers.

**Every configuration is tested.**  The ten feature options in
`meson.options`, `tag-checks` and `timer` among them, each change what
is compiled.  An option nothing exercises rots.  So the regression
suite has to pass with each option on alone and with all of them on
together, twelve configurations in all, and `tools/check-configurations`
builds and tests them.  Where an option changes the output, for
example by adding a timing line, `tests/run-tests` normalises that
output only when the option is on, so a filter can never hide a
difference in the default build.  See
[Check every configuration](../how-to/check-every-configuration.md).

## The C code

The C is meant to be read, so it is written in a small number of
consistent ways.

**Standard C23, written in its C17-compatible subset.**  The one GNU
extension is computed-goto dispatch in `machine.c`, with a `switch`
fallback for compilers that don't have it.  Newer C23 features such as
`nullptr`, `typeof` and `constexpr` are not used casually.

**One file is one translation unit.**  No `.c` file includes another.
A fragment included into several files shares their static state and
macros, and can't be compiled, checked or read on its own.  Where a
fragment used to be a macro template, it is now a function that takes
callbacks.

**Every header compiles on its own.**  A header includes what it uses,
so it can be the first line of a translation unit.  In practice the
only prerequisites are `prelude.h` and `storage.h`.  The include block
of a `.c` file lists what the file calls, and its order doesn't matter.

**A function unless C cannot express it.**  A macro evaluates its
arguments more than once, can hide an assignment, and can't be stepped
into with a debugger.  So anything that can be a function is one,
marked `CUPID_inline` (`static inline` and always inlined, so it costs
nothing even in the files built at `-O0`).  The macros that remain are
those C has no other way to write: lvalue projections such as `fst`
and `hd`, the `map` family, the primitive templates, and anything that
needs `sizeof` of the caller's array.

**Errors are function calls.**  A message is built with `errHead`,
`errPrintf` and the `errExpr`/`errType`/... helpers, and raised with
`errFail`.  Because these are ordinary calls, editors and analysers
understand them, and the compiler checks every format string.

**Plain C spelling.**  Standard types are used (`bool`, `int8_t` and
so on), not private aliases for them.  `Char` and the domain types
(`Cell`, `Text`, `Name`, `Type` and so on) are kept, because they
carry meaning.  Output goes through `snprintf`, never `sprintf`.  Each
function's description is a block comment above its header.

**Switches over cell tags say what they mean.**  Cell tags are a
`typedef enum CellTag`.  A `switch (whatIs(c))` either lists every tag
it can meet and has no `default`, so that `-Wswitch` checks it, or
ends in `default : break;`, which says on purpose that the other tags
are ignored.

**Formatting is clang-format's.**  `.clang-format` formats the sources
in `src/` and `src/prims/`, and generated files are left alone.
String literals are never split, so every message stays searchable
with grep.  New code is formatted before it is committed.

**Static analysis is a short list, reviewed by hand.**  `.clang-tidy`
enables only checks whose findings are worth a person's time here; it
says why each broader group was left out.  Findings are read and fixed
by hand, never auto-fixed.  A finding is checked against a build with
every feature option on before anyone acts on it, because one
configuration is not the whole tree.  An `#include` that only a
feature option needs carries an `IWYU pragma: keep` comment naming
that option.  See [Run the static analysis](../how-to/run-clang-tidy.md).

## Data and memory

**Everything is a cell, checked by its tag.**  Types, kinds,
predicates, lists and expressions are all trees of heap cells (`Cell`).
The C type system can't tell them apart.  Wrapper structs would fight
the thousands of uses of generic operations such as `hd` and `cons`,
and a typed heap would be a rewrite of the runtime.  Instead, under
`-Dchecks`, the entry points where categories meet check a tree's
shape at run time (`tagcheck.c`).  The typedef aliases (`Type`, `Kind`
and so on) remain as documentation.

**Tables are fixed arrays with asserted limits.**  Names, type
constructors, classes, instances and scripts each live in a fixed
array.  A handle such as a `Name` is a `Cell`, because handles are
stored in heap cells and `NIL` must be a valid value.  An accessor
such as `nameRec(n)` turns a handle into a pointer to its record, and
the pointer stays good because the arrays never move.  The table sizes
are in `src/sizes.h`.  The values that partition the cell space are
order-sensitive, so `static_assert`s at the end of `storage.h` check
them all.  A full table is reported as "storage space exhausted", not
handled by growing the table.

**State is passed, not left in globals.**  The code generator's state
is a `CodeGen` record that its entry points create and pass on.  The
type checker returns a `Typing` record instead of leaving its result
in four global registers.  A function's signature then says what it
reads and writes.

**Garbage collection: roots are registered.**  The collector scans the
C stack conservatively and marks the root tables that modules register
at install time, each with `registerRoots(xRoots)`.  A `gcMark` hook is
needed only for structured data.  The fifteen files that hold cells in
C local variables during allocation, listed as `gc_sensitive` in
`src/meson.build`, are compiled at `-O0`.  An optimiser could keep
only an interior pointer to a cell, which the conservative scan would
not find.  The type checker relies on that scan, so it must never run
with conservative collection switched off.

**No code is generated during evaluation.**  `run()` keeps a raw
pointer into the code area, which would be invalid if the area grew
under it.  The area can only grow when code is generated, so
`growMemory` reports an internal error if that ever happens during
evaluation.  An index instead of a pointer would cost an extra
register reload on every instruction.

[How Cupid runs a program](how-cupid-runs-a-program.md) puts these
pieces in the order a program meets them.

## The running interpreter

**Subsystems are a table of hooks.**  Each module that has state to
install, reset, mark, clean up at exit or tidy after an interrupt
contributes one `Subsystem` row, with named functions such as
`resetTypeChecker` and `markStorage`.  The `subsystems[]` array in
`evaluator.c` fixes the install order, and five functions walk it, one
for each phase.  Before this, the order existed only in a comment, and
what each module did in each phase was hidden in a `switch`.
Primitive modules and the stubs `fficupid` generates carry the same
hooks.

**Interrupts are taken at polls only.**  Ctrl-C only sets a flag.  The
flag is acted on at `allowBreak()` polls, which sit in the evaluator,
the bignum loops, the lexer, the type walkers and the driver loops of
the type checker, dependency analysis and compiler.  The garbage
collector is not a poll: allocation happens everywhere, so a poll
there would make every allocation an interrupt point.  After an
interrupt the interpreter is in the same state as after an error: it
jumps back to the prompt and resets the subsystems.

**Fatal signals belong to the program.**  The handler for a crash
makes only async-signal-safe calls: it restores the terminal, writes
one line, and re-raises the signal.  The process then ends with the
signal, so the shell sees it and a core dump is still written.  Each
program's `main` installs the handler, because how a process ends is
its owner's decision, not a library's.

**Output goes to a sink the program chooses.**  All interpreter output
goes through `Printf` and its siblings to an `OutputSink`, or to the
standard streams when none is set.  `runcupid` collects output into a
growable buffer and prints it on error, so long messages are printed
in full.  Where output goes is decided by the program that starts the
interpreter.

**Commands run without a shell.**  The `-F` preprocessor, the `-E`
editor and the C compiler that `fficupid` uses are each built as an
argument vector and started directly (fork and `execvp`, or
`_spawnvp` on Windows).  A file name arrives as one argument, byte for
byte, so there is nothing to quote and nothing to inject.  A
configured command is split on whitespace only; if you need a pipe or
a redirect, write a wrapper script.  The `:!` command and
`System.Cmd.system` are the only places a shell is used, because
running a shell is what they are for.

**Option strings have their own tokeniser.**  `CUPIDFLAGS` and `:set`
are split into words by `opts.c` itself, so reading options never
disturbs the lexer's input.

**`runcupid` is the interpreter without the prompt.**  There is no C
API for embedding Cupid in another program.  `runcupid` loads a module
and runs `main` through the same pipeline the prompt uses, and
`fficupid` is `runcupid` built to compile foreign-import stubs
instead.

## The FFI

`fficupid` generates a C stub for each module with foreign imports.
Each marshallable type is one row of a table in `ffi.c`, so adding a
type means adding a row.  Every stub records the `CUPID_API_VERSION`
it was built for, and the interpreter refuses a stub from a different
version rather than misread its layout.  `HsFFI.h` keeps the name the
FFI standard gives it.  [Foreign import wrappers](foreign-import-wrappers.md)
explains why callbacks into Haskell are not available.

## See also

- [How Cupid runs a program](how-cupid-runs-a-program.md)
- [How constraints are solved](how-constraints-are-solved.md)
- [History](history.md) — how the tree got here from Hugs 98
- [Build options and install layout](../reference/build-options.md)
