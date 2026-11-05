module Main where

total :: Num a => [a] -> a
total []     = 0
total (x:xs) = x + total xs

main :: IO ()
main = print (total [1, 2, 3 :: Int])
