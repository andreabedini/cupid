-- A class whose methods involve a functional dependency cannot be derived
-- via another type: the dependency would fix b by A's instance of F while
-- the reused method was written for Int's, so c could return a Bool where
-- the caller expects a Char.
module Main where

class F a b | a -> b where
    f :: a -> b

instance F Int Bool where
    f = even

class C a where
    c :: F a b => a -> b

instance C Int where
    c = f

newtype A = A Int deriving C via Int

instance F A Char where
    f _ = 'a'

main :: IO ()
main = return ()
