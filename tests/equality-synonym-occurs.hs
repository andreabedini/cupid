-- A synonym that keeps its argument does not hide an infinite type.
module Main where

type I a = a

f :: (a ~ [I a]) => a -> Int
f = length

main :: IO ()
main = return ()
