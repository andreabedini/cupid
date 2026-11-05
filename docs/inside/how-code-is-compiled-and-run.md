# How code is compiled and run

[The tour of the pipeline](how-cupid-runs-a-program.md) came to a listing
with no addition in it and no call: the code for `total` looks at its
argument, builds a graph and returns.  This chapter explains why that is
the right thing for it to do, and how `src/compiler.c` and `src/machine.c`
get there.  By the end you should be able to read what the `+D` toggle
prints, and to predict the shape of a listing before you see it.

The listings here have their addresses replaced by labels and carry
hand-written comments, and invented names are written descriptively; [How
the output here was
made](reading-cupid-in-a-day.md#how-the-output-here-was-made) has the
details and the commands.

The machine is a variant of the Chalmers G-machine.  Most of what follows
is Gofer's design, and where a reason is given for it, the reason usually
comes from [its author's
report](http://web.cecs.pdx.edu/~mpj/pubs/RR-1030.pdf).  The places where
Hugs and Cupid moved on are pointed out as we reach them.

## Reduction, and what it asks of a compiler

A running program is a graph of cells.  An application is a pair, `AP`,
so `total d l` is two of them:

```
      AP            <- the "root" of this call
     /  \
   AP    l
  /  \
total d
```

To *reduce* this is to replace it by the right-hand side of `total`, with
`d` and `l` substituted, and the crucial word is replace.  The root is
overwritten, in place, with the result, so anything else in the graph
that points at this node sees the result too.  That is the whole
difference between lazy evaluation and call by name: in

```haskell
let t = total d l in t + t
```

both uses of `t` point at one node, and the second finds it already
reduced.

So one reduction has three jobs: find the arguments, build an instance of
the right-hand side, and overwrite the root.  Building an instance is
mechanical as long as the right-hand side mentions nothing but the
function's own arguments and global names.  A function like that is
called a *supercombinator*, and for a supercombinator the construction
can be compiled ahead of time.

A listing is that recipe, with one complication: a function may have
several right-hand sides, and it has to find out which one applies.  So
its code evaluates what the choice depends on, the argument a pattern
inspects, a guard, the test of a conditional, and that evaluation can do
any amount of work.  What it doesn't normally do is evaluate the
right-hand side it has chosen.  It builds that as a graph, and the work
of evaluating it falls to `eval`, which picks up from the overwritten
root.

This tells us what the compiler has to do.  It must get every definition
into the form of a closed global function, which means removing local
functions, and with them everything else the machine has no way to
express: nested patterns, `case`, list comprehensions.  The passes in
`compiler.c` remove those, one after another.

## Reading a listing

Before following those passes, it helps to be able to read what they
produce.  The machine has one stack.  When a function's code starts, its
call has been laid out on the stack like this, with `root` being the
position of the call:

```
stack(root)     AP (AP total d) l    the call, to be overwritten
stack(root+1)   l                    the last argument
stack(root+2)   d                    the first argument
```

The arguments are in reverse order, for a reason we'll get to when we
reach `eval`.  Nearly everything you see in a listing is one of four
instructions.  `LOAD n` pushes `stack(root + n)`.  `CELL c` pushes a
constant, usually a global name.  `MKAP n` takes the function on top of
the stack and applies it to the `n` arguments below it, leaving one
application in their place.  `RUPDAP` finishes: it takes a function and
an argument from the stack, overwrites the root with the application of
one to the other, and returns.

Because `MKAP` expects the function on top, expressions appear backwards,
last argument first and function last.  Here is the branch of `total` for
a non-empty list, with the stack written out after each instruction.  By
this point `LEVAL 1` has evaluated the list and left its fields on the
stack, so `stack(root+3)` is the tail `xs` and `stack(root+4)` is the head
`x`:

```
                 what this instruction leaves on top of the stack
LOAD    3        xs
ROOT    1        xs, (total d)
MKAP    1        (total d xs)
LOAD    4        (total d xs), x
LOAD    2        (total d xs), x, d
CELL    +        (total d xs), x, d, (+)
MKAP    2        (total d xs), ((+) d x)
RUPDAP           the root is now (+) d x (total d xs)
```

The odd one is `ROOT 1`, which pushes the current call with its last
argument removed.  `total d` is already sitting in the graph, inside the
root, so there's no need to build it again.  The whole [instruction
set](#the-instruction-set) is in a table further on, and the other
instructions are introduced as they turn up.

## Reducing the language

The first pass, `transBinds` and `translate`, rewrites a binding into a
smaller language.  Line numbers are dropped from right-hand sides
(Gofer's author later called this "a little hasty", because a warning for a
non-exhaustive definition would need them: RR-1030, p. 34, note 9).  Dependency analysis's list
of binding groups is flattened into nested `LETREC`s, list comprehensions
are translated by Wadler's scheme, and `do` blocks become calls of `>>=`
and `>>`.  `case` and lambda expressions become local function definitions
with invented `vN` names.  A `newtype` constructor disappears, and so does
the `_indirect` that the type checker's in-place rewriting left behind.

Very little can go wrong from here on.  The type checker has already
accepted the program, so the only errors left are about size: a binding
too large for the code fails with "Compiled code too complex", and an
expression nested too deeply for these recursive passes usually reports
`Control stack overflow`.

A pattern binding is the interesting case.  It can't stay a pattern,
because it has to be lazy, and nothing can match until something demands
the value.  So

<!-- source: Compile.hs -->
```haskell
firstOf :: (a, b) -> a
firstOf p = let (x, _) = p in x
```

compiles to two local variables, a checked copy of `p`, call it `u`, and an
`x` that picks the first field out of `u` when asked.  `ALLOC 2` makes two empty
nodes for them, at `stack(root+2)` and `stack(root+3)`, and `UPDAP n` is
`RUPDAP` aimed at `stack(root+n)` and without the return:

<!-- session: firstOf-listing -->
```
firstOf, arity 1
        ALLOC   2
        INT     1
        LOAD    3
        CELL    (,)
        CELL    _SEL
        MKAP    2
        UPDAP   2             ; x := _SEL (,) u 1
        LOAD    1
        CELL    check
        UPDAP   3             ; u := check p
        LOAD    2
        RUPDATE               ; the result is x
```

`_SEL` (`nameSel`) extracts the `n`th field of a value built with a given
constructor.  It has no valid Haskell type, which doesn't matter because
it is introduced after type checking.  `check` stands for the conformality
check, an invented function that evaluates its argument, so a pattern that
can't match fails at the moment `x` is demanded and not before.

## Matching: from equations to a case tree

What remains is pattern matching in function definitions.  The
pattern-matching compiler (`pmcTerm`, `match`) rewrites a function into a
tree of `CASE`s on single constructors, `NUMCASE`s on numeric literals,
guards and `FATBAR`s.  Hugs rewrote this pass to handle record syntax, so
it isn't quite the algorithm of Wadler's that the Gofer report follows
(RR-1030, p. 35), but the output has the same shape.  For `total`, which
by now takes a dictionary as its first argument, the output is, schematically:

```
CASE offset 1 of
  []  -> fromInt (offset 2) 0
  (:) -> (+) (offset 2) (offset 4) (total (offset 2) (offset 3))
```

This output has an unusual property: **it contains no binding occurrences
of variables.**  Patterns are reduced to bare constructors, and a local
variable is an `OFFSET`, a number.  (A local function keeps a name until
the next stage lifts it out.)  The last of `a` arguments is offset 1 and
the first is offset `a`, so the list is 1 and the dictionary is 2.  The
fields of a constructor take the next numbers up, the last field first
(`advance` spools them off the right-hand end of the pattern), so the tail
is 3 and the head is 4.  The functions in `compiler.c` carry a "current
offset" parameter, `co`, to know what the next number will be.

In `total`, offset 3 is `LOAD 3`, and that was the original point of the
numbering.  The report gave two reasons for it (RR-1030, p. 36).  Leaving
out the binders saves a little space, which mattered on the PC.  More importantly, in Gofer the
offsets were the variables' positions on the machine's stack when the
function ran, so its code generator needed no environment mapping
variables to stack slots.  Cupid's does keep such a map, `offsPosn[]`,
because with temporaries on the stack the two numbers can differ; Hugs made
it track the stack at compile time.

A guard that falls through needs a way of saying "try the next equation".
That is `FATBAR (l, r)`: `l`, or `r` if `l` fails to match.  Take this
function, whose guards can all fail:

<!-- source: Compile.hs -->
```haskell
classify :: Int -> String
classify n
  | n < 0     = "negative"
  | n == 0    = "zero"
classify 1 = "one"
classify _ = "many"
```

Here is the code, with the first guard left out.  The test of each guard
is compiled to an `EVAL`, because the function can't go on until it has
the answer, so `n == 0` is computed here, by `eval` and whatever it calls:

<!-- session: classify-listing -->
```
classify, arity 1
        ...
L1:     INT     0
        CELL    dNumInt
        CELL    fromInt
        MKAP    2
        LOAD    1
        CELL    dEqInt
        CELL    ==
        MKAP    3             ; (==) dEqInt n (fromInt dNumInt 0)
        EVAL
        TEST    True  L2      ; guard false: on to the next equation
        STRING  zero
        RUPDATE
L2:     SETSTK  1             ; reset the stack to the argument alone
        LOAD    1
        INT     1
        CELL    dNumInt
        CELL    primPmInt
        MKAP    3             ; primPmInt dNumInt 1 n
        EVAL
        TEST    True  L3
        STRING  one
        RUPDATE
L3:     SETSTK  1
        STRING  many
        RUPDATE
```

`FATBAR` is a construct and not a primitive so that the code generator
can recognise it and compile most occurrences away.  Here it has become a
jump target and a `SETSTK`, which is there to discard anything a failed
branch left on the stack (in this listing nothing is).  A numeric literal
in a pattern is a `NUMCASE`, and since it's overloaded like any other
literal it can't be compared directly: `primPmInt` is a small `Prelude`
function, `fromInt n == x`, and the code tests its result.

The same pass collects, for each local function definition, its free
variables and the local functions it calls, into a record `(Var, arity,
(Fvs, FFs, Rhs))`.  That is bookkeeping for the next stage, merged into
this one to avoid another traversal.

## Lambda lifting

The machine runs only closed global functions, so local definitions must
go.  In

<!-- source: Compile.hs -->
```haskell
scale :: Int -> [Int] -> [Int]
scale k xs = go xs
  where go []     = []
        go (y:ys) = k * y : go ys
```

`go` uses `k`, so it isn't closed.  Lambda lifting closes it by adding `k`
as a parameter and moving it to the top level.  Here is what comes out for
it, under an invented name, here written `go'`:

<!-- session: go-listing -->
```
go', arity 2
        LEVAL   1
        TEST    []  L1
        CELL    []
        RUPDATE
L1:     TEST    :   FAIL
        LOAD    3             ; ys
        ROOT    1             ; "go' k", from the current call
        MKAP    1
        LOAD    4             ; y
        LOAD    2             ; k
        CELL    dNumInt
        CELL    *
        MKAP    3
        CELL    :
        MKAP    1
        RUPDAP
```

and `scale` itself shrinks to a single call:

<!-- session: scale-listing -->
```
scale, arity 2
        LOAD    1
        LOAD    2
        CELL    go'
        MKAP    1
        RUPDAP                ; result: go' k xs
```

Mutually recursive local functions take more care.  If `f` mentions `k`
and calls `g`, and `g` mentions `m` and calls `f`, then `f` needs `m` as
well, only to pass it on.  The smallest correct sets of extra parameters
are the least solution of

    lv(f) = fv(f) ∪ ⋃ { lv(g) | g called from f }

which `solve` finds by iterating from `fv(f)` until nothing changes, using
the `Fvs` and `FFs` lists the previous stage left.  This is Johnsson's
algorithm.  Calls to a lifted function need the extra arguments added;
rather than substituting through the bodies, `lift` carries an environment
`tr` that maps each local function to its lifted call, extended before the
scope of each `let` is entered.

Moving the function leaves its offsets wrong.  A lifted function's body
still numbers its variables as they were where it was defined, continuing
the enclosing function's numbering, and its free variables are offsets of
the enclosing function.  `preComp` renumbers them for the function's new
home, where it will find its original arguments at offsets `1..a`, the `e`
extra parameters at `a+1..a+e`, and its temporaries above that.  So in
`go'` the list is offset 1 and `k` is offset 2.

## Generating code: two ways to compile an expression

`codeGen` in `machine.c` is the last stage, and it has a decision to make
at every subexpression: is this certainly going to be needed?  If so it
can evaluate as it goes, which is cheaper.  If not, it must build a graph
and leave it.  So there are two compilation functions:

- `build` generates code that constructs the expression and evaluates
  nothing.  It is used for arguments, and for the right-hand sides of
  local definitions.
- `make` generates code for an expression whose value is wanted now.  It
  may evaluate, test and branch.

The clearest way to see the difference is a conditional in both positions.
`&&` is an ordinary function in the `Prelude`, but `make` knows about it,
and treats `x && y` as `if x then y else False`:

<!-- source: Compile.hs -->
```haskell
both :: Bool -> Bool -> Bool
both a b = a && b
```

<!-- session: both-listing -->
```
both, arity 2
        LEVAL   2             ; evaluate a
        TEST    True  L1
        LOAD    1
        RUPDATE               ; result: b
L1:     CELL    False
        RUPDATE
```

Now put a conditional where nobody yet needs it:

<!-- source: Compile.hs -->
```haskell
lazyIf :: Bool -> Int -> Int
lazyIf c x = let y = if c then x else 0 in y + y
```

<!-- session: lazyIf-listing -->
```
lazyIf, arity 2
        ALLOC   1             ; an empty node for y
        INT     0
        CELL    dNumInt
        CELL    fromInt
        MKAP    2
        LOAD    1
        LOAD    2
        CELL    _IF
        MKAP    2
        UPDAP   3             ; y := _IF c x (fromInt dNumInt 0)
        LOAD    3
        LOAD    3
        CELL    dNumInt
        CELL    +
        MKAP    2
        RUPDAP                ; result: (+) dNumInt y y
```

There is no `TEST` in it.  `_IF` (`nameIf`) is a primitive that does the
test if and when someone evaluates `y`, at the price of building all three
arguments first, so the strict form is preferred wherever it is sound.
`FATBAR` gets the same treatment, with `nameFatbar`, when it turns up in a
lazy position.

`make` is also what compiles the head of an application in the result
position, because the function being applied is certainly needed.  Here
the conditional picks the function, and it's the argument that stays lazy:

<!-- source: Compile.hs -->
```haskell
strictHead :: Bool -> Int -> Int -> Int
strictHead c x y = (if c then negate else id) (x + y)
```

<!-- session: strictHead-listing -->
```
strictHead, arity 3
        LOAD    1
        LOAD    2
        CELL    dNumInt
        CELL    +
        MKAP    3             ; the argument, (+) dNumInt x y, unevaluated
        LEVAL   3             ; evaluate c
        TEST    True  L1
        CELL    dNumInt
        CELL    negate
        MKAP    1
        GOTO    L2
L1:     CELL    id
L2:     RUPDAP                ; result: the chosen function applied to it
```

So what a listing evaluates comes down to a short list.  It evaluates the
argument a pattern inspects (`LEVAL`, or `EVAL` when the expression isn't
just a variable), the test of a guard or a conditional in a strict position,
`&&` and `||` by way of those conditionals, and the comparison for a numeric
literal.  A constructor with a strict field evaluates it too, with `STAP`, in
the small function `implementCfun` writes for that constructor.  Everything
else is built.

`make` takes a label saying where to go when it's finished, and a few
special labels stand for ways of finishing.  `UPDRET` means overwrite the
root and return, `VALRET` means return the value on the stack, for a
constant applicative form that has no root to overwrite, and `RUNON` means
carry on with what comes next.

Generating this code requires knowing where each variable is, and the
stack moves as code runs.  `CodeGen`, the record `codeGen` creates and
passes to everything it calls, has `srsp`, a simulation of the stack
pointer at this point in the code, and `offsPosn[]`, the map from a
variable's offset to its slot.  Hugs added the simulation; Gofer did this
only in its C compiler (RR-1030, p. 36).

### Small improvements on the way out

The assembler does a little peephole work within a basic block.  `LOAD n;
EVAL` becomes `LEVAL n`, adjacent `MKAP`s and `SLIDE`s merge, and an `MKAP`
followed by an update becomes `UPDAP` or `RUPDAP`, which can reuse the node
being overwritten and allocate nothing.  So almost every listing here ends
in `MKAP` and `RUPDAP` where you might expect one bigger `MKAP` and an
update.  `asTEST` emits nothing at all for a tuple or a single-constructor
type, since the type checker guarantees the match; `firstOf`'s check above
has no test in it.

### The root optimisation

Many functions pass the same leading arguments to every recursive call:
`map f` in `map f (x:xs) = f x : map f xs`.  `analyseAp` notices when a
prefix of the root can be reused, and emits `ROOT n`.  For `map` it saves
one of the four application nodes the reduction would otherwise allocate.
Gofer's author described this in a newsgroup post in October 1991, with figures
suggesting a 25-30% reduction in space on some small examples, and in 1994 knew of
no other system that used it (RR-1030, p. 45).

Lambda lifting makes the pattern common, since it adds parameters that
never change; that's the `ROOT 1` in `go'` above.  So does dictionary
passing, which is the `ROOT 1` in `total`.  Now look at what happens when
you give `total` a monomorphic signature, the first exercise in [the
tour](how-cupid-runs-a-program.md#try-it):

<!-- session: mono-listing -->
```
total, arity 1
        LEVAL   1
        TEST    []  L1
        INT     0
        CELL    dNumInt       ; the dictionary is now a constant
        CELL    fromInt
        MKAP    1
        RUPDAP
L1:     TEST    :   FAIL
        LOAD    2             ; the tail has moved down one
        CELL    total         ; and there is no prefix left to reuse
        MKAP    1
        LOAD    3
        CELL    dNumInt
        CELL    +
        MKAP    2
        RUPDAP
```

The prefix of `total xs` is just `total`, a constant, and `CELL total` is
as cheap as anything.  The `+` still goes through the `Num Int` dictionary,
since nothing here specialises.

## The machine

`eval(n)` in `machine.c` reduces `n` to weak head normal form: it stops
when the outermost thing is a constructor, a constant or a function short
of arguments, and leaves everything inside unevaluated.  It's about a
hundred lines and repays reading.  It starts by walking down the spine of
applications, pushing each node, until it reaches whatever is at the head.
What happens then depends on the head:

- A function with enough arguments has them rearranged on the stack and
  is run.
- A constant, a constructor or a function with too few arguments is
  already in weak head normal form.
- A function with no arguments at all is a *constant applicative form*.
  Its code runs once and what it returns is stored in its name record
  (`defn`), so the second use costs nothing.

The stack layout we started with comes from this walk.  Walking down the
spine pushes the outermost application first, and that's the one holding
the last argument, which is why the offsets count from the end.  `LEVAL 1`
then calls `eval` on the list, recursively.  Suppose the list is `x : xs`.
That `eval` reaches a constructor with two arguments and stops, leaving the
constructor in a register, `whnfHead`, and the fields on the stack in place
of the application nodes:

```
stack(root+3)   xs
stack(root+4)   x
```

`TEST c l` compares `whnfHead` with `c`.  C code uses the same convention:
a primitive that walks a list calls `eval`, tests `whnfHead` against
`nameCons`, then pops the head and the tail.

The last instruction overwrites the root and returns to `eval`.  Here is
what happens next, with the profiling hooks left out:

```c
if (nameRec(n)->primDef)
    (*nameRec(n)->primDef)(root);
else
    run(nameRec(n)->code, root);
numReductions++;

sp = root;
n = pop();
...
goto unw;
```

`eval` doesn't return.  It picks up the overwritten root and goes round
again, unwinding what `total` just built.  So a Haskell tail call involves
no C recursion at all, which is what [the trace in the
tour](how-cupid-runs-a-program.md#running-the-program-is-one-more-expression)
was showing: `total` and `+` at the same depth, one after the other.  C
recursion happens only where something needs a value before it can
continue: `EVAL`, `LEVAL` and `STAP` in a function's code, and the
primitives.  `primPlusInt` is a primitive, so `total` on a long list goes
deep.

That depth has limits, and they are why this is an error and not a crash.
`eval` counts its own nesting against `MAX_EVAL_DEPTH` and calls
`stackCheck` each time, which compares the address of a local variable with
a limit worked out from the C stack's size, so a program that goes too deep
reports `Control stack overflow`.  At the default stack limit the count is what
stops `total`; with a much smaller stack the check would.

Most of the recursive passes of the type checker and the compiler call
`stackCheck` too, but not all the C recursion during loading does: `depTypeExp`
and `expandArgs` in `typeexp.c`, `transComp` in `compiler.c` and `lowlink` in
`scc.c` don't.  [Known bugs](../reference/language.md#known-bugs) gives the kinds of
source file that can run those off the end of the stack, into a segmentation fault.

### Updating, and why there are two ways

When a function's result is an application it has just built, `RUPDAP` can
make the root *be* that application, writing both fields.  When the result
is something that already exists, such as a variable, copying it into the
root would lose sharing.  So `RUPDATE` writes an indirection, `INDIRECT` in
the first field of the root, and the evaluator follows indirections as it
meets them.  That's the whole of `id`:

```
LOAD    1
RUPDATE
```

### Dictionaries are just data

Class dictionaries need nothing from the machine.  The `Num Int`
dictionary is a constant applicative form, and when it's first needed its
code runs like any other.  `ALLOC n` makes `n` empty nodes, one per field and
one for the dictionary, and `UPDATE` and `UPDAP` fill them:

<!-- session: dictionary-code -->
```
        ALLOC   10
        CELL    dEqInt
        UPDATE  1
        CELL    dShowInt
        UPDATE  2
        CELL    primPlusInt
        UPDATE  3
        ...
        CELL    fromIntInt
        LOAD    9
        ...
        LOAD    1
        CELL    Make.Num
        MKAP    9
        UPDAP   10
        LOAD    10
        RETURN
```

`Make.Num` is a constructor with ten fields: the two superclass
dictionaries and the eight methods.  And `+` is an ordinary selector,
compiled as `LEVAL 1; LOAD 9; RUPDATE`: evaluate the dictionary, take a
field.

Gofer's machine had a `DICT` instruction, which went when Hugs made
dictionaries ordinary heap values built by per-instance functions.  It also
had `INTEQ`, `INTGE` and `INTDV` for integer constants, `p+k` patterns and `n*v`
patterns (RR-1030, p. 44); these are gone too, and a numeric literal is now
matched through a `Prelude` function.

### The instruction set

Here is all of it.  `root` is the position of the application being reduced
and `sp` is the stack pointer:

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
| `LEVAL n` | `EVAL` of `stack(root + n)` without the push |
| `STAP` | apply strictly: evaluate the argument before building the application, for a strict constructor field |
| `TEST c l` | if `whnfHead` is not `c`, jump to `l` |
| `GOTO l`, `RETURN` | control |
| `FAIL` | the match failed |
| `SETSTK n` | set `sp` to `root + n`, discarding whatever a failed branch left on the stack |
| `ROOT n` | push the application obtained by removing the last `n` arguments from the one at `root` |
| `TABLE` | not executed: marks a table of a type's constructors stored in the code area, which derived instances use |

There is a single `FAIL` instruction, written once at a fixed address when
the machine is installed, and every failing branch jumps to it.  That's the
`FAIL` in the listings, where a raw listing prints its address, `0x0000`.

A string constant is kept as its `Text` and expanded into list cells lazily
as it is consumed (`evalString`), so a long literal costs nothing until
something reads it.

### One invariant to respect

`run` keeps a raw pointer into the code area in a register, and the code
area is an array that grows by `realloc`.  That is sound only because code
is never generated while evaluation is under way.  The prompt and the loader
compile first and evaluate afterwards, and no route leads from `eval` to the
compiler.  `growMemory` turns this into a checked invariant, reporting an
internal error if it's ever asked to grow with `evalDepth` nonzero.  An index
in place of the pointer would remove the rule, at the cost of an extra
register reload on every instruction.  A feature that compiled code from
inside a running program would have to deal with this first.

## Counting the work

The `+s` toggle prints how many reductions an evaluation took and how many
cells it allocated.  A reduction is what the `numReductions++` above counts:
one function or primitive applied to its arguments, or one constant
applicative form built.  Laziness shows up in the counts.  Take this program:

<!-- source: Queens.hs -->
```haskell
module Queens where

queens :: Int -> [[Int]]
queens n = place n
  where
    place 0 = [[]]
    place k = [ q : qs | qs <- place (k - 1)
                       , q  <- [1 .. n]
                       , safe q qs ]
    safe q qs = and [ q /= c && abs (q - c) /= d
                    | (d, c) <- zip [1 ..] qs ]
```

Here are two sessions, each starting fresh.  Your counts may differ with a
different revision of the libraries, but the first should stay far below the
second:

```
$ cupid -i Queens.hs
Queens> :set +s
Queens> head (queens 8)
[4,2,7,3,6,8,5,1]
(126557 reductions, 186588 cells)
```

```
$ cupid -i Queens.hs
Queens> :set +s
Queens> length (queens 8)
92
(2299829 reductions, 3385763 cells, 3 garbage collections)
```

`head` asks for the first solution and only that much gets computed, about
an eighteenth of the reductions `length` needs for all ninety-two.

These counts depend on what the session has already done.  Typing `length
(queens 8)` straight after `head (queens 8)` takes 2299817 reductions, and
typing it a second time takes 2299814.  The difference is the constant
applicative forms, such as dictionaries: each is built once per session and
charged to whichever expression needs it first.  So when you compare two
counts, compare them from fresh sessions, as the next section does.

## Seeing the code

A `checks` build has the `+D` toggle.  With it, `:load` prints each
definition as the code generator receives it and the instructions it
produces, and evaluating an expression traces `run` and `eval`.  A raw
listing has addresses where these have labels, and invented names such as
`v87` where these have `dNumInt`.  A raw cell prints as `$` and its integer
value, so `$302` in a `codeGen =` line is an offset, and `$203` in a listing
is what's written `(,)` above.

These exercises are optional, and they matter more once you've read the whole
chapter.  The programs are in `tests/doc-sessions/Exercises.hs`.  Write down
what you expect before you look.

**1.** `swap :: (a, b) -> (b, a)`, with `swap (a, b) = (b, a)`.  How many
`TEST`s?

<details>
<summary>What it compiles to</summary>

<!-- session: swap-listing -->
```
swap, arity 1
        LEVAL   1
        LOAD    3             ; a
        LOAD    2             ; b
        CELL    (,)
        MKAP    1
        RUPDAP
```

No tests: a pair has one constructor, so evaluating it is all the matching
needed.  It does still evaluate, because the pattern is strict.  Fields get
their offsets from the right, so `b` is 2 and `a` is 3, and the result is
built with its last field first.

</details>

**2.** `twice :: (a -> a) -> a -> a`, with `twice f x = f (f x)`.  Is anything
evaluated?

<details>
<summary>What it compiles to</summary>

<!-- session: twice-listing -->
```
twice, arity 2
        LOAD    1             ; x
        LOAD    2             ; f
        MKAP    1             ; f x
        LOAD    2             ; f
        RUPDAP                ; result: f (f x)
```

Nothing.  `eval` will carry on from the new root and apply the outer `f`.

</details>

**3.** Compare these two.  Which one contains a `TEST`?

<!-- source: Exercises.hs -->
```haskell
isZero :: Int -> Bool
isZero n = if n == 0 then True else False

pick :: Bool -> Int -> Int
pick c x = g (if c then x else 0) where g y = y
```

<details>
<summary>What they compile to</summary>

`isZero` has the conditional in the result position, so it is compiled by
`make`:

<!-- session: isZero-listing -->
```
isZero, arity 1
        INT     0
        CELL    dNumInt
        CELL    fromInt
        MKAP    2
        LOAD    1
        CELL    dEqInt
        CELL    ==
        MKAP    3
        EVAL
        TEST    True  L1
        CELL    True
        RUPDATE
L1:     CELL    False
        RUPDATE
```

In `pick` it's an argument, so it is compiled by `build`, and `g` receives it
unevaluated:

<!-- session: pick-listing -->
```
pick, arity 2
        INT     0
        CELL    dNumInt
        CELL    fromInt
        MKAP    2
        LOAD    1
        LOAD    2
        CELL    _IF
        MKAP    3
        CELL    g'
        RUPDAP
```

</details>

**4.** Write `map` for yourself and annotate its listing as was done for
`total`.  Which node does `ROOT` save?  Then look at the listing for the
`Prelude`'s own `map`, by starting a `checks` build with `CUPIDFLAGS=+D` in the
environment so that the `Prelude`'s compilation is printed too.  Why is it two
functions?

**5.** Change the source.  How much is the root optimisation worth today?  In
`analyseAp` in `machine.c`, make the test `e == cg->definingName` always false,
so that `rootPortion` stays 0 and every argument is built.  Rebuild, and use
`:set +s` to compare `length (queens 8)` in a fresh session before and after.
Before you run it, decide which of the two numbers `+s` prints ought to change.

<details>
<summary>What happens</summary>

The regression suite still passes, as it should for an optimisation.  The
reductions don't change at all (2299829 in both, at the revision above), and the
cells go from 3385763 to 3770207, about 11% more.  That's a long way from the
report's 25-30%, though the workload is different and it says nothing about what
the figure was in 1991.

If you tried disabling the `ROOT` branch in `buildAp` instead, the library build
fails.  With the branch gone, `buildAp` hands `make` the same expression it was
given, and `make` calls `buildAp` again.  The two functions rely on `analyseAp`
having told the truth.

</details>

Peyton Jones's *The Implementation of Functional Programming Languages* (1987) is
the place to go next, for the G-machine, pattern-matching compilation and lambda
lifting that this follows.
