-- In a left section (e op) the operator must apply to the whole of e:
-- 1 + 2 * x is 1 + (2 * x), so (1 + 2 *) is not a section of *
-- (Haskell 2010 3.5) and must be rejected rather than read as one.
module Main where

main :: IO ()
main = print ((1 + 2 *) 10)
