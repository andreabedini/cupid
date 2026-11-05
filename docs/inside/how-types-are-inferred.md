# How types are inferred

In the [tour of the pipeline](how-cupid-runs-a-program.md), the type
checker found the types of the bindings and inserted dictionary arguments
for their overloaded names. This chapter follows those two jobs into
`src/type.c` and `src/subst.c`. It assumes you can read Haskell types,
but not that you already know how an interpreter infers them.

Start with a function that needs no type classes:

<!-- source: Inference.hs -->
```haskell
apply f x = f x
```

Without running it, Cupid finds `apply :: (a -> b) -> a -> b`. What tells
the interpreter that `f` is a function, or that its result can have a
different type from `x`? The body supplies that information. Before
looking at how Cupid stores types, we can work out what it has to learn.

## What a use tells us

At the start of checking `apply`, the types of its arguments are unknown.
Give each a *fresh type variable*: a placeholder that has not been used
for any other unknown. Call the type of `f` `a`, the type of `x` `b`, and
the type of the body `c`. These letters stand for types, not Haskell
values. The checker keeps a *type environment*, a record of the names in
scope and their types; Cupid's code calls these records *assumptions*.

The application `f x` requires `f` to accept an argument of the type of
`x` and return the type of the body. That gives one equation:

```text
a = b -> c
```

The whole function takes `f`, then `x`, and returns the body's result,
so its type starts as `a -> b -> c`. Replacing `a` by what we learned
gives `(b -> c) -> b -> c`. Renaming the remaining variables gives the
familiar `(a -> b) -> a -> b`: the choice of letters does not change a
type.

Solving equations between types is called *unification*. Its result is
a *substitution*, a record of which types replace which unknowns. In
this example the substitution records that the first unknown is a
function from the second to the third. It leaves the latter two
unknown: nothing in the body requires a particular argument or result
type, or requires them to be equal.

Other expressions add other requirements. In `if p then x else y`, `p`
must have type `Bool`, and `x` and `y` must have the same type. In
`[x, y]`, both elements must have the same type. For a name already in
the environment, the checker starts from its recorded type. Each part
of an expression contributes information which must agree with the
information from the rest.

Unification follows the structure of a type. To make `a -> Bool` agree
with `Char -> b`, it must make their argument types agree and their
result types agree: `a` becomes `Char`, and `b` becomes `Bool`. For two
list types it compares the element types. An unknown can be bound to a
type, but two distinct concrete types such as `Bool` and `Char` cannot
be made equal.

This is all work on the program's syntax. No call of `apply` has to run,
and no sample values are needed.

## When types cannot agree

The expression `if True then False else 'x'` is rejected because its
branches require `Bool` and `Char` to be the same type. Another failure
is less obvious:

```haskell
\x -> x x
```

Both occurrences refer to the same argument, so they have the same
unknown type, say `a`. But the application requires the first occurrence
to be a function taking the second. If its result has type `b`, the
equation is `a = a -> b`. Substituting the right-hand side for `a` only
makes the type larger; there is no finite solution. The *occurs check*
rejects a binding when the variable being bound occurs inside the type
it would be bound to. Cupid reports that unification would give an
infinite type.

In `src/subst.c`, `unify` follows existing substitutions before comparing
the types. `varToVarBind` links two unknowns; `varToTypeBind` checks and
records a binding to a non-variable type, including the occurs check.
The representation of that record comes shortly. First there is another
distinction to make: which unknowns belong to a particular use of a
function?

## One definition, fresh types at each use

Consider a completed definition:

<!-- source: Inference.hs -->
```haskell
identity x = x
```

Both occurrences of `x` have the same type, so the inferred type is
`a -> a`. Nothing outside this definition fixes `a`. The checker can
therefore *generalise* it: record that `identity` works for every choice
of `a`. Such a type is a *type scheme*, often written `forall a. a -> a`.
Cupid's usual type display leaves the `forall` implicit.

Each use of that scheme gets its own fresh unknown. This is
*instantiation*. In `(identity True, identity 'x')`, the first use starts
with `b -> b` and learns `b = Bool`; the second starts with `c -> c` and
learns `c = Char`. Within either use, the two occurrences of its variable
must still agree. Between the uses, there is no such requirement. The
result has type `(Bool, Char)`.

Generalisation does not turn every remaining unknown into an independent
choice. An unknown may still describe something in the surrounding
environment:

```haskell
capture f = let g y = f y
            in (g True, g 'x')
```

When the checker reaches `g`, `f` is an argument of `capture` whose type
is still being inferred. The body of `g` makes that type `a -> b`, with
`y` of type `a` and the result of type `b`. Both variables are part of
`f`'s type in the enclosing environment. Generalising them for `g` would
forget that `g` calls this particular `f`.

They must stay shared. The two calls of `g` consequently require the
argument type of `f` to be both `Bool` and `Char`, and the definition is
rejected. If the body were just `g True`, `capture` would have type
`(Bool -> a) -> a`.

For these examples, the rule is: solve the equations, then generalise
the unknowns in the inferred type that are not fixed by the surrounding
environment. Haskell's overloading and its monomorphism restriction add
conditions, which we will reach with `total`. This core of inference,
generalisation and instantiation is Hindley–Milner inference.

<a id="expressions-and-types-are-trees"></a>

## From equations to the checker

The parser represents [expressions and types as trees of
`Cell`s](how-source-is-read.md#expressions-and-types-are-trees).
The checker walks the expression tree and unifies the types it finds.

We wrote equations separately to expose the reasoning. Cupid unifies as
it visits expressions, updating a substitution shared by the current
inference. It does not first collect all these equations into a separate
solver input. Class predicates such as `Num a` are different: they do
accumulate for the constraint solver.

`typeExpr` dispatches on the expression cell. Applications go to
`typeAp`, which instantiates the head's type and checks it against the
function type required by the arguments. To read another case, the
conditional, we first need the representation of those types.

## Types are a skeleton and an offset

In `identity`'s type, `a -> a`, both occurrences of `a` must refer to
the same unknown within one use, while each use needs its own unknown.
Cupid keeps one type tree and gives each use a separate set of unknowns.

A stored type scheme is a `POLYTYPE` cell containing a list of kinds and
a type whose quantified variables are numbered offsets. For `identity`
it is, schematically:

```text
POLYTYPE ([*], OFFSET 0 -> OFFSET 0)
```

The one `*` says that the scheme has one variable standing for an
ordinary type; kinds are discussed below. `OFFSET 0` identifies that
variable within the scheme. It does not identify a particular use's
unknown in the substitution.

During inference a type is a pair of a *skeleton* and a base *offset*.
The skeleton is the stored type expression, uncopied. If the base is
`o`, `OFFSET i` in the skeleton refers to substitution entry `o + i`.
To instantiate a scheme, `instantiate` allocates fresh consecutive
entries for its quantified variables and returns the same skeleton
with the base of those entries. Thus the two uses of `identity` can
share `OFFSET 0 -> OFFSET 0` without sharing an unknown.

For illustration, suppose their bases were 20 and 21:

| use | skeleton | base | entry after unification |
|---|---|---|---|
| `identity True` | `OFFSET 0 -> OFFSET 0` | 20 | 20 is bound to `Bool` |
| `identity 'x'` | `OFFSET 0 -> OFFSET 0` | 21 | 21 is bound to `Char` |

These are illustrative entry numbers, not a trace of the interpreter.
The important distinction is that the skeleton is shared and the fresh
entries are not. This technique is called *structure sharing*, after
the technique of the same name in logic programming.

The substitution is an array of `Tyvar` records. An entry's `bound`
field is `NIL` for an unknown, or a skeleton read at the base in its
`offs` field when a binding has been found. Entries can link to other
entries, so reading a type follows those links (`deRef`). Each entry
also has a kind. A special `SKOLEM` binding makes a variable rigid,
meaning unification may not choose a type for it; signature and scope
checks need this distinction.

`tyvar(n)` accesses the `n`th entry, and the array grows in chunks of
`NUM_TYVARS` as inference needs more variables (`expandSubst`). Early
versions of Gofer had a fixed size; Gofer itself made it grow by the
time of its report, for the very large binding groups that
machine-generated parsers produce
([RR-1030](http://web.cecs.pdx.edu/~mpj/pubs/RR-1030.pdf), p. 25).

The result of typing an expression travels as a `Typing` record: its
skeleton and base offset, the number of variables freshly allocated
when a scheme was instantiated, and any predicates peeled off the
scheme. Gofer and Hugs used global registers (`typeIs`, `typeOff`);
Cupid's record makes the result visible in a function's signature.

### Checking a conditional

Return to `if p then x else y`: the condition must be `Bool`, and the
two branches must share a result type. Here is the complete `COND`
case from [`typeExpr` in `src/type.c`](https://github.com/andreabedini/cupid/blob/main/src/type.c):

<!-- source: ../../src/type.c -->
```c
    case COND: {
        int beta = newTyvars(1);
        fst3(snd(e)) = check(l, fst3(snd(e)), e, cond, typeBool, 0);
        snd3(snd(e)) = check(l, snd3(snd(e)), e, cond, aVar, beta);
        thd3(snd(e)) = check(l, thd3(snd(e)), e, cond, aVar, beta);
        *ty = tyvarType(beta);
    } break;
```

`ty` points to the `Typing` result. `e` is the conditional expression,
whose `snd` holds a triple: the condition, then-branch and else-branch.
`fst3`, `snd3` and `thd3` access
those three cells. `newTyvars(1)` allocates one fresh substitution entry
and returns its index as `beta`. The skeleton `aVar` is `OFFSET 0`, so
both checks using `(aVar, beta)` refer to that same unknown. A branch
that fixes it to `Bool` makes the other branch agree with `Bool` too.

Each `check` call infers its child's type and unifies it with the
required skeleton and base given in the last two arguments. The other
arguments identify the source line `l`, the child, the whole expression
`e` and the diagnostic label `cond`. The assignments store the translated
children back into the triple, preserving any dictionary arguments
inserted while checking them. Finally, `tyvarType(beta)` follows the
substitution to produce the conditional's type in the caller's `*ty`.

### Assumptions

The environment must distinguish a definition still being inferred
from a completed scheme that may be instantiated. `defnBounds` holds
the bindings of the group being checked; `varsBounds` holds lambda-bound
variables, bindings with signatures and bindings whose group has
finished. Each is a list of lists, one per nested binding scope.
`enterBindings` pushes a level and `leaveBindings` pops it. A lambda
argument in `varsBounds` still has one shared unknown type; belonging
to that list does not make it polymorphic.

`findAssum` looks up a local name. A global binding stays in these
lists until its top-level group is done, when `typeDefnGroup` stores
its type in its name record. The narrower scopes needed by existential
types and local givens have a separate stack, `TcScope`, described in
[How constraints are solved](how-constraints-are-solved.md#scopes-skolems-and-local-givens).

## Generalisation

The implementation of generalisation has to preserve the distinction
between `identity` and the captured `f`. Cupid records it with marks
on the substitution entries themselves. For an unbound variable,
`offs` is available for this bookkeeping because there is no bound
skeleton whose base it needs to store.

`clearMarks` initially makes unbound variables `UNUSED_GENERIC`.
`markType`, called on types in the surrounding assumptions, follows
the substitution and marks their unknowns `FIXED_TYVAR`. Those are
the unknowns that must stay shared with the environment. When
`copyType` copies the inferred type into a stored scheme, it assigns
each remaining unbound variable the next numbered offset and records
`GENERIC + n` in its mark. A second occurrence gets the same offset.
`generalize` wraps the result in `POLYTYPE`, with `QUAL` for any
remaining class predicates.

For `identity`, both occurrences of its unknown become `OFFSET 0`.
In the local `g` above, the variables tied to `f` are marked fixed
instead. Notice where the copying happens: generalisation constructs
a stored scheme; instantiation reuses its skeleton with fresh
substitution entries. The two operations are not inverses that copy
the same tree back and forth.

## Binding groups

Haskell definitions can refer to themselves and to each other. For
an implicitly typed recursive group, the checker first assigns each
binding a provisional unknown type. Uses within the group share that
type, so a recursive call contributes equations to the definition
being inferred. The group is generalised only after its definitions
have been checked. Once completed, its schemes can be instantiated by
later groups. This is why [dependency analysis](how-source-is-read.md#static-analysis)
separates independent bindings before inference.

`typeBindings` receives one group and chooses between two treatments:

- If the group contains a pattern binding, or a binding with no
  arguments and no signature, `monorestrict` applies the monomorphism
  restriction. Constrained variables are not generalised, so this
  route adds no dictionary parameters; unconstrained variables can
  still be generalised.
- Otherwise `unrestricted` handles it. Bindings with signatures are
  made available at their declared schemes, and the implicitly typed
  bindings are sorted again by their dependencies on each other
  (`itbscc`). A signature can therefore break a recursive cycle. It
  also permits polymorphic recursion: recursive calls may instantiate
  the declared scheme instead of sharing a provisional monotype.

The class predicates collected during inference affect what can be
generalised, deferred to an enclosing group or defaulted to a concrete
type. [The loop at a binding group](how-constraints-are-solved.md#the-loop-at-a-binding-group)
describes those decisions.

## The expression is rewritten in place

Now return to the program from the pipeline tour. Leave out `total`'s
signature so that its type must be inferred:

<!-- source: Inference.hs -->
```haskell
total []     = 0
total (x:xs) = x + total xs
```

The patterns require a list argument. In the second equation, `x` has
the list's element type and `xs` has the same list type as the
argument. The recursive call shares `total`'s provisional type.
Addition requires its two operands and result to have one type and
asks for a `Num` instance for that type. The zero in the first
equation asks for `Num` too: Haskell integer literals are overloaded.
Both equations must return the same type. The result is
`total :: Num a => [a] -> a`.

The equations between types and the class predicate `Num a` have
different jobs. Unification establishes which types are equal. The
predicate asks where the operations for that type come from. Here
`a` remains general, so a caller must supply those operations as a
dictionary. Conceptually, the translated equations are:

```text
total d []     = fromInt d 0
total d (x:xs) = (+) d x (total d xs)
```

This is a schematic view of Cupid's internal translation; `fromInt`
is its small-integer conversion. The dictionary is an ordinary extra
argument by the time compilation begins.

Cupid constructs that translation during inference. `typeExpr`
returns the translated expression, and callers store it back into the
parent node. When `typeAp` instantiates an overloaded name, it calls
`assumeEvid` for each class predicate in its context. That allocates
a dictionary placeholder, a `DICTVAR` cell, and the expression gets
an application to the placeholder immediately. When the solver finds
the evidence, `overEvid` overwrites that cell with an application of
`nameInd` (`_indirect`) to it. Every use sees the change through the
shared cell; no later traversal has to find those uses.

The recursive reference needs the same treatment from the other
direction. The assumptions for the current group share one cell for
each referenced binding per scope (`findAssum` returns it again on
later references). If that binding needs dictionary parameters,
`addEvidParams` overwrites the cell with its application to those
parameters, updating the recursive uses together. Gofer's author
noted that a second traversal of nested bodies could be very
expensive (RR-1030, p. 32). This is why assumptions for the current
group stay separate from those already settled.

With the explicit `Num a => [a] -> a` signature restored, the
dictionary is instead available from the declared context while
the body is checked. Its placeholders are discharged using that
given dictionary, as the pipeline tour showed. The compiler receives
ordinary function applications in either case.

## Kinds use the same machinery

Types must themselves be used consistently: `Int` is a type, while
`[]` takes a type to make a type, as in `[Int]`. Their *kinds* record
that distinction. In Haskell notation they have kinds `Type` and
`Type -> Type`; Cupid's C calls the former `STAR`. A class such as
`Eq` takes a type to make a constraint, so its kind is
`Type -> Constraint`.

Kind inference in `src/kind.c` uses `STAR`, `CONSTRAINT` and pairs
`(k1, k2)` for kind arrows. Its unknowns are offsets in the same
substitution array, and `newKindvars` is `newTyvars`. There is no
polymorphism or dictionary passing in this inference. Calls to
`emptySubstitution` separate its use of the array from type inference.
With unification in mind, `kind.c` is a smaller place to study it
before returning to the type checker.

## Check the types

`tests/doc-sessions/Inference.hs` contains the successful definitions
from this chapter, including `captured`, whose body only uses
`g True`. These commands check their types and the two independent
uses of `identity`:

<!-- session: inference-types -->
```
$ cupid -i Inference.hs
Inference> :type apply
apply :: (a -> b) -> a -> b
Inference> :type identity
identity :: a -> a
Inference> (identity True, identity 'x')
(True,'x')
Inference> :type captured
captured :: (Bool -> a) -> a
Inference> :type total
total :: Num a => [a] -> a
```

Before trying it, predict what happens if the two calls of `identity`
become a list, `[identity True, identity 'x']`. The calls still have
fresh types, but the list adds a requirement that the pair did not.
Also compare `let identity x = x in (identity True, identity 'x')`
with `\identity -> (identity True, identity 'x')`: which occurrence
introduces a scheme, and which introduces a single shared unknown?

## Where to read

| operation | function | file |
|---|---|---|
| infer an expression and enforce a required type | `typeExpr`, `typeAp`, `check`, `shouldBe` | type.c |
| instantiate a scheme | `instantiate` | subst.c |
| allocate and unify unknowns | `newTyvars`, `deRef`, `unify`, `varToTypeBind` | subst.c |
| look up names in the environment | `enterBindings`, `findAssum`, `addEvidParams` | type.c |
| generalise variables not fixed by that environment | `clearMarks`, `markType`, `copyType`, `generalize` | subst.c, type.c |
| check a recursive group | `typeBindings`, `monorestrict`, `unrestricted` | type.c |
| insert dictionary placeholders and supply evidence | `assumeEvid`, `overEvid` | preds.c |
| infer kinds | `kindTCGroup`, `kindInst` | kind.c |

The examples above settle type equations. A class predicate may
instead require an instance, a superclass dictionary, or a parameter
supplied by the caller. [How constraints are solved](how-constraints-are-solved.md)
follows those questions through the solver.

<a id="see-also"></a>
For the core theory, see Norman Ramsey, *Programming Languages:
Build, Prove, and Compare*, chapter 7, especially §7.5 on inference,
and Benjamin C. Pierce, *Types and Programming Languages*, chapter
22, especially §22.7 on let-polymorphism. These explain the core
without Haskell's type classes. Mark P. Jones's *Qualified Types:
Theory and Practice* develops the theory of overloading;
[RR-1030](http://web.cecs.pdx.edu/~mpj/pubs/RR-1030.pdf), §7,
describes Gofer's implementation on which Cupid's is based.
