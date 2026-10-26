-- At the prompt, the context of an expression is reduced the same way
-- and stops at the -c limit, here 5, rather than running until the C
-- stack overflows.
module Main where

class C a where
    c :: a -> Int

instance C [[a]] => C [a] where
    c _ = 0
