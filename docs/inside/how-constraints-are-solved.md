# How constraints are solved

[Type inference](how-types-are-inferred.md) establishes which types must
agree. A class constraint asks a further question: where do the operations
for that type come from? This definition needs equality on its argument
type:

<!-- source: Constraints.hs -->
```haskell
same :: Eq a => a -> a -> Bool
same x y = x == y
```

Unification tells Cupid that `x` and `y` have the same type. It does not
supply an equality operation for every possible type. The `Eq a`
constraint says what a caller must supply. This chapter follows that
obligation from the Haskell definition to the dictionary expression
Cupid builds for it, then looks at how the solver stores and processes
obligations in `src/preds.c`, `src/subst.c` and `src/type.c`.

## Predicates and evidence

A *predicate* is a claim about types, such as `Eq a` or `Ord Int`.
For a class predicate, the implementation needs a *dictionary*: a value
holding the operations and superclass dictionaries for that class at
those types. Dictionary passing turns `same` into a function with an
extra argument, schematically:

```text
same dEq x y = (==) dEq x y
```

The extra argument supplies equality on the particular argument type.
It is ordinary data by the time compilation begins. The solver builds
an expression that will supply it; it does not search for an instance
by inspecting `x` and `y` at run time.

Inside `same`, using `(==)` creates a **wanted** `Eq a`: an obligation
to provide that dictionary. The written signature supplies a **given**
`Eq a`: a dictionary the body may assume its caller will pass. These
match, so the wanted can use the given dictionary directly. A *context*
is the constraints before `=>`; when checking a definition against its
signature, that context is available as givens.

Without the signature, the body still creates the same wanted:

<!-- source: Constraints.hs -->
```haskell
sameInferred x y = x == y
```

Nothing in the body supplies equality for the still-general argument
type. Cupid can retain `Eq a` in the inferred type and add a dictionary
parameter. Thus `sameInferred` has the same type as `same`. Failing to
find a dictionary immediately is not itself an error: the surrounding
scope or a caller may be responsible for it.

**Evidence** is the expression used to supply a wanted dictionary. It
might be a parameter already in scope, a superclass selected from it,
or an instance builder applied to other dictionaries. Those three
possibilities explain most of the solver.

## Entailment

To say that givens *entail* a wanted means that the wanted follows from
them and the instance declarations. The solver must construct the
evidence as well as establish that it exists.

### A concrete instance

Here the argument type is fixed:

<!-- source: Constraints.hs -->
```haskell
sameInt :: Int -> Int -> Bool
sameInt x y = same x y
```

Instantiating `same` creates a wanted `Eq Int`. The library has an
instance for it, so no dictionary parameter is needed on `sameInt`:

```text
sameInt x y = same dEqInt x y
```

`dEqInt` is our schematic name for the instance's dictionary.

### A superclass supplies the dictionary

Cupid's `Ord` class has `Eq` as a superclass. A dictionary for `Ord a`
therefore includes access to equality on `a`:

<!-- source: Constraints.hs -->
```haskell
sameOrdered :: Ord a => a -> a -> Bool
sameOrdered x y = same x y
```

The signature gives `Ord a`, while the call wants `Eq a`. Selecting the
superclass dictionary supplies it:

```text
sameOrdered dOrd x y = same (eqFromOrd dOrd) x y
```

`eqFromOrd` names the superclass selector in this schematic translation.
No choice of a concrete type for `a` is required.

### An instance creates smaller obligations

List equality depends on equality of the elements. The library's
instance has the form `Eq a => Eq [a]`:

<!-- source: Constraints.hs -->
```haskell
sameList :: Eq a => [a] -> [a] -> Bool
sameList xs ys = same xs ys
```

The call wants `Eq [a]`; the signature gives only `Eq a`. The list
instance turns the wanted into an obligation for its element type,
which the given discharges:

```text
sameList dEq xs ys = same (eqList dEq) xs ys
```

`eqList` stands for the instance's dictionary builder. Reducing a
constraint therefore changes the evidence too: solving `Eq [a]` by
solving `Eq a` means applying that builder to the element dictionary.
All the translations here are schematic, with invented names for
dictionaries, selectors and builders.

### Following those steps in the C

`entail` first calls `scEntail` to look for a matching given or a
superclass of one. `scFind` follows superclass selectors, building
applications such as `eqFromOrd dOrd`. The classes' `level` fields
bound the search through the hierarchy.

If that fails, `inEntail` asks `findInstFor` for an instance whose
*head* matches the wanted: in `Eq a => Eq [a]`, the head is `Eq [a]`.
It recursively entails the instance's context and applies the builder
to the resulting dictionaries, as in `eqList dEq`.

Instance selection uses **one-way matching**: it may choose types for
the instance's variables, but may not arbitrarily specialise the
wanted. For a wanted `Eq [Bool]`, the list instance's variable can be
`Bool`. For a wanted `Eq a`, the existence of `Eq Int` does not justify
choosing `a = Int`. Otherwise an apparently general function could
silently become an integer-only function.

The search can recurse through instance contexts, and those contexts
need not become smaller in Cupid's language. The `cutoff` depth,
controlled by `-c`, bounds the search. `Typeable` has a further source of
evidence: if no instance matches, `typeableEvid` can construct its
dictionary from the structure of the type.

With overlapping instances enabled (`+o`), a potential more specific
match can block a current match until the types are known; `+O` skips
that check. Context reduction could otherwise commit to the wrong
instance, so with either toggle `elimTauts` removes only predicates
that entailment proves outright. The accepted declarations and toggle
semantics belong to the [language reference](../reference/extensions.md).

## How obligations are stored

A class predicate is represented as a class cell applied to type cells.
During inference it is read with a base offset, just like a type
skeleton. The list `preds` stores triples `(pi, o, ev)`: a predicate
skeleton, its offset, and a dictionary placeholder already inserted in
the expression that needs it.

`typeAp` instantiates an overloaded name and calls `assumeEvid` for
each constraint in its context. `assumeEvid` makes that triple and
returns the placeholder for the new dictionary argument. Literals and
`do` notation create obligations for their operations in the same way.
Contexts assumed while checking signatures or instances go through
`assumeGivens`; class contexts can be represented by `makePredAss`.

Once evidence is found, `overEvid` overwrites the placeholder with an
application of `_indirect` to that evidence. Every use sharing the
cell sees the result. If an inferred function retains a constraint,
`qualifyBinding` instead adds its dictionary variable to the
function's parameters. [How types are inferred](how-types-are-inferred.md#the-expression-is-rewritten-in-place)
shows how these placeholders enter the expression tree.

## Improvement

Some constraints also tell the checker which types must agree. A
functional dependency declares that some parameters of a class determine
others. In this small class, `c -> e` says that a container type fixes
its element type:

<!-- source: Constraints.hs -->
```haskell
class Element c e | c -> e where
  first :: c -> e

instance Element [a] a where
  first = head
```

Two predicates `Element c a` and `Element c b` must therefore agree on
`a` and `b`. Against the instance, `Element [Bool] e` tells us that
`e` must be `Bool`. This is called *improvement*: the constraints make
the types more specific before the solver decides how to supply their
dictionaries.

`improve` compares predicates with each other (`pairImprove`) and
with instance declarations (`instImprove`), unifying the positions
that the dependencies determine. It repeats until nothing changes.
`improve1` does the corresponding work for an individual obligation
before entailment. This matters when one dictionary builder's context
contains a type variable that only a dependency can determine.

## The loop at a binding group

The solver must decide which obligations a binding can settle and
which belong elsewhere. The important outcomes are:

- **Solved:** use a given, a superclass or an instance, as above.
- **Deferred:** return an obligation about an enclosing scope's unknowns
  to that scope. A local binding cannot independently choose the type
  of an argument captured from its enclosing function.
- **Retained:** put a constraint on an inferred function's type and add
  a dictionary parameter, as for `sameInferred`.
- **Defaulted:** choose a permitted concrete numeric type where the
  defaulting rules allow it.
- **Rejected:** report a constraint that cannot be supplied where it
  is required. A declared signature cannot acquire extra constraints
  that its context did not promise.

For example, `same :: a -> a -> Bool` would promise equality for every
`a` without asking the caller for operations. Its body still uses
`(==)`, so that signature is insufficient. Conversely, a function
returning `show (1 + 2)` leaves a numeric type that its result,
`String`, does not identify; defaulting can resolve that type rather
than add an ambiguous dictionary parameter.

`unrestricted` in `src/type.c` manages this for function bindings.
Each implicitly typed subgroup starts with an empty `preds`, while
`savePreds` holds obligations from outside it. After inferring the
bodies, the checker improves the constraints and marks unknowns fixed
by the enclosing environment. `elimOuterPreds` reduces instance
obligations, removes redundant superclass constraints and defers
predicates with no variables available for generalisation here.

`resolveDefs` tries defaulting for variables the inferred binding types
do not mention, then reduction and deferral run again if it changed a
type. Remaining predicates enter the generalised context, and
`qualifyBinding` adds dictionary parameters. Explicitly typed bindings
are checked using their signatures' givens: `elimPredsUsing` supplies
what follows from those givens and the instances, and defers enclosing
obligations. The declared and inferred schemes must agree
(`sameSchemes`), and an unsolved local obligation is an error
(`cantEstablish`).

`monorestrict` handles groups subject to the monomorphism restriction.
It improves and reduces, passes remaining predicates outward, and does
not generalise their constrained variables. At the top level it must
resolve them, using defaulting where allowed. Module-level checking
reports any obligations still unresolved when there is no outer group
to receive them.

Ordinary numeric defaulting requires constraints of the form `C a`,
where `C` is a standard class and at least one constraint is numeric.
It chooses the first type in the `default` list that satisfies them all. It is distinct from
instance matching's refusal to guess a wanted type. The prompt uses
interactive defaulting, which also lets `Show`, `Eq` or `Ord` alone
trigger a default. The [language reference](../reference/haskell-2010.md#4-declarations-and-bindings)
and [Haskell report](https://www.haskell.org/onlinereport/haskell2010/haskellch4.html#x10-790004.3.4)
describe the language rules.

## Equality predicates

A predicate can instead assert that two types are equal, `a ~ b`:

<!-- source: Constraints.hs -->
```haskell
sameType :: (a ~ b) => a -> b
sameType x = x
```

While checking this body, the given equality makes the argument and
result types agree. At a use, the wanted equality must agree too.
Cupid implements both by unification. There is no dictionary argument
for an equality, and it never enters the `preds` list.

`assumeGivens` unifies a signature's or instance's given equalities
before the body is checked. `assumeEvid` solves wanted equalities as
they arise; `applyToEvid` and `inEntail` do the same for an instance's
context. The declared type is compared with the inferred type through
the resulting substitution. In Cupid, which has no type families, an
equality whose sides cannot unify is rejected immediately.

## Scopes, skolems and local givens

Some evidence is available only inside a pattern match. A packed
value can carry a hidden type and operations on it:

<!-- source: Constraints.hs -->
```haskell
data Packed = forall a. Show a => Packed a

render :: Packed -> String
render (Packed x) = show x
```

Constructing `Packed` chooses a type and stores its `Show` dictionary
with the value. Matching it brings both into scope, so `render` can
use `show` without knowing the hidden type. It cannot choose that type
or return `x` as if the type were known outside the match.

The hidden type is represented by a *skolem*: a fresh rigid type
constant that unification cannot bind to a chosen type. Rejecting its
escape prevents a definition such as `unpack (Packed x) = x` from
pretending to expose a single, freely usable result type.

`enterScope` and `leaveScope` in `src/type.c` maintain scopes for
lambdas, alternatives, generators, `let`s and top-level groups. Each
scope holds its skolems, its givens and the enclosing predicate list.
On exit the checker uses local givens to discharge local obligations,
passes the rest outward, and checks that skolems have not escaped into
the result type or assumptions. The lifetime of the evidence is part
of the typing rule, not just housekeeping for the list.

## Check the evidence

`tests/doc-sessions/Constraints.hs` contains these definitions. The
session checks both the inferred constraint and calls whose dictionaries
come from different sources:

<!-- session: constraint-evidence -->
```text
$ cupid -i Constraints.hs
Constraints> :type sameInferred
sameInferred :: Eq a => a -> a -> Bool
Constraints> sameInt 3 3
True
Constraints> sameOrdered True False
False
Constraints> sameList [True, False] [True, False]
True
Constraints> :type first [True, False]
first [True,False] :: Bool
Constraints> render (Packed True)
"True"
Constraints> sameType True
True
```

Before trying another example, predict the remaining obligation for
`sameList [] []`. The list instance reduces it to equality on an unknown
element type; the call has not identified that type merely by supplying
empty lists. Compare giving those lists the type `[Bool]` with relying
on the prompt's interactive defaulting.

## Where to read

| Step | Function | File |
|---|---|---|
| Create a wanted, or solve a wanted equality | `assumeEvid` | `preds.c` |
| Take a context as given | `assumeGivens`, `makePredAss` | `preds.c` |
| Find a given or its superclass | `scEntail`, `scFind` | `preds.c` |
| Build evidence from instances | `inEntail`, `findInstFor` | `preds.c`, `subst.c` |
| Improve types using dependencies | `improve`, `pairImprove` | `subst.c` |
| Reduce, simplify or defer obligations | `elimTauts`, `elimOuterPreds`, `elimPredsUsing` | `preds.c` |
| Resolve defaults | `resolveDefs` | `preds.c` |
| Check binding groups | `unrestricted`, `monorestrict` | `type.c` |
| Check local evidence and skolem escape | `enterScope`, `leaveScope` | `type.c` |

<a id="see-also"></a>

The type checker has now supplied dictionary arguments or made them
parameters. [How code is compiled and run](how-code-is-compiled-and-run.md)
starts from those ordinary applications. For the theory behind the
solver, see Mark P. Jones, *Qualified Types: Theory and Practice*, and
[Type Classes with Functional Dependencies](https://web.cecs.pdx.edu/~mpj/pubs/fundeps.html).
Cupid keeps [one solver](language-design.md#how-a-feature-gets-in) across
these features; the extra cases must work with the same predicates,
substitution and evidence expressions.
