-- deriving C via V gives a type the instance of another with the same run
-- time representation, reusing its methods; deriving newtype C is deriving
-- C via the type the newtype wraps.  A declaration may have several
-- deriving clauses, and the via type may mention the declaration's type
-- variables and have its instance further down the module.
module Main where

import Data.Monoid

newtype Score = Score Int
    deriving Show
    deriving (Eq, Ord) via Int
    deriving (Semigroup, Monoid) via Sum Int
    deriving newtype Num

newtype Opt a = Opt (Maybe a)
    deriving (Eq, Ord) via Maybe a
    deriving stock Show

newtype Rev = Rev Int
    deriving (Eq, Ord) via Down Int
    deriving stock Show

newtype Down a = Down a

instance Eq a => Eq (Down a) where
    Down a == Down b = a == b

instance Ord a => Ord (Down a) where
    compare (Down a) (Down b) = compare b a

type Ints = [Int]

newtype Bag = Bag Ints
    deriving (Semigroup, Monoid) via Ints
    deriving newtype Show

main :: IO ()
main = do
    print (Score 3 == Score 3, compare (Score 1) (Score 2))
    print (mconcat [Score 1, Score 2, Score 39], Score 2 * Score 21)
    print (Opt (Just 1) < Opt Nothing, Opt (Just 'x') == Opt (Just 'x'))
    print (maximum [Rev 3, Rev 1, Rev 2])
    print (mconcat [Bag [1], Bag [2, 3]])
