# How source is read

This page follows a script from characters to the bindings the type
checker takes, through the lexer (`src/input.c`), the grammar
(`src/parser.y`) and the static analysis that `src/static.c` drives.
The shape of this front end is Gofer's, and the reasons for its less
obvious choices, which the rest of the page gives, come from there.

## Lexing

The lexer keeps two characters of lookahead, `c0` and `c1`, which is
enough to tell every token apart, and the row and column of `c0`.
Columns drive the layout rule; rows are stored into the parsed form at
the points where a later stage might need to report an error (each
equation's right-hand side, each signature, each fixity declaration),
so that a type error can name a line without the type checker knowing
anything about files.

The layout rule is applied here, not in the parser.  The lexer keeps a
stack of the indentations of the open layout blocks and inserts the
`{`, `;` and `}` tokens that indentation implies, so the grammar only
ever sees explicit braces.  One case cannot be decided by indentation
alone: the rule says an implicit block also closes when the next token
would be a parse error, as in `let x = 1 in x`, where `in` closes the
`let`.  The lexer cannot know what is a parse error, so the grammar's
`begin` and `end` productions have an `error` alternative that, on a
parse error inside an implicit block, calls `yyerrok` and closes the
block (`canUnOffside`, `unOffside`).  Those `error` alternatives are
load-bearing: they are how the layout rule is implemented, not just
how syntax errors are reported.

## The parser's stack is shadowed

The parser builds its result in the heap, one cell at a time, and a
garbage collection can happen at any allocation.  Bison keeps the
fragments it has parsed so far on its own stack, which the collector
cannot see.  So the interpreter mirrors that stack on its own: the
lexer pushes every token's value on the cell stack as it is returned,
and every grammar action that builds a value from `n` fragments calls
`gcShadow(n, e)`, which pops the `n` and pushes `e`, keeping the held
lookahead token in place when there is one.  The two stacks stay in
step, and everything the parser is holding is a root.

The rule for anyone adding a production is that its action must call
`gcShadow` with the number of symbols on its right-hand side that
carry values, or the shadow drifts and the next collection frees a
fragment the parser still needs.

## What the grammar leaves to later

The grammar is more liberal than the language in a few places, each
because bison has one token of lookahead and the Haskell grammar needs
more.

**Contexts are parsed as types.**  Whether `(C a, C b)` is a context or
a tuple type is only known when the `=>` after it is seen.  So the
grammar reads the part before `=>` as a type and, on reaching `=>`,
checks that what it read can be a context (`checkCtxt`).  `Int => Bool`
is therefore a static error, not a syntax error.

**Function heads and generators are parsed as expressions.**  `f (x:xs)
= ...` and `p <- e` have patterns on the left that the grammar reads as
expressions, and `checkPat` in `src/depend.c` later checks that each is
a pattern and converts it.  A wildcard in an expression, or a `let` in
a pattern, is reported there.  Where the pattern is unambiguous, in a
`case` alternative or a lambda, the grammar has pattern productions of
its own.

**Operator chains are resolved after parsing.**  An expression with
infix operators is read as a flat sequence of operands and operators
(an `INFIX` chain, with prefix minus as a `NEG` node), and the
precedence and associativity are applied afterwards by a small
shift-reduce pass, `tidyInfix` in `src/fixity.c`.  Doing it in the
grammar would need a family of productions indexed by precedence, as
the Haskell report's grammar has.  The pass runs during dependency
analysis, called from `depExpr` and `checkPat`, which is after the
module's imports have been processed; Gofer's author suggested exactly
this delay as the way to make imported fixities work when a module
system was added, and Hugs did it.  Prefix minus gets the Haskell
precedence, which Gofer did not give it.

**Some syntax is rewritten on the spot.**  A right section becomes an
application of `flip`, an arithmetic sequence becomes `enumFromTo` and
its relatives, a list literal becomes a `FINLIST` cell, and a pattern
guard becomes a hidden `Maybe` binding and a boolean guard.  [The
design of Cupid](design.md#the-language) says why desugaring in the
parser is preferred.

Declarations other than equations do not wait for the end of the
script: a data, class or instance declaration is entered into its
table as it is parsed, a fixity declaration takes effect at once for
the rest of the file, and primitive bindings are collected for the
static checks.

## Static analysis

The static checks are mundane and numerous: a type constructor is not
defined twice, its parameters are distinct, the right-hand side of a
data declaration mentions only those parameters and only constructors
that exist, every equation for a function has the same number of
arguments, and so on.  They exist so that later stages can assume
well-formed input: the type checker never meets an unbound variable,
and the compiler never meets an undefined constructor.  Because
definitions in a script may come in any order, most checks run only
when the whole script has been parsed.

Alongside the checks, static analysis *translates*, so that later
stages do no lookups.  An identifier that names a constructor, a class
method, a global function or a type constructor is replaced by the
`Name`, `Class` or `Tycon` cell itself.  In a type signature the type
variables are replaced by offsets, `mkOffset(0)`, `mkOffset(1)`, ...,
in order of first occurrence, and the whole type is wrapped in a
`POLYTYPE` cell whose first component lists the kinds of those
variables; `a -> m a` becomes `POLYTYPE ([*, * -> *], OFFSET 0 -> OFFSET 1 (OFFSET 0))`.
Offsets rather than names are what make instantiating a type scheme
cheap; [How types are inferred](how-types-are-inferred.md) builds on
this.  There can be at most `NUM_OFFSETS` of them in one scheme.

**Bindings.**  The parser's output is a list of equations and
signatures.  `src/binding.c` groups them into bindings of two shapes:

- a function or variable binding, `(Var, (Attr, [Alt]))`, one per
  variable, holding the variable, its declared type and fixity if any,
  and its alternatives, each a list of patterns and a right-hand side;
- a pattern binding, `([Var], ([Attr], (Pat, Rhs)))`, one per equation
  whose left-hand side is a pattern, listing the variables it binds.

The two are told apart by `isVar(fst(b))`, a test that every later
pass makes.  The equations for one variable must be contiguous and
have the same number of arguments.  A pattern binding that binds no
variable, such as `True = False`, is accepted, as Haskell requires;
Gofer rejected it on the grounds that nothing could ever force it, and
Hugs allowed it because it is legal and occasionally useful for the
typing constraints it introduces.

**Dependency analysis.**  `src/depend.c` resolves every name and then
sorts the bindings of a script, and of each `let` and `where`, into a
list of lists: each inner list is a group of mutually recursive
bindings, and a group comes after every group it uses.  The reason is
polymorphism.  Hindley-Milner inference gives all the bindings in one
group a monomorphic type while the group is being checked, so in

```haskell
let id x   = x
    succ y = id y + 1
in id True
```

`id` would be restricted to `Int -> Int` if the two were checked
together, and is fully polymorphic when `id` is its own group.  A
second reason is practical: the type checker's substitution grows with
the size of a group, and small groups keep it small.  The sort is
Tarjan's strongly-connected-components algorithm in `src/scc.c`,
written once as `sccSort` with callbacks that say where a node keeps
its dependency list; the same function sorts bindings, and sorts type
constructors and classes together before kind inference, because a
class and a data type may refer to each other.  Gofer did this with a
C file included several times under different macro definitions and
remarked that templates or functors would have been more elegant; a
function with callbacks is the C answer.

## See also

- [How Cupid runs a program](how-cupid-runs-a-program.md) — the
  stages before and after
- [Cells, text and tables](cells-text-and-tables.md) — what the parser
  builds its result from
- [The language Cupid accepts](../reference/language.md)
