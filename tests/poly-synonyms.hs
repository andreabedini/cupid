-- A type synonym for a polymorphic type is expanded where it is used.  A
-- forall it leaves to the right of an arrow moves to the front of the
-- signature; as an argument it is a rank-2 type; as a field, a polymorphic
-- component.  A forall may give its variables kinds.
module Main where

import Data.Kind (Type)

newtype Identity a = Identity { runIdentity :: a }
instance Functor Identity where fmap f (Identity a) = Identity (f a)
newtype Const r a = Const { getConst :: r }
instance Functor (Const r) where fmap _ (Const r) = Const r

type Lens s t a b = forall f. Functor f => (a -> f b) -> s -> f t
type SimpleLens s a = Lens s s a a                -- a synonym of one
type Getting r s a = (a -> Const r a) -> s -> Const r s
type ASetter s t a b = (a -> Identity b) -> s -> Identity t

lens :: (s -> a) -> (s -> b -> t) -> Lens s t a b
lens sa sbt afb s = fmap (sbt s) (afb (sa s))

_1 :: Lens (a, c) (b, c) a b
_1 = lens fst (\(_, c) b -> (b, c))

_2 :: SimpleLens (c, a) a
_2 = lens snd (\(c, _) a -> (c, a))

view :: Getting a s a -> s -> a
view l s = getConst (l Const s)

over :: ASetter s t a b -> (a -> b) -> s -> t
over l f = runIdentity . l (Identity . f)

viewAndOver :: SimpleLens s a -> (a -> a) -> s -> (a, s)  -- rank 2
viewAndOver l f s = (view l s, over l f s)

newtype ReifiedLens s a = ReifiedLens { runLens :: SimpleLens s a }

-- In the style of "Scrap your boilerplate"
class Term a where
    gmapT :: GenericT -> a -> a
    gmapT _ x = x

type GenericT = forall a. Term a => a -> a
type GenericQ r = forall a. Term a => a -> r
type Fun a = a -> Int

-- a synonym's argument goes where its body puts it, so it may be
-- polymorphic there: GenericQ GenericT is forall a b. (Term a, Term b) =>
-- a -> b -> b, and Fun GenericT has a rank-2 argument
twinT :: GenericQ GenericT -> Int -> Bool -> (Int, Bool)
twinT f x y = (f y x, f x y)

countWith :: Fun GenericT
countWith g = case g (1 :: Int, True) of (n, _) -> n

instance Term Int
instance Term Bool
instance (Term a, Term b) => Term (a, b) where
    gmapT f (a, b) = (f a, f b)

everywhere :: GenericT -> GenericT
everywhere f x = f (gmapT (everywhere f) x)

-- a forall written in the result, and one giving kinds
twice :: (forall a. a -> a) -> (forall b. b -> (b, b))
twice f x = (f x, f x)

lift2 :: forall (t :: (Type -> Type) -> Type -> Type) (m :: Type -> Type) a.
         t m a -> t m a
lift2 x = x

main :: IO ()
main = do
    print (view _1 (1 :: Int, "x"))
    print (over _2 (+ 1) ("y", 41 :: Int))
    print (viewAndOver _1 negate (3 :: Int, True))
    print (view (_1 . _2) ((True, 'q'), ()))
    print (view (runLens (ReifiedLens _2)) ((), "field"))
    print (everywhere id (1 :: Int, (True, 2 :: Int)))
    print (twice id 'x')
    print (twinT (\_ b -> b) 1 True, countWith id)
