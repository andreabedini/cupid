-- A type can only be derived via one that is the same at run time once
-- newtypes and synonyms are unwrapped; Int and Bool are not.
module Main where

newtype Age = Age Int deriving Eq via Bool

main :: IO ()
main = return ()
