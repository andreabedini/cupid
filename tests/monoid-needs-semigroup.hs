-- A Monoid instance needs a Semigroup instance for the same type.
module Main where

newtype Count = Count Int

instance Monoid Count where
    mempty = Count 0
    mappend (Count a) (Count b) = Count (a + b)

main :: IO ()
main = case mconcat [Count 1, Count 2] of Count n -> print n
