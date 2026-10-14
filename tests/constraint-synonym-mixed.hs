-- A tuple with a class in it is a constraint, so a type in it is reported
-- as an undefined class.
module Main where

type Printable a = (Show a, Int)

main :: IO ()
main = return ()
