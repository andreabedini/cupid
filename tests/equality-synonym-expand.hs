-- A synonym that drops its argument relates nothing through it: K a ~ K b
-- holds for any a and b, as both are Int, and a ~ [K a] is the finite
-- a ~ [Int].  A synonym that keeps its argument still relates it.
module Main where

type K a = Int
type L a = [a]

anyTwo :: (K a ~ K b) => a -> b -> Int
anyTwo _ _ = 1

same :: (L a ~ L b) => a -> b
same x = x

finite :: (a ~ [K a]) => a -> Int
finite = length

phantom :: K a -> K Bool
phantom x = x + 1

main :: IO ()
main = print (anyTwo True 'x', same 'y', finite [3, 4], phantom 5)
