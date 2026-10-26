# How code is compiled and run

Once a module has been type checked, `src/compiler.c` turns its
bindings into supercombinators and `src/machine.c` turns those into
instructions for an abstract machine and runs them.  The machine is a
variant of the Chalmers G-machine; the compiler is the sequence of
transformations needed to reach it.  This page explains both and the
choices in them.  Most of the design is Gofer's and the reasons given
are its author's; the places where Hugs changed it are noted.

## From typed bindings to a core language

Almost nothing after the type checker can report an error to the user
(the exception is a binding too large for the code, which fails with
"Compiled code too complex"), which frees the compiler to change
representation.  `translate` reduces the typed program to a core
language with only the constructs the later stages understand:

- Line numbers are dropped from right-hand sides.  (Gofer's author
  later called this hasty, because a warning for a non-exhaustive
  definition would need them.)
- The lists of lists of bindings that dependency analysis produced are
  flattened into nested `LETREC`s, one per group.
- List comprehensions are translated by Wadler's scheme.
- `case` and lambda expressions become local function definitions,
  named by `inventVar`; this is where the `vN` names come from.
- A pattern binding `(x:xs) = e` becomes a variable `u` bound to `e`
  guarded by a conformality check, and `x` and `xs` become selections
  `_SEL (:) u 1` and `_SEL (:) u 2`.  `_SEL` (`nameSel`) extracts the
  `n`th field of a value built with a given constructor; it has no
  valid Haskell type, which does not matter because it is introduced
  after type checking.

## The pattern-matching compiler

What remains is pattern matching in function definitions.  The
pattern-matching compiler (`pmcTerm`, `match`) rewrites each function
into a tree of `CASE`s on single constructors, `NUMCASE`s on literals,
guards and `FATBAR`s, with no nested patterns.  Hugs rewrote this pass
to handle record syntax, so the algorithm is not Wadler's left-to-right
one as the Gofer report describes it, but the output has the same
shape.  `FATBAR (l, r)` means "`l`, or `r` if `l` fails to match"; it
is a construct rather than a primitive so that the code generator can
recognise it and compile most occurrences away (`canFail` decides
when one is needed at all).

The output has an unusual property: **it contains no binding
occurrences of variables.**  A function is annotated with its arity,
and a bound variable is an `OFFSET` whose value is its position
counting from the first argument; the patterns in a `CASE` list only
constructors.  The functions in `compiler.c` carry a "current offset"
parameter, `co`, to know what the next variable's offset will be.  The
report gave two reasons.  Leaving out the binders saves space, which
mattered on the PC.  More importantly, the offsets are chosen to be the
variables' positions on the machine's stack when the function runs, so
the code generator needs no environment mapping variables to stack
slots.

The same pass collects, for each local function definition, its free
variables and the local functions it calls, into a record
`(Var, arity, (Fvs, FFs, Rhs))`.  That is bookkeeping for the next
stage, merged into this one to avoid another traversal.

## Lambda lifting

The G-machine runs closed global functions, supercombinators, so local
function definitions must go.  Lambda lifting adds each free variable
of a local function as an extra parameter and moves the function to
the top level.  With mutually recursive local functions the extra
parameters of one depend on those of the others, and the smallest
correct sets are the least solution of

    lv(f) = fv(f) ∪ ⋃ { lv(g) | g called from f }

which `solve` finds by iterating from `fv(f)` until nothing changes,
using the `Fvs` and `FFs` lists the previous stage left.  This is
Johnsson's algorithm.  Calls to a lifted function need the extra
arguments added; rather than substituting through the bodies, `lift`
carries an environment `tr` that maps each local function to its
lifted call, extended before the scope of each `let` is entered.

Lifting is not required for a lazy implementation (Turner's and
Peyton Jones's machines do without it); it is here because the
G-machine wants supercombinators.

## The pre-compiler

Adding parameters to a lifted function breaks the offsets in its body,
because the offsets were counted from the original first argument.
`preComp` renumbers them.  When the function runs, its original
arguments are at offsets `1..a`, the `e` extra parameters at
`a+1..a+e`, and its temporaries above that, so an offset below the
original right-hand side's base maps into the extra-parameter range by
its position in `extraVars`, an original parameter keeps its position,
and a temporary moves up by `e`.  The calculation is small; it exists
only because binders are implicit.

## The machine

A running program is a graph of cells: applications (`AP` pairs),
constants, names, and indirections.  `eval(e)` reduces `e` to weak head
normal form.  It walks down the spine of applications pushing each
node, until it reaches the head:

- A constant or a constructor is already in normal form.  The head is
  left in the register `whnfHead` (and an integer's value in
  `whnfInt`), and the arguments of a constructor are left on the stack
  in place of the application nodes, so C code can take them.  This is
  how a primitive such as the one that prints a list walks it: `eval`,
  test `whnfHead` against `nameCons`, pop the head, pop the tail.
- A function with enough arguments has them rearranged so that its
  code sees the argument values at `stack(root + 1 ..)` rather than
  the application nodes, with the outermost application kept at `root`
  because the function's code will overwrite it with the result.  Then
  `run` executes the function's instructions, or a primitive's C body
  is called.  When it returns, evaluation continues from the result
  until a normal form is reached.
- A function with no arguments is a constant applicative form: it is
  evaluated once and the result is stored in its name record (`defn`),
  so the second use costs nothing.

An update writes an indirection, `INDIRECT` in the first component of
the overwritten cell, rather than copying the value into it, so that
sharing is not lost; the evaluator follows indirections as it meets
them.

The instruction set is Gofer's with a few additions.  `root` is the
position of the application being reduced and `sp` the stack pointer:

| instruction | effect |
|-------------|--------|
| `LOAD n` | push `stack(root + n)`: an argument or local variable |
| `CELL c`, `CHAR c`, `INT n`, `DOUBLE d`, `STRING t` | push a constant |
| `MKAP n` | replace the function on top and the `n` arguments below it by their application, unevaluated |
| `UPDATE n` | pop a value and overwrite `stack(root + n)` with an indirection to it |
| `UPDAP n` | like `MKAP 1` then `UPDATE n`, but reuses the application node already at `stack(root + n)` |
| `RUPDATE`, `RUPDAP` | `UPDATE 0` or `UPDAP 0` followed by `RETURN` |
| `ALLOC n` | push `n` fresh `AP (NIL, NIL)` cells, to be filled in by a recursive `let` |
| `SLIDE n` | drop the `n` entries below the top: leave a `let`'s scope |
| `EVAL` | pop an expression and reduce it to weak head normal form |
| `LEVAL n` | `EVAL` of `stack(root + n)` without the push (a peephole for `LOAD; EVAL`) |
| `STAP` | apply strictly: evaluate the argument before building the application, for a strict constructor field |
| `TEST c l` | if `whnfHead` is not `c`, jump to `l`; the heart of pattern matching |
| `GOTO l`, `RETURN` | control |
| `FAIL` | the match failed |
| `SETSTK n` | set `sp` to `root + n`, where the stack depth is not known at compile time |
| `ROOT n` | push the application obtained by removing the last `n` arguments from the one at `root` |
| `TABLE` | not executed: marks a table of a type's constructors stored in the code area, which derived instances use |

Gofer's `DICT` and `SELECT` went when Hugs made dictionaries ordinary
heap values built by per-instance functions ([How constraints are
solved](how-constraints-are-solved.md)); its `INTEQ`, `INTGE` and
`INTDV` went with `n+k` patterns, and a literal is now matched by
calling an equality primitive and `TEST`ing the result.

**One `FAIL`.**  There is a single `FAIL` instruction, written once at a
fixed address when the machine is installed, and every failing branch
jumps to it.  The label `0` in a listing is that address.

**The root optimisation.**  Many functions pass the same leading
arguments to every recursive call, `map f` in `map f (x:xs) = f x : map
f xs`.  The code generator notices when a prefix of the application
being reduced can be reused in the result and emits `ROOT n`, which
pushes that prefix instead of rebuilding it; for `map` it saves one of
the four application nodes the reduction would otherwise allocate.
Gofer's author described it in 1991, measured a 25-30% reduction in
space on small examples, and knew of no other system that used it.

**Strict and lazy contexts.**  A conditional whose value is certainly
needed compiles to `EVAL` the test, `TEST True`, and the two branches
with a jump; one whose value may not be needed, say on the right-hand
side of a local definition, is built as an application of the
primitive `nameIf` with `MKAP 3` and left for later.  `FATBAR` is
treated the same way, with `nameFatbar`.  The strict form is preferred
wherever it is sound because it avoids building both branches.

**What Hugs added.**  The interpreter now also simulates the stack at
compile time (`srsp`, `offsPosn[]`), which Gofer did only in its C
compiler, and the code generator runs peephole optimisations within a
basic block: merging `MKAP`s, turning `MKAP 1; UPDATE` into `UPDAP`,
merging `SLIDE`s.  A string constant is kept as its `Text` and expanded
into list cells lazily as it is consumed.  The depth of nested `eval`
calls is bounded, and the C stack is checked, so a runaway program
reports an overflow instead of crashing.

## Seeing the code

A `checks` build has the `+D` option, which prints each definition as
the code generator receives it and the instructions it produces, and
traces `run` and `eval` as they go.  The report's advice holds: the
best way to understand the compiler and the machine is to read what
they produce for `id`, `(.)`, `(&&)` and `map`, and then to work out
how the listing for `map` implements it.

## See also

- [Cells, text and tables](cells-text-and-tables.md) — the heap the
  graph lives in and the stack the machine uses
- [The design of Cupid](design.md#data-and-memory) — why no code is
  generated during evaluation
- [How Cupid runs a program](how-cupid-runs-a-program.md)
- S. L. Peyton Jones, *The Implementation of Functional Programming
  Languages* (1987), for the G-machine, pattern-matching compilation
  and lambda lifting this follows
