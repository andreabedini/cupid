-- Show (f Int) is not a class applied to type variables, so it cannot be
-- in a derived context; a standalone declaration can give it.
module Main where

data T f = T (f Int) deriving Show

main :: IO ()
main = return ()
