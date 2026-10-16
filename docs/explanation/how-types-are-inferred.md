# How types are inferred

The type checker in `src/type.c` has two jobs: to find a type for
every binding and expression, checking it against any signature, and
to rewrite the program so that overloaded names receive the
dictionaries they need.  "Type checker" is the traditional name;
checking is the smaller part.  This page is about the machinery the
first job uses and the way the second one is threaded through it.
[How constraints are solved](how-constraints-are-solved.md) takes over
where the class constraints are decided.  The representation described
here is Gofer's, and the reasons for it are the ones its author gave.

## Types are a skeleton and an offset

A polymorphic type is stored as a `POLYTYPE` cell: a list of kinds and
a type in which the variables are offsets, `mkOffset(0)`,
`mkOffset(1)`, ..., as [How source is read](how-source-is-read.md)
describes.  The identity function's type is
`POLYTYPE ([*], OFFSET 0 -> OFFSET 0)`.

Using such a type means instantiating it with fresh type variables.
The obvious way, copying the type with each offset replaced by a new
variable, is never done.  Instead a type during inference is a pair of
a *skeleton* and an *offset* into the current substitution: the
skeleton is the stored type itself, uncopied, and `mkOffset(i)` in it
means "the type variable at position `o + i`".  To instantiate a
scheme with `n` variables, `instantiate` allocates `n` consecutive
entries in the substitution and returns the same skeleton with the
offset of the first.  Every use of `id` shares one skeleton.  Gofer's
author called this structure sharing, after the technique of the same
name in logic programming.

The current substitution is an array of `Tyvar` records, each holding
what the variable is bound to (as a skeleton and offset of its own, or
`NIL` when unbound, or `SKOLEM` for a variable that must not be bound),
a `kind`, and a field `offs` whose other use is described below.
`tyvar(n)` is the `n`th entry.  Unification binds entries; it never
copies.  The array grows in chunks of `NUM_TYVARS` as a binding group
needs more variables (`expandSubst`); Gofer's was fixed and overflowed
on the large binding groups that generated parsers produce, which is
why it was made to grow.

The result of typing an expression travels as a `Typing` record: the
skeleton, the offset, the number of fresh variables that were created
for it, and any context peeled off a scheme.  Gofer and Hugs left
these in global registers (`typeIs`, `typeOff`); the record was
introduced so that a function's signature says what it produces.

## The expression is rewritten in place

`typeExpr` returns the translated expression, and every caller stores
the result back into the parent node, so a `let`, an application or a
conditional is updated destructively as its parts are typed.  The
translation is where dictionaries go.  When an overloaded name is
instantiated, `assumeEvid` invents a *dictionary variable* (a
`DICTVAR` cell) for each predicate in the name's context, records the
predicate, and returns the variable to be applied as an extra
argument, so `x == y` becomes `(==) dv x y` on the spot.  When the
solver later finds what `dv` is, `overEvid` overwrites that very cell
with an indirection to the evidence.  Nothing walks the expression a
second time to fill the dictionaries in, and if the evidence is itself
overwritten later, every reference sees it through the indirection.

The same idea handles the other direction: a binding that turns out to
be overloaded must take dictionary *parameters*, and every reference
to it inside the group must pass them.  The assumptions for let-bound
variables are arranged so that there is one cell per variable per
scope (`findAssum` allocates it on first reference and returns the same
cell afterwards), and every occurrence in the group's bodies is that
cell.  When the group is generalised, `addEvidParams` overwrites the
one cell with `v dv1 ... dvk`, and every occurrence is updated at once.
Gofer's author noted that the alternative, a second traversal of the
bodies, would be quadratic with nested groups.  It is the reason the
assumptions distinguish let-bound from lambda-bound variables at all.

## Assumptions

The types of variables in scope are two lists of lists: `defnBounds`
for let-bound variables, which may be polymorphic and overloaded, and
`varsBounds` for lambda-bound ones, which are monomorphic and carry no
dictionaries.  `enterBindings` pushes an empty level on each when a
scope is entered and `leaveBindings` pops it.  Only locally bound
variables are here; a global has its type in its name record, and
needs no assumption.  The scopes that existential types and local
givens require are a separate stack (`TcScope`), described in [How
constraints are solved](how-constraints-are-solved.md#scopes-skolems-and-local-givens).

## Generalisation

Going the other way, from a monotype to the most general scheme, is
done with marks.  Each unbound `Tyvar` has its `offs` field set to one
of three values: `FIXED_TYVAR`, meaning the variable occurs free in
the assumptions and may not be generalised; `UNUSED_GENERIC`, meaning
it may be and has not yet been given an offset; or `GENERIC + n`,
meaning it has been given offset `n`.  `clearMarks` sets every
variable to `UNUSED_GENERIC`, `markType` walks the assumptions
marking what they mention `FIXED_TYVAR`, and `copyType` then copies
the type to be generalised, turning each unmarked variable it meets
into the next offset and recording the choice in `offs` so that later
occurrences get the same one.  `generalize` wraps the copy in `QUAL`
with the surviving predicates and `POLYTYPE` with the kinds.  The marks
live in the substitution itself, so `clearMarks` must be called before
any `copyType`, which is why it appears so often.  A scheme with more
than `NUM_OFFSETS` variables is an error.

## Binding groups

A binding group arrives from dependency analysis as a set of mutually
recursive bindings, and `typeBindings` picks one of two treatments:

- If the group has a pattern binding, or a variable binding without a
  signature, the monomorphism restriction applies: `monorestrict`
  types the group and does not generalise over any constrained
  variable, so no dictionary parameters are added.
- Otherwise `unrestricted` handles it.  The bindings with signatures
  are typed against their signatures, and the implicitly typed ones
  are sorted again by dependency among themselves (`itbscc`), so that a
  signature breaks a recursive cycle and the bindings on either side
  of it can be more polymorphic than one monomorphic group would
  allow.  A signature is also what permits polymorphic recursion: a
  recursive call to a binding with a signature uses the declared
  scheme, not the monomorphic placeholder.

In either case the group's types are found by allocating a fresh
variable for each binding, typing the right-hand sides with the
predicates accumulating, and then running the solver and generalising;
[How constraints are solved](how-constraints-are-solved.md#the-loop-at-a-binding-group)
is that part.

## Kinds use the same machinery

Kind inference, in `src/kind.c`, is a type checker without
polymorphism or overloading: the kinds are `STAR` and pairs `(k1, k2)`
for `k1 -> k2`, unknown kinds are offsets, and they are unified in the
same substitution array (`newKindvars` is `newTyvars`).  That is why
`kind.c` is among the files compiled without optimisation, and why a
kind check is bracketed by `emptySubstitution`.  Gofer's author
suggested reading `kind.c` first as the easier introduction to the
type checker; that is still good advice.

## Where to read

| step | function | file |
|------|----------|------|
| instantiate a scheme | `instantiate` | subst.c |
| the substitution | `tyvar`, `newTyvars`, `expandSubst`, `unify` | subst.c |
| type an expression | `typeExpr`, `check` | type.c |
| assumptions | `enterBindings`, `findAssum`, `addEvidParams` | type.c |
| dictionary placeholders | `assumeEvid`, `overEvid` | preds.c |
| generalisation | `clearMarks`, `markType`, `copyType`, `generalize` | subst.c, type.c |
| binding groups | `typeBindings`, `monorestrict`, `unrestricted` | type.c |
| kinds | `kindTCGroup`, `inferKinds` | kind.c |

## See also

- [How constraints are solved](how-constraints-are-solved.md)
- [How source is read](how-source-is-read.md) — where offsets and
  `POLYTYPE` come from
- Mark P. Jones, *Qualified Types: Theory and Practice*, for the
  theory, and [*The implementation of the Gofer functional programming
  system*](http://web.cecs.pdx.edu/~mpj/pubs/RR-1030.pdf) (1994), §7,
  for the original description of this machinery
