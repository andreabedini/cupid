# Reading Cupid in a day

Cupid is about 44,000 lines of C.  You can't read that in a day, and this
page doesn't suggest you try.  What a day can get you, if you know some C
and some Haskell, is the shape of the thing: what the main stages are, and
one execution path through them, from the bytes of a small program to its
output.  You'll know what the one data structure is, where a type class
goes when it disappears, and what the machine does when it runs, and you'll
be able to say which file does each step.

That is all this page promises.  You won't understand the type checker at
the end of it, and you won't yet be someone who can safely change the code.
Some of the code is also harder to read than it should be: much of the C is
Hugs's, tidied rather than rewritten.  What makes the day possible is that
there's only one way through.  There is one representation, one pipeline and
one machine, and no flags to choose between paths that would otherwise have
to be read twice.

No one has yet timed this route with a new reader, so treat it as a
proposal.

## Before you start

You'll want an interpreter that can show you what it's doing.  The `checks`
build option adds the `+D` toggle, which prints generated code and traces
evaluation.  From the top of the source tree:

```sh
meson setup build-checks -Dchecks=true
ninja -C build-checks
export CUPIDDIR="$PWD/build-checks/libraries/cupiddir"
build-checks/src/cupid
```

```
Cupid> :set +D
Cupid> 1 + 2
```

You should see a page of instructions and `Entering name` lines, with a `3`
somewhere among them.  [Build Cupid on Linux and
macOS](../start/build-on-linux-and-macos.md) has the prerequisites.  The
file to keep open beside everything else is `src/storage.h`.

## The first day

This is five chapters of the sixteen, in this order, three of them only in
part.  Read the chapter first and then the code, because the chapters tell you
what to look for.

1. **The whole path.**  [How Cupid runs a
   program](how-cupid-runs-a-program.md), all but its "Try it", with the example
   running under `+D`.  Then `addScript` in `script.c` and `runProgram` in
   `evaluator.c`, which are short.  You don't need to understand everything in
   the listing yet.

2. **The data.**  [Cells, text and tables](cells-text-and-tables.md), with
   `storage.h` open, and `pair` and `markSlot` in `storage.c`.  This is the
   investment that pays for the rest of the day.  Once you can see that `fst`,
   `snd` and `hd` work on anything, and that `whatIs` is how code decides what a
   cell is, most of the code becomes approachable.

3. **The front end, in part.**  From [How source is
   read](how-source-is-read.md), the sections on lexing and on what the grammar
   leaves to later.  Find the layout code in `yylex` (search for `goOffside`) and
   the `end` rule in `parser.y`, then read `checkDefns` in `static.c` line by
   line without following the calls.

4. **Types, in part.**  From [How types are
   inferred](how-types-are-inferred.md), the first two sections: types as a
   skeleton and an offset, and how the expression is rewritten in place.  Read
   `typeExpr` and `typeAp` in `type.c` far enough to see `assumeEvid` called,
   and `overEvid` in `preds.c`.  This is the hardest part of the day, so give it
   a bounded amount of time and move on even if you're lost.  The next step
   needs only the idea of a dictionary.

5. **Compiling and the machine, in part.**  From [How code is compiled and
   run](how-code-is-compiled-and-run.md), the opening on reduction, "Reading a
   listing" and "The machine".  Read `compileGlobalFunction` in `compiler.c` to
   see the order of the passes, without following them.  `eval` and `run` in
   `machine.c` are short enough to read whole.

That opens about 16,000 lines, of which you'll read a small part closely.

### What can wait

All of this is optional, and most of it is a better second day than a longer
first one:

- **Deeper into the type checker:** the rest of [How types are
  inferred](how-types-are-inferred.md), `kind.c`, which is kind inference and
  makes a gentle introduction to the machinery, and all of [How constraints are
  solved](how-constraints-are-solved.md), with `unrestricted` and `entail`.
- **The rest of the compiler:** the sections of [How code is compiled and
  run](how-code-is-compiled-and-run.md) on reducing the language, matching,
  lambda lifting and generating code, with their examples, and `match`, `lift`,
  `make` and `buildAp`.
- **Exercises:** "Try it" in the first chapter and the exercises that end the
  compiler chapter.  The last of those changes the source and measures the result.

### How to tell whether it worked

If the day went well, you can answer these without looking anything up, or at
least you'll know exactly where to look:

- What is `NIL`, as a C value?  What does `isPair(c)` test, and how can a single
  comparison answer that question?
- Why can't Cupid use a copying garbage collector, and why are fifteen of its
  files compiled without optimisation?
- The parser sees `{` and `;` that weren't in the file.  Who put them there?
  And when the lexer can't decide where a block ends, who decides?
- Why does the parser leave `a + b * c` as a flat list, and which pass finishes
  the job?
- A function has type `Num a => a -> a`.  How many arguments does its compiled
  code take?  Where does the extra one come from at a call site?
- What does `x + y` compile to, if there's no instruction for adding?  When does
  the addition happen?
- Why is `f x = g x` no deeper on the C stack than `g x` itself?

## The complete tour

[The sixteen chapters](README.md), read in order, are the longer way through.
They put the design right after this page, take in everything the first day
skipped, and go on to the parts around the pipeline: [the running
interpreter](the-running-interpreter.md),
[modules](modules-and-the-search-path.md), [foreign
functions](foreign-functions.md), [character
encoding](character-encoding.md), [the build](platforms-and-the-build.md) and
[how the C is written](how-the-c-is-written.md).  They end with [why the language
is what it is](language-design.md) and [the history](history.md).

## The map

For when you want to know where something lives, here is `src/`, grouped by what
it does, with approximate sizes and leaving out a few small utility files.

| Part | Files | Lines | Chapter |
|---|---|---|---|
| The data | `storage.h`, `storage.c`, `sizes.h` | 4,800 | [Cells, text and tables](cells-text-and-tables.md) |
| Reading source | `input.c`, `parser.y`, `fixity.c` | 3,600 | [How source is read](how-source-is-read.md) |
| Static analysis | `static.c`, `binding.c`, `depend.c`, `scc.c`, `pattern.c` | 2,000 | [How source is read](how-source-is-read.md#static-analysis) |
| Declarations | `tycon.c`, `class.c`, `typeexp.c`, `kind.c`, `deriving.c` | 5,500 | Mostly none yet |
| Modules | `module.c` | 1,200 | [Modules, files and the search path](modules-and-the-search-path.md) |
| Types | `type.c`, `subst.c`, `preds.c` | 5,300 | [How types are inferred](how-types-are-inferred.md), [How constraints are solved](how-constraints-are-solved.md) |
| Compiling | `compiler.c` | 1,800 | [How code is compiled and run](how-code-is-compiled-and-run.md) |
| The machine | `machine.c` | 1,800 | [How code is compiled and run](how-code-is-compiled-and-run.md#the-machine) |
| Primitives | `prims/` | 7,200 | Mostly none yet |
| The driver | `cupid.c`, `run.c`, `evaluator.c`, `script.c`, `opts.c`, `edit.c`, `goal.c`, `errors.c`, `output.c` | 3,900 | [The running interpreter](the-running-interpreter.md) |
| The platform | `machdep.c`, `sysdeps.h`, `char.c`, `unitable.c` | 3,600 | [Platforms and the build](platforms-and-the-build.md), [Character encoding](character-encoding.md) |
| Foreign functions | `ffi.c`, `HsFFI.h` | 1,500 | [Foreign functions](foreign-functions.md) |

The headers beside the static-analysis files are short and show what each exposes;
`connect.h` declares the entry points of the big stages, such as `type.c`,
`compiler.c` and `machine.c`.

Static analysis and the declarations are spread over many files because each stage
that `checkDefns` runs has its own: `tycon.c` and `class.c` check the data, class,
instance and default declarations, `typeexp.c` resolves the type expressions in
them, `kind.c` infers their kinds, `pattern.c` turns parsed expressions into
patterns, `binding.c` turns equations into bindings, and `depend.c` resolves names
and sorts the bindings into dependency groups, with `scc.c` doing the
strongly-connected-components sort.  `deriving.c` writes the derived instances,
`ffi.c` checks the `foreign` declarations, and `module.c` works out what a module
exports and what an import brings into scope.

The primitives are what the `Prelude` stands on: `prims/builtin.c` has arithmetic,
characters and the pieces of the FFI, `prims/array.c` arrays, `prims/iomonad.c` the
`IO` monad and exceptions, and `prims/bignums.c` the `Integer` type.  Unicode
classification and the character encodings are in `char.c`.

## How the output here was made

These chapters quote the interpreter in three different ways.

*Raw* output is what a terminal shows, apart from the banner: the sessions, and `$
cupid Total.hs`.

*Normalised* output is real output tidied so that it doesn't change for incidental
reasons.  A listing's addresses are gone, its jump targets are labels, `L1`, `L2`
and so on, and the address of the shared failure instruction is `FAIL`.  The names
the compiler invents depend on what has been compiled so far, so a listing in these
pages writes them descriptively, by hand: `dNumInt` is the dictionary for `Num Int`,
which a raw listing prints as `v87` at the revision below, and `go'` is the lifted
form of a local function called `go`.  What follows a `;` is a comment added by hand,
a line of `...` stands for lines left out, and traces show only the names entered.

*Schematic* forms were written by hand to show a shape, and no command prints them:
the trees and stack pictures, and the "as if you had written" Haskell.

Everything was produced from `src/` and `libraries/` as of commit `7a38d5c`, on
x86-64 Linux with gcc, built as above with `-Dreadline=disabled` as well.  The
programs are in `tests/doc-sessions/`.  With `CUPIDDIR` set as above, this shows a raw
listing, the tokens and cells from running the program under gdb, and a listing's
normalised form:

```sh
cd tests/doc-sessions
printf ':set +D\n:load Total.hs\n:quit\n' | ../../build-checks/src/cupid
gdb -q -batch -x show-stages.py --args ../../build-checks/src/cupid Total.hs
cd ../..
tests/check-doc-sessions build-checks/src/cupid \
    'build-checks/libraries/cupiddir/packages/*' . --show total-listing
```

`meson test` runs `tests/check-doc-sessions`, which compares each marked block in
these pages with the normalised output of the current tree, and fails if they differ,
so those blocks don't depend on the revision above.  It checks that each invented name
is written consistently, though not that the descriptive name is the right one.
Listings and traces need a `checks` build and the tokens and cells need gdb; where those
are missing the blocks are counted as skipped, but a gdb that runs and fails is a failure.
One CI job, Linux with gcc, runs the check with `--require-all`, which allows no skips.

The counts quoted in [Counting the work](how-code-is-compiled-and-run.md#counting-the-work)
and [What survives a collection](cells-text-and-tables.md#what-survives-a-collection) do
change with the libraries, so they belong to that revision.  The test checks three things
about them: that `head (queens 8)` takes under a tenth of the reductions of `length (queens
8)`; that repeating either in a session costs fewer reductions; and that forcing `table`
leaves between 50,000 and 65,000 fewer cells free, which a further list doesn't add to.  It
also checks that `total` on 16,000 elements gives an answer and on 20,000 reports an overflow.
