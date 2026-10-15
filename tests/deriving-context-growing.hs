-- The context of the derived instance would be Show (f a), Show (f [a]),
-- Show (f [[a]]), ... without end.  As in GHC, a derived context may only
-- hold a class applied to distinct type variables, so Show (f [a]) is an
-- error, and inferring the context stops.
module Main where

data T f a = T (f a) (T f [a]) deriving Show

main :: IO ()
main = return ()
