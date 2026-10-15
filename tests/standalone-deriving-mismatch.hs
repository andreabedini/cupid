-- Deriving via a type needs the two types to be the same at run time.
module Main where

newtype Age = Age Int
deriving via Bool instance Show Age

main :: IO ()
main = print (Age 3)
