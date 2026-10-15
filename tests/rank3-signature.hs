-- A polymorphic argument of an argument is rank 3, which is not supported:
-- an error, where it used to be an internal one.
module Main where

f :: ((forall a. a -> a) -> Int) -> Int
f g = g id

main :: IO ()
main = print (f (\h -> h 3))
