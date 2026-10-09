# How constraints are solved

The type checker in `type.c` infers types by unification, and alongside
the types it collects *predicates*: the class constraints that uses of
overloaded names give rise to.  This page is about what happens to
those predicates.  The code is in `preds.c` and the second half of
`subst.c`; `type.c` decides when each step runs.  The aim is that this remain
one solver as the language grows, and this page is the description to
check a change against.

## Predicates and evidence

A predicate is a class applied to types, `Eq a` or `MonadState s m`,
represented as the class cell applied to the type cells.  Overloading
is implemented by dictionary passing: a function with the type
`Eq a => a -> Bool` takes one more argument than its type shows, the
dictionary for `Eq a`, and a use of it supplies that dictionary.  The
solver's job is to say, for every predicate a binding gives rise to,
where its dictionary comes from.

While a binding group is being checked, the predicates it has given
rise to so far live in one list, `preds`.  Each entry is a triple
`(pi, o, ev)`: the predicate skeleton `pi`, the offset `o` at which its
type variables are read in the substitution, and `ev`, a fresh
*dictionary variable*.  The dictionary variable is a placeholder cell
that has already been written into the translated expression where the
dictionary argument goes.  When the solver finds the dictionary, it
overwrites the placeholder with it (`overEvid`); when it does not, the
placeholder becomes a lambda parameter of the binding (`qualify`), and
the predicate goes into the binding's type as part of its context.

Predicates enter the list in two ways.  A *wanted* predicate comes from
a use: `typeAp` instantiates the type of the name being applied, and
for each predicate in its context calls `assumeEvid`, which invents the
dictionary variable, adds the triple to `preds` and returns the
variable to be applied as an argument.  Literals and `do` blocks do the
same for `Num`, `Fractional` and `Monad`.  A *given* predicate comes
from a context that may be assumed: the context of a type signature
while its binding is checked, the context of an instance while its
methods are checked, the constraints a constructor was declared with
while a match on it is checked.  `makePredAss` turns such a context
into triples whose dictionary variables are the binding's extra
parameters, and these are the evidence the wanteds may be discharged
with.

## Entailment

The question the solver asks is whether a predicate follows from a set
of given ones, and if so what its dictionary is.  `entail` answers it
in two stages.

`scEntail` uses only the class hierarchy.  A predicate follows from a
given one that is the same predicate, with the given's dictionary as
evidence, or from a given whose class has it as a superclass, with the
superclass selector applied to the given's dictionary.  `scFind` walks
down the superclass hierarchy from each given; classes carry a `level`
so that the walk stops as soon as it is below the predicate's class.

`inEntail` uses the instance declarations.  `findInstFor` looks for an
instance whose head matches the predicate by one-way matching (only
the instance's variables may be bound), and if there is one, the
instance's own context, read at the fresh variables the match
allocated, is what now has to be entailed.  Each of those predicates
goes back through `entail`, and the dictionary is the instance's
*builder* applied to their dictionaries.  The recursion is bounded by
the `cutoff` depth, the `-c` option, since with flexible contexts and
instances it need not terminate.

With overlapping instances allowed (`+o`), `findInstFor` also refuses a
match when a more specific instance could still apply once more is
known about the type; this is why context reduction is unsound under
overlap and `elimTauts` falls back to plain entailment then.

## Improvement

Functional dependencies let a predicate say something about types
rather than only ask something of them.  If a class has `| a -> b`,
two predicates for it that agree on `a` must agree on `b`, and a
predicate that matches an instance head on `a` must have that
instance's `b`.  `improve` runs over the list looking for such pairs
and unifies the determined positions (`pairImprove`, `instImprove`);
the unification binds type variables, which may make further
predicates reducible, so the loop runs until nothing changes.  An
instance's context is improved against the predicates in hand
(`improve1`) before it is entailed, so that the information flows into
the recursion too.

## The loop at a binding group

A binding group is checked with `preds` emptied; `savePreds` holds the
enclosing group's list meanwhile.  When the group's bindings have been
typed, `unrestricted` or `monorestrict` runs the solver over what has
accumulated, in this order:

1. **Improve** (`improve`), so that the types are as specific as the
   constraints make them before anything is decided.
2. **Reduce** what can be reduced.  `elimTauts` discharges every
   predicate an instance resolves (`reducePreds`), replacing it by
   the instance's context; `scSimplify` then drops any predicate that
   another in the list entails through superclasses.  Where a
   signature or an instance context is given, `elimPredsUsing`
   discharges each predicate the givens entail and complains about
   those that cannot be, since a declared context must be enough
   (`cantEstablish`).
3. **Defer** what belongs further out.  A predicate none of whose
   variables are generic in this group, that is, one that mentions
   only the enclosing scope's variables, cannot be decided here and
   is handed back to `savePreds` (`elimOuterPreds`, or
   `elimPredsUsing` when there are givens, which also splits a
   predicate that mixes generic and fixed variables by an instance
   if one applies).
4. **Default** what is left over.  At the top level, and for
   predicates on variables the group's types do not mention,
   `resolveDefs` applies the `default` declaration: a variable
   constrained only by standard classes, at least one of them
   numeric, is set to the first default type that satisfies them all,
   and the loop goes round again.
5. **Generalise** the rest.  Each predicate still in `preds` goes into
   the inferred type's context (`generalize`) and its dictionary
   variable becomes a parameter (`qualifyBinding`).  For a binding
   with a signature there is no rest: the signature's context was
   given, the inferred type must be at least as general as the
   declared one (`sameSchemes`, else `tooGeneral`), and anything the
   givens could not discharge is an error.

At the very top of a module, nothing is left to defer to, so
`typeDefnGroup` reports whatever survives as an unresolved instance.
An expression typed at the prompt is treated the same way with
`interactive` defaulting, which also lets `Show`, `Eq` and `Ord` on
their own pick a default.

## Scopes, skolems and local givens

Two features put constraints and type variables into a scope narrower
than a binding group.  An existential constructor, matched in a
pattern, introduces type variables that must not escape the
alternative: they are made *skolem* constants, which `unify` refuses
to bind.  A constructor declared with a context packs the dictionaries
for it, and a match on it gets them back as givens for the alternative.

`type.c` keeps a stack of scopes for this, one entry per lambda,
alternative, generator, `let` and top-level group, each holding the
skolems it bound and the givens it was handed (`enterScope`,
`leaveScope`).  On leaving, the predicates collected inside are
discharged against the scope's givens where they can be
(`elimPredsUsing` again) and the rest passed outward, and the inferred
type and the assumptions are checked for skolems that would escape.
The scope stack is where a future given that holds for one alternative
only, as a GADT match provides, would live; 0043 says so.

## Where to read

| step | function | file |
|------|----------|------|
| invent a wanted | `assumeEvid` | preds.c |
| take a context as given | `makePredAss` | preds.c |
| entailment by superclasses | `scEntail`, `scFind` | preds.c |
| entailment by instances | `inEntail`, `findInstFor` | preds.c, subst.c |
| improvement by dependencies | `improve`, `pairImprove` | subst.c |
| context reduction | `elimTauts`, `reducePreds`, `scSimplify` | preds.c |
| discharge against givens | `elimPredsUsing` | preds.c |
| deferral outward | `elimOuterPreds` | preds.c |
| defaulting | `resolveDefs` | preds.c |
| the loop itself | `unrestricted`, `monorestrict`, `typeExpected`, `typeMember` | type.c |
| scopes and skolems | `enterScope`, `leaveScope` | type.c |

## See also

- [How Cuddles runs a program](how-cuddles-runs-a-program.md) for the
  stages around the type checker
- [Language extensions](../reference/extensions.md) for what the
  flexible contexts, overlap and functional dependencies accept
- Mark P. Jones, *Qualified Types: Theory and Practice*, and
  *Type Classes with Functional Dependencies*, for the theory the
  implementation follows
