-- Working out a derived instance's context need not terminate: here
-- C [a] needs C [[a]], which needs C [[[a]]], and so on.  The reduction
-- is cut off when it is as deep as the -c limit, and reported.
module Main where

class C a where
    c :: a -> Int

instance C [[a]] => C [a] where
    c _ = 0

newtype L a = L [a] deriving C via [a]

main :: IO ()
main = return ()
