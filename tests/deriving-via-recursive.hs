-- A recursive newtype never reaches its type at run time, so checking
-- that it is the same as the via type must stop rather than loop.
module Main where

newtype Loop = Loop Loop deriving Show via Int

main :: IO ()
main = return ()
