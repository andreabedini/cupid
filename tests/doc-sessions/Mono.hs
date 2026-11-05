module Mono where

total :: [Int] -> Int
total []     = 0
total (x:xs) = x + total xs
