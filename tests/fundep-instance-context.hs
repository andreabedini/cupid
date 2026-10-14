-- An instance context may mention a variable that is not in the instance
-- head but is fixed by a functional dependency, as r is here.  Using the
-- instance must improve that constraint by the dependency before solving
-- it, or Show r matches no instance.  An expression at the prompt is
-- reduced by the instance rather than entailed, which is where it was
-- missed.
module Main where

class Rep a r | a -> r where
    rep :: a -> r

instance Rep Bool Int where
    rep b = if b then 1 else 0

class Describe a where
    describe :: a -> String

newtype Wrap a = Wrap a

instance (Rep a r, Show r) => Describe (Wrap a) where
    describe (Wrap x) = show (rep x)

main :: IO ()
main = putStrLn (describe (Wrap True))
