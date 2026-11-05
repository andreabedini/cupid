module Compile where

firstOf :: (a, b) -> a
firstOf p = let (x, _) = p in x

classify :: Int -> String
classify n
  | n < 0     = "negative"
  | n == 0    = "zero"
classify 1 = "one"
classify _ = "many"

scale :: Int -> [Int] -> [Int]
scale k xs = go xs
  where go []     = []
        go (y:ys) = k * y : go ys

both :: Bool -> Bool -> Bool
both a b = a && b

lazyIf :: Bool -> Int -> Int
lazyIf c x = let y = if c then x else 0 in y + y

strictHead :: Bool -> Int -> Int -> Int
strictHead c x y = (if c then negate else id) (x + y)
