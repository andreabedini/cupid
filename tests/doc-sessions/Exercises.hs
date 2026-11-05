module Exercises where

swap :: (a, b) -> (b, a)
swap (a, b) = (b, a)

twice :: (a -> a) -> a -> a
twice f x = f (f x)

isZero :: Int -> Bool
isZero n = if n == 0 then True else False

pick :: Bool -> Int -> Int
pick c x = g (if c then x else 0) where g y = y
