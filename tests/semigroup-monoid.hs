-- Semigroup is a superclass of Monoid, as in GHC's base: a Monoid
-- instance needs a Semigroup instance, mappend defaults to <>, and the
-- Prelude exports both classes.
module Main where

import Data.Monoid
import qualified Data.Semigroup as S
import Data.Semigroup (Min(..), Max(..), Arg(..), sconcat, stimes)
import Data.List (sortBy)

-- A monoid defined the modern way: no mappend.
newtype MaxLen = MaxLen Int deriving Show

instance Semigroup MaxLen where
    MaxLen a <> MaxLen b = MaxLen (max a b)

instance Monoid MaxLen where
    mempty = MaxLen 0

data P = P String Int deriving Show

main :: IO ()
main = do
    print (mconcat (map (MaxLen . length) ["a", "abc", "ab"]))
    print (MaxLen 2 `mappend` MaxLen 5, mempty :: MaxLen)
    putStrLn ("ab" <> "cd" <> mempty)
    print (Just [1] <> Nothing <> Just [2, 3 :: Int])
    print (sortBy (\(P a x) (P b y) -> compare x y <> compare a b)
                  [P "b" 2, P "a" 2, P "c" 1])
    print (getSum (foldMap' Sum [1 .. 10 :: Int]), getProduct (Product 3 <> Product (4 :: Int)))
    print (getAll (All True <> All False), getAny (Any True <> mempty))
    print (getFirst (First Nothing <> First (Just 1) <> First (Just (2 :: Int))))
    print (getLast (Last (Just 1) <> Last (Just 2) <> Last (Nothing :: Maybe Int)))
    print (getAlt (Alt Nothing <> Alt (Just 'x')))
    print (getMin (Min 3 <> Min (1 :: Int)), getMax (sconcat (Max 3 S.:| [Max 7, Max (5 :: Int)])))
    print (S.getFirst (S.First 'a' <> S.First 'b'), S.getLast (S.Last 'a' <> S.Last 'b'))
    print (stimes (3 :: Int) "ab", stimes (2 :: Int) (Sum (5 :: Int)))
    let Min (Arg k v) = Min (Arg 3 "three") <> Min (Arg 1 "one") <> Min (Arg (2 :: Int) "two")
    print (k, v)
    print (appEndo (Endo (+ 1) <> Endo (* 2)) (5 :: Int), getDual (Dual "a" <> Dual "b"))
    (r, n) <- return (("x", Sum 1) <> ("y", Sum (2 :: Int)))
    print (r, getSum n)
  where
    foldMap' f = mconcat . map f
