-- A polymorphic type cannot be the argument of a type constructor.
module Main where

type Id = forall a. a -> a

ids :: [Id]
ids = [id]

main :: IO ()
main = return ()
