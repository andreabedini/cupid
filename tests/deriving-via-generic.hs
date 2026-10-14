-- An instance derived via a type whose instance works over the generic
-- representation: the derived context is found by reducing through
-- Generic, whose dependency fixes the representation, and through one
-- constraint per constructor, field and index of the representation,
-- which is wide but not deep.  The context comes out as Eq a.
module Main where

import GHC.Generics

newtype Generically a = Generically a

class GEq f where
    geq :: f p -> f p -> Bool

instance GEq U1 where
    geq _ _ = True

instance Eq c => GEq (K1 i c) where
    geq (K1 a) (K1 b) = a == b

instance GEq f => GEq (M1 i c f) where
    geq (M1 a) (M1 b) = geq a b

instance (GEq f, GEq g) => GEq (f :+: g) where
    geq (L1 a) (L1 b) = geq a b
    geq (R1 a) (R1 b) = geq a b
    geq _      _      = False

instance (GEq f, GEq g) => GEq (f :*: g) where
    geq (a :*: b) (c :*: d) = geq a c && geq b d

instance (Generic a r, GEq r) => Eq (Generically a) where
    Generically x == Generically y = geq (from x) (from y)

data T a = A Int a Bool | B | C { f1 :: Char, f2 :: [a], f3 :: Int } | D a a a
    deriving Generic
    deriving Eq via Generically (T a)

same :: Eq a => T a -> T a -> Bool
same = (==)

main :: IO ()
main = do
    print (A 1 'x' True == A 1 'x' True, A 1 'x' True == A 1 'y' True)
    print (B == (B :: T ()), C 'c' "ab" 1 == C 'c' "ab" 2)
    print (same (D 1 2 3) (D 1 2 3), D 1 2 3 == D 1 2 (4 :: Int))
