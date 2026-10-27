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
has are always on, except overlapping instances, which need the `+o`
toggle (`+O` for the unsafe kind).  The extensions Hugs had that it
doesn't keep are gone from the source.

Cupid is meant to run real Haskell, because an implementation that
runs only examples shows little of how Haskell works.  That is not the
same as following GHC.  A program written for GHC loads if it keeps to
the language and libraries described below; for one that doesn't,
Cupid grows no compatibility mode and no flag.

## What is inherited and what is not

The architecture is Gofer's, designed in 1991 by Mark Jones and
described in his 1994 report ([History](history.md#gofer-and-hugs)
says where to find it).  Gofer's goals were compatibility with Haskell,
extensibility as a test bed for type-system ideas, and portability
down to an 8 MHz PC with 640K; speed was explicitly not one.  Cupid
keeps the parts of that design that serve readability, and they are
most of it: one cell type for everything, fixed tables whose
high-water marks double as checkpoints, a conservative collector so
that ordinary C can build in the heap, a type checker that rewrites
the program in place, and a small abstract machine.  Where the report
gives a reason for a choice, the explanation pages repeat it, because
the reason is usually still the reason.

The departures follow from the different aim.  Gofer was written for
K&R compilers and 16-bit machines, so it leaned on the preprocessor:
`Args(...)` prototypes, map-like macros, a source file included
several times as a template, error reporting as macros.  Cupid targets
C23 on 64-bit systems and makes each of these a function or a plain
declaration.  Gofer's components were driven by a `control` function
per file answering `INSTALL`, `RESET`, `MARK` and `EXIT` codes; Cupid
makes each phase a named function in a subsystem table.  Gofer marked
its static cells in each file's `MARK` case; Cupid registers them
once.  Gofer kept old features compilable behind definitions, with a
footnote wishing it could remove some; Cupid deletes them.  And Gofer
aimed at compatibility with a moving target, which its author came to
find "a little frustrating"; Cupid does not chase GHC.  A feature
gets in by the rules under [The language](#the-language), and what is
left out is written down as well as what is in.

## The language

**Haskell 2010, plus what real code needs and one type checker can
hold.**  The accepted language is Haskell 2010 with multi-parameter
type classes, functional dependencies, flexible contexts and instances,
existential and rank-2 types, polymorphic and constraint synonyms, kind
signatures, GADT syntax, equality constraints, type operators, tuple
sections and GHC's deriving strategies, all always on, except overlapping
instances (`+o`, `+O`).  `forall` is always a keyword.  Literate
scripts are not read.  A `#!` first line is read as a blank line, so
an executable script is an ordinary Haskell file.  The details are in
[The language Cupid accepts](../reference/language.md) and
[Language extensions](../reference/extensions.md).

**How a type-system feature gets in.**  The type system grows one
feature at a time.  Each feature is held to the following rules, which
exist to keep the type checker one pipeline that a reader can follow:

- *Needed by real code.*  A feature comes in because programs people
  have written need it, found by loading packages from Hackage and
  seeing what stops them.  Without that evidence a feature stays out,
  however small; restricted type synonyms, kept from Hugs, are the one
  exception.  Where a need can be met by a library module, or by a
  tool such as the C preprocessor before loading, it is met there and
  not in the C.
- *Always on.*  There are no language-extension flags like GHC's
  `-XFoo` (Cupid's `-X` only re-reads an option string).  A feature is
  good enough for every program, or it is not in the tree.  The one
  exception is overlapping instances, which need `+o` or `+O`.
- *One solver.*  A feature extends the existing constraint solver; it
  never adds a second one.  Separate engines are how the extensions
  Hugs used to have were written, and they are why those are gone.
- *Take the common case, reject the rest.*  Where inference is the hard
  part, a type signature is required.  Where termination is the hard
  part, the existing `-c` depth bound comes before any new termination
  check.
- *GHC's syntax, Cupid's rules.*  The syntax is GHC's, so programs
  read the same.  Where the rules differ, the difference is listed in
  [Language extensions](../reference/extensions.md).
- *Small and local.*  A feature is a few named functions in the file
  that owns its stage of the pipeline.
- *Nothing below the type checker changes.*  The type checker turns
  evidence into ordinary expressions (dictionary arguments), so code
  generation never sees a predicate; a GADT match compiles as an
  existential match, and a feature that needs more than dictionary
  arguments at run time is not a candidate.

**What stays out.**  Some of GHC's language can't meet those rules.
Unboxed types and `MagicHash` change the machine below the type checker.
Template Haskell and linear types each need a mechanism of their own,
rewrite rules have nothing to rewrite in an implementation that does not
optimise, and STM and asynchronous exceptions need preemptive threads.
None of them is in.  Type families are not in either; whether they can
be added without a second solver is an open question.  [Language
extensions](../reference/extensions.md#not-accepted) lists what is left
out and why.

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
compilers.  Their API follows GHC's base, roughly base 4.18: the class
hierarchy, and the modules and functions that code written since 2006
imports.

"Small" is a property of the C, not of the libraries.  The C is what a
reader of Cupid reads; a library module is plain Haskell, apart from a
few small C helpers in `base`, and adds no C to the interpreter, so the
libraries can follow what programs import without working against the
aim.

The parts that depend on the platform, such as the C types behind
`CInt` and `CSize` and the values of `errno` constants, are
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

**Modern 64-bit systems only.**  Cupid targets 64-bit Linux, macOS and
the BSDs on x86-64 and ARM64, and Windows on x86-64.  On these, the
questions autoconf used to ask all have the same answer: `int` is four
bytes, pointers eight, floating point is IEEE, the stack grows down, and
every C99 header is present.  `src/sysdeps.h` writes those answers down,
and `static_assert`s in `prelude.h` check them.  The only real
differences left are Windows against POSIX, and sometimes macOS.  Linux,
macOS and Windows are built and tested on every change; on Windows, see
[Build Cupid on Windows](../install/build-on-windows.md).

**meson, with generated files kept in the build tree.**  The build is
meson only.  Three kinds of file are generated:

- `config.h` holds the build options and the install paths.
- `platform.h` is generated from `platform.h.in` with the operating
  system and architecture names as meson knows them, in the
  `*_HOST_OS` spellings the C sources test.  Only C reads it; the
  libraries get the same facts from `Cupid.SysConfig`.
- `parser.c` and `parser.h` are generated by bison into the build
  directory, and no copies are kept in `src/`.  A hand-regenerated
  file goes stale.  A stale `parser.h` next to `input.c` would also
  win over the generated one for a quoted `#include`, and the build
  would then use the wrong token numbers.

The library build also generates `HsBaseConfig.h` and
`Cupid/SysConfig.hs`, as described under
[The libraries](#the-libraries).

**Two feature options.**  A build option is a flag like any other, so
each of the ten inherited from Hugs had to justify its second code
path, and eight didn't.  Where an option chose between two behaviours,
the less surprising one stays: the one the default build already had,
unless that left a documented feature missing.  So the one-line
banner, the build without a fatal-signal handler, absolute file names
in messages, the elapsed-time line after each command and the "while
evaluating:" lines after a stack overflow are gone, with the code only
they compiled.  So are the primitives that exposed the interpreter's
cells and bytecode, and the three experimental modules over them that
nothing imported.  `:xplain` and `+x` are documented, so they are
always built instead, and `+x` is off by default.  Two options are
left.  `profiling` changes the layout of every heap cell, so it can't
become a run-time switch without costing every build.  `checks` is a
developer build that keeps assertions, checks cell tags and adds the
`+D` bytecode listing.  `char-encoding` and `readline` remain as
well, but they choose a character set and a library, not a feature.

**Every configuration is tested.**  An option nothing exercises rots.
So the regression suite has to pass in the default build, with each
option on alone and with both on, and `tools/check-configurations`
builds and tests all four.  With two feature options those four are
every combination of them; a third would make the sweep a sample, five
of eight.  `char-encoding` and `readline` are not swept.  No feature
option changes what a program prints (a build with `checks` adds the
`+D` toggle to the `:set` listing), so every configuration is compared
against the same expected output, with no filters.  See [Check every
build configuration](../how-to/check-every-configuration.md).

## The C code

The C is meant to be read, so it is written in a small number of
consistent ways.

**Standard C23, written in its C17-compatible subset.**  The GNU
extensions are computed-goto dispatch in `machine.c`, with a `switch`
fallback for compilers that don't have it, and a few function attributes
in `prelude.h`, each with a fallback for other compilers.  Newer C23
features such as `nullptr`, `typeof` and `constexpr` are not used
casually.

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
type means adding a row.  Every stub records the `CUPID_API_VERSION` it
was built for, and the interpreter refuses a stub whose layout it cannot
read (the current version 6 and the older 2 to 4 are accepted; version 5
and unknown versions are not).  `HsFFI.h` keeps the name the FFI
standard gives it.  [Foreign import
wrappers](foreign-import-wrappers.md) explains why callbacks into
Haskell are not available.

## See also

- [How Cupid runs a program](how-cupid-runs-a-program.md)
- [Cells, text and tables](cells-text-and-tables.md)
- [How constraints are solved](how-constraints-are-solved.md)
- [History](history.md) — how the tree got here from Gofer and Hugs 98
- [Build options and install layout](../reference/build-options.md)
