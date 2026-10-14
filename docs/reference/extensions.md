# Language extensions

The language features Cupid accepts beyond Haskell 2010.  They are
always enabled; there is no option to turn them off (see
[the design](../explanation/design.md#the-language)).  Most are needed by
the shipped libraries (see [The libraries](libraries.md)) and
correspond to GHC extensions, though GHC's versions are often more
general.

## Type classes

### Flexible contexts

In Haskell 2010 a context constrains type variables, possibly applied
to types.  In Cupid any type may be constrained, in signatures and in
class and instance declarations:

```haskell
g :: (C [a], D (a -> b)) => [a] -> b
```

### Flexible instances

In Haskell 2010 an instance head is a `data` or `newtype` constructor
applied to distinct type variables.  In Cupid any type may be made an
instance, and a type synonym may stand for it:

```haskell
instance Monoid (a -> a) where ...
instance Show (Tree Int) where ...
instance MyClass a where ...
instance C String where ...

type Pair a = (a, a)
instance Swap (Pair a) where ...
```

With flexible contexts as well, constraint solving is undecidable in
general.  Cupid bounds the depth of constraints it will reduce, set by
the `-c` option (default 40), and reports an error when the bound is
reached.  Reaching it usually means the constraints would not have
terminated.  GHC instead restricts the form of instances
syntactically.

### Overlapping instances

Two instances overlap when a type could match both:

```haskell
class C a where c :: a
instance C (Bool,a) where ...
instance C (a,Char) where ...
```

Cupid rejects this:

```
ERROR "Test.hs":4 - Overlapping instances for class "C"
*** This instance   : C (a,Char)
*** Overlaps with   : C (Bool,a)
*** Common instance : C (Bool,Char)
```

With `+o`, overlap is allowed when one instance type is a substitution
instance of the other and not equal to it:

```haskell
class C a where toString :: a -> String
instance C [Char] where ...
instance C a => C [a] where ...
```

For `[Char]` the first instance is chosen; for `[t]` with `t` any
other type, the second.  Contexts play no part in the choice.

When `t` is a type variable, as in

```haskell
f :: C a => [a] -> String
f xs = toString xs
```

the choice cannot be made and Cupid rejects the definition.  With `+O`
it is accepted and the more general instance is chosen, which is wrong
if `f` is later applied to a `[Char]`.

The `Prelude` avoids this situation for `Show` by giving the class a
`showList` method; the same technique often removes the need for
overlap.

### Multi-parameter type classes

A class may have more than one parameter, expressing a relation
between types:

```haskell
class Isomorphic a b where
    from :: a -> b
    to   :: b -> a
```

It may also have none.  A zero-parameter class is a named set of
values with a single instance, a way to give a program a configuration
point:

```haskell
class Config where
    verbose :: Bool

instance Config where
    verbose = True
```

### Functional dependencies

Multi-parameter classes are often ambiguous.  A functional dependency
after a `|` says that some parameters determine others:

```haskell
class MyClass a b c | a -> b where ...
```

There cannot be two instances with the same `a` and different `b`, and
the type checker uses that to resolve `b` once `a` is known.  Several
dependencies may be given, and a dependency may have several
parameters on either side:

```haskell
class MyClass a b c | a -> b, a -> c where ...
class MyClass a b c | a -> b c where ...
class MyClass a b c | a b -> c where ...
```

The design is from *Type Classes with Functional Dependencies*,
Mark P. Jones, ESOP 2000.

## Quantified types

### Rank-2 types

A function argument may be polymorphic if the function's signature
says so with `forall`:

```haskell
g :: (forall a. a -> a) -> (Bool, Char)
g f = (f True, f 'a')
```

Restrictions:

- A function with a polymorphic argument must have an explicit
  signature.
- A polymorphic argument may only be matched by a variable or a
  wildcard pattern.
- Such a function must always be applied to its polymorphic argument;
  `g` cannot be used on its own, passed to another function, or
  defined as `myRunST = runST`.  Write `myRunST x = runST x`.
- A type variable bound by `forall` must be used in the quantified
  type.

`runST :: (forall s. ST s a) -> a` in `Control.Monad.ST` has such a
type.

One exception to the third restriction: `f $ x` is checked as
`f x`, as GHC does, so `runST $ do ...` is accepted, and so is
`C $ \x -> ...` for a constructor `C` with a polymorphic field (see
below).  This applies to the Prelude's `$` only, and a type error in
`f $ x` is reported against `f x`.  Other uses of `$` are not
affected; `map ($ x) fs` means what it always did.

### Polymorphic components

A constructor field may be polymorphic:

```haskell
newtype List a = MkList (forall r. r -> (a -> r -> r) -> r)
newtype NatTrans f g = MkNT (forall a. f a -> g a)
data MonadT m = MkMonad {
    my_return :: forall a. a -> m a,
    my_bind   :: forall a b. m a -> (a -> m b) -> m b }
```

The constructors then have rank-2 types and must be applied to their
polymorphic arguments when used.  Record update is not available on
records with polymorphic fields.

### Existential quantification

A `forall` before a constructor quantifies a type variable that does
not appear in the result type:

```haskell
data Accum a = forall s. MkAccum s (a -> s -> s) (s -> a)

adder    = MkAccum 0 (+) id
averager = MkAccum (0,0) (\x (t,n) -> (t+x, n+1)) (uncurry (/))
```

Matching on `MkAccum s add get` gives an `s` of unknown type that can
only be passed to `add` and `get`:

```haskell
runAccum (MkAccum s add get) []     = get s
runAccum (MkAccum s add get) (x:xs) = runAccum (MkAccum (add x s) add get) xs
```

Existential constructors may not have named fields and their types may
not use `deriving`.

## GADT syntax

A `data` or `newtype` declaration may list its constructors as
signatures in a `where` block:

```haskell
data Shape a where
    Circle :: a -> Shape a
    Rect   :: a -> a -> Shape a
    Named  :: { label :: String, inner :: Shape a } -> Shape a
    Strict :: !Int -> a -> Shape a
    Nil, Unit :: Shape a
    (:+:)  :: Shape a -> Shape a -> Shape a
  deriving Show
```

Each signature means what the equation form would have meant.  Its
result must be the type being declared applied to distinct type
variables; those variables are the signature's own and are matched by
position with the ones the head binds, so `C :: a -> b -> T b`
declares a constructor whose second argument has the type's parameter
and whose first is existential.  Any variable not in the result is
existential, and a context is packed with the constructor and given
back on a match, as for a `forall` constructor in the equation form:

```haskell
data Some where
    Some :: Show a => a -> Some

showSome :: Some -> String
showSome (Some x) = show x
```

The head may give the type's kind instead of naming its parameters,
`data Expr :: * -> * where ...`, in which case the kind's arguments
bind as many variables as it has arrows (see [Kind
signatures](#kind-signatures)).  A `forall` at the front of a
signature is accepted and makes no difference.  A `newtype` in this
syntax has exactly one constructor, with one argument.

Differences from GHC's `GADTSyntax` and `GADTs`:

- A constructor whose result refines the type's arguments, such as
  `IntE :: Int -> Expr Int`, is rejected: these are generalised
  algebraic data types proper, which Cupid does not have.
- A context on a constructor constrains the constructor's own
  arguments, as in the equation form; it cannot be used to refine
  the result type either.
- The equation form's restrictions carry over: no `deriving` for a
  type with an existential or constrained constructor, and no
  selectors on one.

## Kind signatures

A declaration may say what kind its type constructor or class has.
The kind can be written on the variables the declaration binds:

```haskell
newtype Fix (f :: * -> *) = In (f (Fix f))
class Container (f :: * -> *) where
    empty :: f a
```

or in a standalone signature before or after the declaration, where
the kind of a class ends in `Constraint`:

```haskell
type Compose :: (* -> *) -> (* -> *) -> * -> *
newtype Compose f g a = Compose (f (g a))

type Shaped :: (* -> *) -> Constraint
class Shaped f where
    shape :: f a -> String
```

A kind is `*`, `Type`, `Constraint`, or an arrow between kinds, with
parentheses to group.  `Type` and `Constraint` are names the kind
grammar knows; nothing has to be in scope for them, but
`import Data.Kind (Type, Constraint)` is accepted so that programs
written for GHC load unchanged.  Write `* -> *` with spaces: `*->*` is
one operator symbol.

Every kind so declared is unified with the kind that inference would
otherwise start from, and inference then proceeds as usual, so the
signature can only narrow what is inferred.  A signature that does
not fit its declaration, for example one with the wrong number of
arrows, is reported against the signature; a declaration whose
constructors use a variable at a kind other than the one given is
reported as the usual kind error at the use.  A standalone signature
for a type or class the module does not declare is an error.  With
`+k`, `:info` shows the kinds.

Differences from GHC's `KindSignatures` and `StandaloneKindSignatures`:

- A kind annotation may be written only on the variables a `data`,
  `newtype`, `type` or `class` declaration binds, not inside a type
  or on a `forall`.
- A kind has no variables: there is no kind polymorphism.
- `Type` and `Constraint` are keywords of the kind grammar, not types
  exported from `Data.Kind`, and `Data.Kind.Type` cannot be written
  qualified in a kind.

## Equality constraints

A context may say that two types are the same:

```haskell
f :: (a ~ Int) => a -> Int
f x = x + 1

g :: (f a ~ Maybe Int, g ~ (Int -> Bool)) => f a -> g -> Bool
g m p = maybe False p m
```

Each side of `~` is a type without a function arrow, or any type in
parentheses.  A single equality may stand without parentheses,
`a ~ [Char] => a -> Int`, and an equality may sit beside class
constraints in the same context.  An equality may be written in the
context of a type signature, of an expression signature, of a class
method's signature, of a polymorphic argument or field type, and of an
instance declaration.

In a signature the equality is *given*: the body is checked with the two
sides made one type, so `f` above is checked at `Int -> Int`, and the
inferred type must be as general as the declared type read that way.
At a use of `f` the equality is *wanted*, and is solved at once by
unifying its sides, so `:type f` reports `Int -> Int`; `:info f` shows
the signature as written.  An equality carries no evidence: it is never
a dictionary parameter or argument.

In an instance context an equality lets the head match more than it
commits to, and the equality then fixes the rest:

```haskell
class Describe a where
    describe :: a -> String

instance (a ~ Char) => Describe [a] where
    describe s = "string " ++ s
```

`describe []` is accepted: the constraint `Describe [t]` matches the
instance for any `t`, and the equality makes `t` `Char`.  A type the
equality cannot make fit, `describe [True]`, is an error, reported at
the instance.

The two sides of an equality must have the same kind.  A type variable
that occurs only in an equality is not ambiguous, since the other side
determines it.

Differences from GHC's `TypeFamilies` and `GADTs`, which are where `~`
comes from:

- An equality may not appear in the context of a `data` or `newtype`
  declaration or of one of its constructors, in either syntax: that
  would refine the result type on a match, which is a generalised
  algebraic data type, and Cupid does not have those.
- An equality may not be a superclass.
- A given equality that cannot hold, `(Int ~ Bool) => ...`, is rejected
  at the signature; GHC accepts the signature and the function is
  simply uncallable.
- An instance whose context has an equality cannot be used by
  `deriving`.
- There are no type families, so an equality is never stuck: it is
  solved, or it is an error, where it arises.

## Anonymous type variables

An underscore in a type stands for a fresh type variable:

```haskell
count :: [_] -> Int
count = length
```

`:type count` reports `[a] -> Int`.  Each `_` is a different variable,
so `_ -> _` is `a -> b`.

## Type annotations in patterns

A pattern may carry a type signature:

```haskell
f (x :: Int) = fromIntegral x :: Double
```

A type variable in such an annotation, unless already bound by an
enclosing pattern annotation, is bound over the pattern's scope, and
type signatures inside that scope refer to it:

```haskell
sortImage :: Ord b => (a -> b) -> [a] -> [a]
sortImage (f :: a -> b) = sortBy cmp
  where
    cmp :: a -> a -> Ordering
    cmp x y = compare (f x) (f y)
```

The `a` in the signature of `cmp` is the one bound by the pattern.
The signature of `sortImage` itself is not connected to it: pattern
annotations bind variables in inner signatures, never the other way
round.

Differences from GHC's scoped type variables:

- GHC's pattern-bound type variables may be instantiated; in Cupid they
  are rigid, so `g (xs :: [a]) = xs ++ "\n"` is rejected.
- Variables bound in a class or instance head do not scope over the
  method definitions in Cupid.
- Result type signatures on the left of a definition are not accepted.

## Field puns

In a record pattern or construction, a field name on its own stands
for `field = field`:

```haskell
data R = R { a :: Int, b :: String }

getA (R {a}) = a
mk a b = R {a, b}
```

## Restricted type synonyms

A type synonym may be declared transparent inside the definitions of
named functions and opaque everywhere else, which makes an abstract
type without a `newtype` or a module boundary:

```haskell
type Stack a = [a] in emptyStack, push, pop, topOf, isEmpty

emptyStack :: Stack a
emptyStack  = []

push :: a -> Stack a -> Stack a
push  = (:)

pop :: Stack a -> Stack a
pop (_:xs) = xs
```

Within `emptyStack`, `push` and the other listed functions a `Stack a`
is a `[a]`.  Elsewhere it is not: `length emptyStack` is a type error,
because `Stack a` does not match `[a]`.  The list of functions may
carry their signatures instead of naming them:

```haskell
type Table = [(String, Int)] in
    emptyT :: Table,
    addT   :: String -> Int -> Table -> Table,
    findT  :: String -> Table -> Maybe Int
```

The demo `demos/Stack.hs` uses this.  The same effect is available
portably with a `newtype` or by not exporting a type's constructors.

## See also

- [Options](options.md) for `+o`, `+O` and `-c`
- [The language Cupid accepts](language.md)
