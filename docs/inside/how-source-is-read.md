# How source is read

Before Cupid can infer a type, it must know which characters make a name,
which expressions belong together, and which definition each name refers
to. Consider this small module, in `Source.hs`:

<!-- source: Source.hs -->
```haskell
module Source where

answer = twice (3 + 4 * 5)

twice x =
  let y = x + x
  in y
```

The definition of `answer` uses `twice` before its definition appears.
Multiplication groups more tightly than addition, and indentation marks
where the local definition of `y` ends. All three facts must be settled
before type inference starts.

The *front end* does this work in three parts. The **lexer** turns
characters into tokens, such as an identifier, a number or `let`. The
**parser** combines tokens according to the grammar, building trees for
expressions and declarations. **Static analysis** resolves names, checks
declarations and arranges bindings in dependency order. Cupid splits some
of the work differently from that simple outline: layout needs help from
the parser, and infix grouping waits for static analysis. This chapter
explains why, then connects the steps to the C.

## Lexing

A token has a kind and a value. In `answer = twice (3 + 4 * 5)`, the
lexer recognises `answer` and `twice` as identifiers, the digits as
numeric literals, and the punctuation and operators as separate tokens.
It consumes all of `answer` as one identifier.

The kind tells the parser what sort of symbol it has received. The value
distinguishes particular names or literals: an identifier token must
distinguish `answer` from `twice`. Cupid stores identifier text once in a table and carries its
`Text` handle in the token's value. At this point that handle identifies a
spelling, not the definition the identifier will refer to.

`yylex` in `src/input.c` returns one token at a time to the parser. It
keeps two characters of lookahead, `c0` and `c1`: characters available for
inspection before the lexer decides what to consume. For example, `{`
followed by `-` opens a comment, while `{` alone opens an explicit block.
Most token choices need no more lookahead than this.

The lexer also tracks rows and columns. Columns support layout; row
numbers are stored in the parsed form where later errors may need them,
such as right-hand sides, signatures and fixity declarations. A type
error can then name a line without the type checker reading the file.

## Indentation becomes punctuation

Haskell allows a block of declarations to be written with indentation or
with braces and semicolons. For our example, the layout rule supplies the
punctuation shown here, giving an equivalent spelling of the module:

```haskell
module Source where {
  answer = twice (3 + 4 * 5);
  twice x = let { y = x + x } in y
}
```

After `where`, the first declaration establishes the indentation of the
module body. `twice` begins at that same column, so it starts a new
declaration: the lexer inserts `;` before it. The `let` opens a nested
block whose indentation is the column of `y`. The `in` is further left,
so that block closes before `in`. End of file closes the module body.

Cupid implements this with a stack of open layout contexts. A context
records either an indentation column or the fact that the programmer
wrote an explicit `{`. After a layout keyword such as `let`, a missing
`{` causes `yylex` to insert one and remember the following token's column
(`goOffside`). At a later line's indentation, an equal column inserts
`;`, a smaller one inserts `}`, and a larger one continues the current
declaration. An explicit block instead uses the punctuation in the file.
Nested blocks need a stack because one token may close more than one.

Columns do not decide every case. In `let x = 1 in x`, `in` is on the
same line and is further right than the declaration's first token. It
still closes the declaration block: it can continue the expression only
after a `}`. The [layout specification](https://www.haskell.org/onlinereport/haskell2010/haskellch10.html#x17-17500010.3)
allows an implicit block to close when continuing without that brace
would cause a parse error.

The lexer cannot make that grammatical decision. Cupid uses the `error`
alternative of the grammar's `end` production: if the current block is
implicit (`canUnOffside`), it closes the block (`unOffside`) and resumes
parsing (`yyerrok`). That `error` alternative implements valid syntax as
well as participating in error handling. The grammar otherwise works
with explicit braces and semicolons, whether written or inserted.

## Expressions and types are trees

Tokens give the order of the symbols; a syntax tree gives their
structure. In `twice x`, one expression is applied to another. In
`twice (3 + 4 * 5)`, the parentheses make the entire arithmetic expression
the argument of `twice`. The parser records that grouping so later stages
do not have to recover it from characters.

A *grammar production* describes how a larger phrase is built from
smaller ones. Here is the application production from
[`src/parser.y`](https://github.com/andreabedini/cupid/blob/main/src/parser.y):

<!-- source: ../../src/parser.y -->
```text
appExp    : appExp aexp			{$$ = gcShadow(2,ap($1,$2));}
	  | aexp			{$$ = $1;}
	  ;
```

`aexp` is an atomic expression, such as a name, literal or parenthesised
expression; `appExp` is an application expression. In the first
alternative, `$1` is the function cell and `$2` the argument cell.
`ap($1,$2)` pairs them, and `$$` receives the result for the whole phrase.
`gcShadow(2,...)` replaces the two fragments on the collector's shadow
stack with that result; we will follow it below. The second alternative
passes an atom through unchanged.

Repeated application groups to the left: `f x y` becomes `(f x) y`, so
the pair for `f x` becomes the function half of the outer pair.

The trees use the `Cell` representation from [Cells, text and
tables](cells-text-and-tables.md). An application is a pair: one half
refers to the function, the other to its argument. Other constructs have
tags identifying their shape; an `if`, for example, is a `COND` node
holding a condition and two branches.

Types use the same representation. A type constructor can be applied to
type arguments, so `Bool -> Bool` is the constructor `(->)` applied first
to `Bool`, then to `Bool` again. Schematically, leaving out how names are
stored:

```text
expression:  f x             (f . x)
type:        Bool -> Bool    (((->) . Bool) . Bool)
```

Here `(u . v)` means a pair. In the C, `ap(f, x)` builds an application
pair, and `fn(a, b)` builds the two pairs for `a -> b`. `Type` is the C
alias for `Cell` used for these type trees. A type signature written in
the source is parsed into a tree too; inferring a type for an expression
is a later job.

## What the grammar leaves to later

In our example, parentheses establish the argument of `twice`, but they
do not settle how `3 + 4 * 5` groups. **Precedence** decides which operator
binds more tightly: multiplication groups first, giving `3 + (4 * 5)`.
**Associativity** decides how operators of equal precedence group. For
example, `a - b - c` means `(a - b) - c`, while `a : b : xs` means
`a : (b : xs)`.

These properties together are an operator's *fixity*. Haskell allows a
fixity declaration after the operator's use, and imported operators have
fixities too. The parser cannot commit to a grouping before it has that
information. It preserves the operands and operators as an `INFIX` chain,
with prefix minus represented by `NEG`.

Once imports and declarations have been processed, `tidyInfix` in
`src/fixity.c` turns that chain into an application tree. It keeps pending
operators on a stack and uses their precedences and associativities to
decide which applications to form. Inconsistent equal-precedence fixities
are reported as ambiguous. Our arithmetic expression becomes,
schematically:

```haskell
(+) 3 ((*) 4 5)
```

This groups operations; it does not perform them. The program will still
have to pass through type inference, compilation and evaluation before
`answer` can print `46`. `tidyInfix` runs during dependency analysis, from
`depExpr` and `checkPat`, when the operators' scopes and fixities are
available. Prefix minus has its own fixed precedence in this pass.

### Recognising a phrase before checking it

Some phrases cannot be classified from their first token alone. The
grammar is read by **Bison**, which generates the C parser from the
productions and their actions. Cupid's parser uses one token of
lookahead to choose its next action. Some Haskell constructs need a
later token to reveal what the earlier part meant, so the grammar reads
a broader form and checks it afterwards.

In `(C a, C b) => t`, the `=>` reveals that the preceding part is a
*context*: constraints on the type variables. Until then, it could have
been a tuple type. The grammar parses it as a type, then `checkCtxt`
checks that it can be a context. `Int => Bool` is rejected by that check
rather than by the grammar alone.

Likewise, `p <- e` in a `do` block, comprehension or guard starts like a
plain expression. The grammar reads the left side as an expression;
`checkPat` in `src/pattern.c` later checks and converts it to a pattern.
For example, `x` can bind a variable there, but `f x` cannot be a pattern
if `f` is a function rather than a data constructor. Elsewhere the
grammar has pattern productions of its own, including function heads,
`case` alternatives and lambdas. A wildcard left in an expression is
rejected by `depExpr` in `src/depend.c`.

### Rewriting syntax early

Some syntax is translated as soon as it is parsed. This is
*desugaring*: replacing a convenient surface form with a form the rest
of the interpreter already handles. A right section such as `(+ 1)`
becomes an application of `flip`, and `[1 .. 3]` becomes an application
of `enumFromTo`. A list literal is retained as a `FINLIST` node. Pattern
guards become hidden `Maybe` bindings and boolean guards.

The choice is whether later passes need to know the original form.
Translating a section here lets later stages process ordinary
applications. Keeping a `FINLIST` node lets later stages process its
elements together. Early desugaring reduces the number of forms every
later pass must handle; it does not remove all surface syntax at once.

## Static analysis

A tree can obey the grammar and still refer to an undefined name, repeat
a declaration, or use a data constructor with the wrong number of
arguments in a pattern. Static analysis checks these conditions before
type inference. It also resolves references and reorganises declarations
into the form the type checker needs. `checkDefns` in `src/static.c`
coordinates these passes.

### Finding the definition behind a name

In our module, the use of `twice` in `answer` refers to the top-level
binding. The two uses of `x` refer to the argument of `twice`, and the
last `y` refers to the local `let` binding. Recognising their spellings
was enough for the lexer; resolving them requires knowing **scope**:
which bindings are visible at each point.

For a nested scope, local bindings take priority over outer ones. In
`let x = True in (\x -> x + 1) 3`, the `x` in `x + 1` refers to the
lambda's argument, not the outer `x`. Both have the same spelling, so
text handles alone cannot distinguish them. `depExpr` walks expressions
with stacks of variables and bindings in scope; `depVar` searches those
stacks from the innermost scope out, then checks global names. An
unresolved name produces an error here.

References to constructors and imported functions or class methods
become `Name` handles. References to the current module's bindings can
remain variable cells, but they have been checked and linked to the
binding structures used for dependency analysis; `typeAp` and `pmcVar`
find their `Name`s later. Static analysis therefore resolves what a
reference means without requiring every reference to have its final
representation yet.

### Equations become bindings

The parser collects value equations, type signatures and fixity
declarations. `src/binding.c` associates them: `eqnsToBindings` makes one
binding for all the equations of a function, attaching its signature
and fixity if present. The equations must be contiguous and take the
same number of arguments. For a definition such as `total` from the
[pipeline tour](how-cupid-runs-a-program.md), its empty-list and nonempty-list
equations become two alternatives of one binding.

A pattern binding such as `(x, y) = pair` can introduce several names
at once. It stays a distinct kind of binding, with one shared pattern
and right-hand side; `binding.c` describes both representations.

Data, class and instance declarations begin entering their tables as
they are parsed; value equations are collected for these later passes.
This does not make source order the checking order. Most checks wait
until the whole module has been parsed, because declarations can refer
to later declarations. Primitive definitions are also collected for
checking, and a fixity is attached to its operator during binding
grouping, before expressions are traversed.

### Resolving written types

Type signatures need name resolution too. In an ordinary implicitly
quantified signature `a -> m a`, the two occurrences of `a` must identify
the same type variable. `src/typeexp.c` replaces type-variable names by
numbered offsets in order of first occurrence, and type-constructor and
class names by their table handles. Schematically:

```text
a -> m a
POLYTYPE ([*, * -> *], OFFSET 0 -> OFFSET 1 (OFFSET 0))
```

`a` is variable 0 and `m` is variable 1. The list records their *kinds*:
`a` stands for an ordinary type (`*`), while `m` takes a type to produce
a type (`* -> *`). The `POLYTYPE` wrapper records that these variables
are quantified. Scope and explicit `forall` declarations add cases to
this process; this is the simple case the next chapter starts from.
[How types are inferred](how-types-are-inferred.md#types-are-a-skeleton-and-an-offset)
explains how offsets allow each use of a type scheme to receive fresh
unknowns. A scheme is limited by `NUM_OFFSETS`.

## Definitions are ordered by dependency

Our module writes `answer` first, but the type checker needs to finish
`twice` first. The body of `answer` uses `twice`; the body of `twice`
does not use `answer`. Draw an arrow from a binding to a binding it
uses:

```text
answer --> twice
```

So these can be checked as two groups, `twice` before `answer`. The
local `y` is handled in its own scope. Source order is no guide to
this ordering.

Recursion adds cycles. These definitions depend on each other:

<!-- source: Source.hs -->
```haskell
evenLength [] = True
evenLength (_:xs) = oddLength xs
oddLength [] = False
oddLength (_:xs) = evenLength xs
```

Following calls from either function reaches the other and returns to
the start. They must be checked together. A **strongly connected
component** is a maximal group of bindings in which every binding can
reach every other by following dependency arrows. Each component
becomes one group, with groups ordered so dependencies come first.
A binding with no cycle still forms a component on its own.

`dependencyAnal` in `src/depend.c` walks the bindings to build those
arrows, calls `bscc`, and removes the temporary dependency fields.
`topDependAnal` does the top-level version. The sort is a depth-first
search in `src/scc.c`, based on Tarjan's algorithm; `sccSort` receives
accessors saying where each caller keeps its dependency information.
It is also used for type constructors and classes before kind
inference, because those declarations can depend on each other too.

Grouping matters for polymorphism, not just for finding names. With
implicitly typed definitions, uses within a recursive group share
provisional types until the group is finished. A later group can use a
completed binding at different types. In this expression:

```haskell
let identity x = x
    successor y = identity y + 1
in identity True
```

`identity` can be checked first and used independently by `successor`
and by the body. If `identity` and `successor` were forced into one
group, the numeric use inside `successor` would constrain `identity`
before it could be generalised. The later call at `Bool` would then
require a `Num Bool` instance. The next chapter explains [why completed
definitions can have fresh types at each use](how-types-are-inferred.md#one-definition-fresh-types-at-each-use).
Smaller groups also limit the amount of substitution state the type
checker needs at once.

## The parser's stack is shadowed

There is one memory-management detail to understand when reading the
grammar's actions. The generated parser builds larger phrases from
smaller ones on a stack. Consuming a token onto that stack is a *shift*;
replacing the fragments for a production's right-hand side with its
result is a *reduction*. An action such as `ap(function, argument)`
constructs the tree fragment for that result.

Those constructions allocate heap cells and can trigger garbage
collection. Every parsed fragment still needed on the stack must
remain reachable by the collector. Bison's stack starts in
`yyparse`'s frame but can move to the C heap as it grows, so Cupid
cannot rely on the collector finding all those references there.

Instead it keeps a matching stack of cell values among the
interpreter's roots. The lexer pushes each token's value onto it.
An action combining `n` fragments calls `gcShadow(n, e)` to replace
their values with `e`. Here is the function's body in
[`src/parser.y`](https://github.com/andreabedini/cupid/blob/main/src/parser.y),
leaving out the stack-diagram comment:

<!-- source: ../../src/parser.y -->
```c
    if (yychar>=0) {
	pushed(n-1) = top();
	pushed(n)   = e;
    }
    else
	pushed(n-1) = e;
    sp -= (n-1);
    return e;
```

For the application rule, `n` is 2 and `e` is the new pair.
`pushed(0)` is the top cell, `pushed(1)` the one below it, and `sp` is
the cell-stack pointer. With no lookahead token, the `else` branch
writes `e` over the older fragment. Moving `sp` down by `n - 1`, here
one position, leaves that result on top.

When `yychar >= 0`, the top cell instead holds a lookahead token that
Bison has read but not consumed. The first assignment saves it above
where `e` will go; the second writes `e` below it. After `sp` moves,
the stack ends with the result and the lookahead, both still reachable.
The shadow stack must follow each production's reduction, or a later
collection can free a fragment the parser still needs.

## Check the path

`tests/doc-sessions/Source.hs` contains the opening example and the
mutually recursive definitions. Before running it, predict `answer`
from the operator grouping, then check it:

<!-- session: source-reading -->
```text
$ cupid -i Source.hs
Source> answer
46
Source> evenLength [True, False]
True
Source> let identity x = x; successor y = identity y + 1 in identity True
True
```

For a scope check, predict what happens to
`let x = True in (\x -> x + 1) 3` if the inner binder is renamed to `z`
without changing its body. The original returns `4`; after that change,
`x` refers to the outer boolean and the addition fails type checking.
For a layout check, compare the multiline `let` in `twice` with
`let y = x + x in y`: why does only the second need the parser's help
to close its block?

<a id="see-also"></a>

## Where to read

| Question | Entry point | File |
|---|---|---|
| Which token comes next, and does layout insert punctuation? | `yylex`, `goOffside` | `input.c` |
| Which phrases does the grammar recognise? | `appExp`, `end`, `gcShadow` | `parser.y` |
| When can operators be grouped? | `tidyInfix` | `fixity.c` |
| Which checks run before type inference? | `checkDefns` | `static.c` |
| Which equations belong to one binding? | `eqnsToBindings` | `binding.c` |
| Which definition does a name refer to? | `depExpr`, `depVar` | `depend.c` |
| How is a written type resolved? | `checkSigType`, `depTypeExp` | `typeexp.c` |
| Which bindings must be checked together? | `sccSort` | `scc.c` |

At this point the type checker receives bindings arranged by dependency,
with names checked and infix chains resolved. It still has to find types
that agree with all uses of those bindings. [How types are
inferred](how-types-are-inferred.md) takes up that job.
