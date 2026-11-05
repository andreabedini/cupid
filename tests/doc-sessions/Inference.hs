module Inference where

apply f x = f x

identity x = x

captured f = let g y = f y in g True

total []     = 0
total (x:xs) = x + total xs
