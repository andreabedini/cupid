# How Cupid runs a program

Here is a program, in a file called `Total.hs`:

<!-- source: Total.hs -->
```haskell
module Main where

total :: Num a => [a] -> a
total []     = 0
total (x:xs) = x + total xs

main :: IO ()
main = print (total [1, 2, 3 :: Int])
```

<!-- session: total-run -->
```
$ cupid Total.hs
6
```

Eight lines, and they already need the layout rule, type-class
overloading, pattern matching and lazy evaluation.  This chapter follows
them from the bytes in the file to the `6`, watching what `total` turns
into at each stage.  The later chapters each take one stage and go deep;
the point here is to have the whole path in your head, so that you know
where you are when you get there.

## The whole compiler is five lines

`main` in `cupid.c` looks at its arguments, decides it has been given a
file to run, and calls `runFile` in `run.c`.  Before `runFile` looks at
your file it loads the `Prelude`, because nothing is ever compiled ahead
of time: a script is a script, whether it is `Total.hs` or the 2,500-odd
lines of `Cupid/Prelude.hs`, and each one goes through `addScript` in
`script.c`.  With the error handling and bookkeeping taken out, this is
what is left of `addScript`:

```c
parseScript(fname, len);
if (needsImports)
    return false;
checkDefns();
typeCheckDefns();
compileDefns();
```

Those calls are the compiler.  What is odd about them, if you are used
to reading modern compilers, is that none of them takes a program as an
argument or returns one.  There is no syntax tree type, no typed tree
type, no core language type.  `parseScript` leaves its result in global
variables (`valDefns`, `tyconDefns`, `classDefns`, `instDefns` and a few
more) and in the symbol tables, and each stage after it picks those up
and rewrites them in place.  One representation, destructively updated
from the parser to the code generator, is what the rest of this tour
keeps coming back to, and the next chapter describes it.

`needsImports` is for when the parser meets an import.

## Characters become tokens

`yylex` in `input.c` hands the parser one token at a time.  Set a
breakpoint on it and print what it returns for `Total.hs`, and you get
this, with line breaks added and the marker for the start of the input
left out:

<!-- session: tokens -->
```
module CONID where {
  VARID :: CONID VARID => [ VARID ] -> VARID ;
  VARID [ ] = NUMLIT ;
  VARID ( VARID CONOP VARID ) = VARID + VARID VARID ;
  VARID :: CONID ( ) ;
  VARID = VARID ( VARID [ NUMLIT , NUMLIT , NUMLIT :: CONID ] )
}
```

The file has no braces and no semicolons.  The lexer put them there.
After the keyword `where` it looks at the next character, and since that
isn't a `{` it returns one anyway and remembers the column of the token
that follows, here column 1, on a stack of open blocks (`goOffside`).
From then on, a token that starts a line at that column gets a `;` in
front of it, and one that starts further left closes the block.  End of
file counts as column 0, which is where the final `}` comes from.  So
the grammar in `parser.y` never hears about indentation, with one
exception that [How source is read](how-source-is-read.md#lexing)
explains.

Each token also has a value.  The occurrences of `total` are separate
tokens that all carry the same `Text`, a number identifying the string
in a table where every name is stored once.  From here on, comparing two
names means comparing two integers.

## Tokens become cells

`parser.y` is a bison grammar whose actions build the program in the
heap, out of the same material the running program will be made of.  The
heap is an array of pairs of `int`s.  A `Cell` is one such `int`: a
negative one is the index of a pair, and a non-negative one is a small
constant, a tag or a handle into a symbol table.  You can get a long way
knowing only that.  The second equation is parsed by this rule:

```
decl : funlhs rhs   {$$ = ap(FUNBIND, pair($1, $2));}
```

(the real action also keeps the collector informed), and comes out as a tree of pairs, which we can write schematically like
this:

```
FUNBIND (total (x:xs),
         (5, INFIX [x, +, total xs]))
```

The 5 is the line number, there for error messages.  `x + total xs` is
deliberately left as a flat sequence of operands and operators, because
the parser can't know the fixity of `+`: a fixity may be declared later
in the file or in an imported module, and the imports have not been
processed yet.  The list `[1, 2, 3 :: Int]` is a `FINLIST` cell, and the
signature is a declaration in the same list as the equations.  A `data`,
`class` or `instance` declaration, had there been one, would have gone
straight into its table during parsing.

Had the file imported a module that isn't loaded, parsing would have
stopped right after the import declarations.  The grammar's `chase`
action queues the missing modules ahead of this one and returns from the
parser; `addScript` sees `needsImports` and gives up; `readScripts` then
loads the imports and comes back to read this file again from the top.
So a module's imports can be chased without a separate pass to find them,
at the price of parsing the first lines of such a file twice.

## Static analysis: names and order

`checkDefns` in `static.c` runs the static checks, and its comment says
what kind of code to expect: "these steps work by side effect, so their
order matters".  Most of the steps have nothing to do for this program,
since it declares no types or classes.  It still gets an implicit
`import Prelude`, and three things then happen to `total`.

`eqnsToBindings` in `binding.c` gathers the two equations and the
signature into a single binding, checking that both equations take the
same number of arguments.  Schematically:

```
(total, (signature, [ ([ []     ], (4, 0)),
                      ([ (x:xs) ], (5, INFIX [x, +, total xs])) ]))
```

The signature has become a type scheme in which type variables are
numbered rather than named, in `POLYTYPE` and `QUAL` cells around
`[OFFSET 0] -> OFFSET 0`.  The numbering lets the type checker use `total`
at a new type without copying its signature.

`topDependAnal` in `depend.c` then walks each binding resolving names.
`print` and the constructor `:` are imported, so the identifier is
replaced by the `Name` cell itself and nothing later has to look them
up.  The flat `INFIX` chain is now resolved too, by `tidyInfix` in
`fixity.c`, because by this point the imports are in and the fixity of
`+` is known, so it becomes `(+) x (total xs)`.

Finally the bindings are sorted into groups of mutually recursive
definitions, each group after the ones it uses.  `main` uses `total`, so
`valDefns` is now `[[total], [main]]`.  Inference needs that order
because an unsigned definition's type must be generalised before its
users are checked, or it could be used at only one type.  [How source is
read](how-source-is-read.md#static-analysis) has an example.

## Type checking rewrites the program

`typeCheckDefns` in `type.c` takes those groups in order.  This stage
doesn't only check: it inserts what overloading needs, by overwriting
the expression it is checking.

Cupid implements a class constraint as a hidden argument: a record of
the class's methods, called a *dictionary*.  `Num a =>` means that
`total` receives the `Num` dictionary for whatever `a` turns out to be.
So while the body is checked, `Num a` is something it may assume (a
*given*, in the type checker's vocabulary), and every overloaded thing
in the body asks for a dictionary of its own.  The literal `0` has the
type `Num t => t`, so `typeExpr` turns it into `fromInt dv 0`, where `dv`
is a fresh placeholder cell.  `+` and the recursive `total` get
placeholders the same way.  The constraint solver in `preds.c` then
works out that each of them is asking for `Num a`, the given one, and
overwrites each placeholder with the parameter.  Nothing has to walk the
body a second time.  The result is as if you had written:

```haskell
total d []     = fromInt d 0
total d (x:xs) = (+) d x (total d xs)
```

In `main` the same questions are asked at `Int`, where there is an
instance to answer them, so the placeholders become the dictionaries for
`Num Int` and `Show Int`.  These are ordinary global values with invented
names; the listings below call them `dNumInt` and `dShowInt`.

From here on there are no type classes.  A dictionary is an argument
like any other and `(+)` is a function that takes one.  Keeping it so is
one of the rules for [what gets into the
language](language-design.md#how-a-feature-gets-in): nothing below the
type checker knows that predicates exist.

## Compiling produces code that builds graphs

`compileDefns` in `compiler.c` takes each binding through a few passes.
`transBinds` reduces it to a smaller language, `match` turns the
equations into a tree of single-constructor case tests, `lift` moves
local functions to the top level and `preComp` fixes up the numbering of
variables; then `codeGen` in `machine.c` emits instructions.  With the
`+D` toggle a `checks` build prints what comes out: type `:set +D` and
then `:load Total.hs`.

The machine has a stack, and a listing is a program for it.  `LOAD n`
pushes a slot of the current call, `CELL` pushes a global name, and `MKAP
n` applies the top item to the `n` below it, so expressions appear with
the function last.  `RUPDAP` applies one more argument and overwrites the
call with the result.  Here is `total`, with addresses replaced by labels
and comments added ([how these listings are
made](reading-cupid-in-a-day.md#how-the-output-here-was-made)):

<!-- session: total-listing -->
```
total, arity 2
        LEVAL   1             ; evaluate the list, the last argument
        TEST    []  L1        ; if it isn't [], go to L1
        INT     0
        LOAD    2             ; the dictionary, the first argument
        CELL    fromInt
        MKAP    1
        RUPDAP                ; result: fromInt d 0
L1:     TEST    :   FAIL      ; if it isn't (:), fail
        LOAD    3             ; the tail
        ROOT    1             ; "total d", reused from the current call
        MKAP    1
        LOAD    4             ; the head
        LOAD    2             ; the dictionary
        CELL    +
        MKAP    2
        RUPDAP                ; result: (+) d x (total d xs)
```

There is no addition in there, and `total` doesn't call itself.  The
code evaluates its argument just far enough to choose a branch, *builds*
that branch's right-hand side as a graph of unevaluated applications,
overwrites the call with that graph, and returns.  It's a graph because
two uses of a variable point at one node, so evaluating it once serves
both.  A function's code does whatever evaluation it needs to decide what
to build, and that can be a good deal when a guard or a conditional asks
for it; but it hands back the result unevaluated, for whoever is asking
to carry on from.  [How code is compiled and
run](how-code-is-compiled-and-run.md#reading-a-listing) takes this listing
apart.

## Running: the program is one more expression

With the files loaded, `runFile` calls `runProgram` in `evaluator.c`,
which feeds the interpreter a line of source text:

```c
stringInput("main >> return () :: IO ()");
```

That string goes down the same path as your file, through `parseExp`,
`checkExp`, `typeCheckExp` and `compileExp`, and what comes out is a
little code that builds a graph.  `eval` in `machine.c` then reduces the
graph until there is nothing left to do.  This is also exactly how an
expression at the prompt is run, apart from what is wrapped around it to
print the result.

`print` asks for a string, and showing an `Int` needs the `Int`, so
`primShowsInt` eventually evaluates `total dNumInt (1 : 2 : 3 : [])`.
The same `+D` toggle traces the reduction.  Here is the start of it for
`total [1, 2, 3 :: Int]` typed at the prompt, from `primShowsInt` on,
filtered down to the names being entered, with invented names written
descriptively.  The indentation shows the height of the machine's stack:

<!-- session: total-trace -->
```
Entering primShowsInt
    Entering total
       Entering :
    Entering +
        Entering dNumInt
        Entering Make.Num
    Entering primPlusInt
       Entering fromInt
          Entering dNumInt
          Entering Make.Num
       Entering fromIntInt
       Entering total
          Entering :
       Entering +
           Entering dNumInt
           Entering Make.Num
```

This is the program from the top of the chapter, run in demand order.
`total` evaluates its list as far as the first `:` and replaces itself
with `(+) d x (total d xs)`.  `eval` carries on from the replacement, and
now the head of the application is `+`.

A class method is a function that takes a dictionary apart.  So `+`
forces `dNumInt`, whose value is an application of the constructor
`Make.Num`, and selects one of its fields, `primPlusInt`, which is C at
last.

That primitive needs both its arguments as integers, so it calls `eval`
on each: first `fromInt dNumInt 1`, which is how `1` gets to be an `Int`
(`fromIntInt` stands for `fromInt`'s definition at `Int`), then `total d
xs`, one level deeper.

`:` and the dictionary dip in and return, but each `primPlusInt` stays
where it is, waiting in C for a value, inside a nested call of `eval`.
`eval` counts how deep it is against a fixed limit, `MAX_EVAL_DEPTH`, and
checks the C stack as well.  So `total [1 .. 16000 :: Int]` is fine and
`total [1 .. 20000 :: Int]` reports `Control stack overflow`, at the default
stack limit or a much larger one, because the count is what stops it.

## What this design costs, and what it buys

Everything is compiled every time.  There is no object code for modules,
so every run recompiles the `Prelude`, and `:reload` recompiles the first
module whose file has changed and every module loaded after it, whether
or not they depend on it.  That sounds worse than it is: `time cupid` on
a program that prints one line, default build, x86-64 Linux, took between
20 and 50 milliseconds, Prelude included.  Execution is another matter.
There is no optimising pass, only some local tricks in the code
generator, as the `fromInt` call at every use of a literal shows; even a
`total` declared at `[Int] -> Int` goes through the dictionary to find
`+`.  Nobody has benchmarked Cupid against a compiler for these pages, so
they give no figure for how much slower that makes it.

In exchange, the stages above are all there is: no second representation
to learn, no optimiser to read past, and no Haskell compiler involved in
building it.  Under fifty thousand lines of C is short for a language
with type classes, and it's what makes it reasonable to set out to read
the thing.

## Try it

These are optional.  Before running any of them, write down what you
expect.

Change the signature to `total :: [Int] -> Int` and look at the `+D`
listing.  What happens to the arity, and to the `LOAD 2`s?  The `ROOT 1`
goes too.  Can you see why, given what it was reusing?

Delete the signature and ask `:type total` at the prompt.  Would you
expect the code to change?  Look.

Change `main` to `print (head [1, total undefined])`.  What does it
print?  Look at the listing for `main`: which instruction would evaluate
`undefined`?

The first of these is worked through in [How code is compiled and
run](how-code-is-compiled-and-run.md#the-root-optimisation).
