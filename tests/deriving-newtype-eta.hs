-- Functor (N) would be derived via what N wraps less its last parameter,
-- but Either a Int does not end in a.
module Main where

newtype N a = N (Either a Int) deriving newtype Functor

main :: IO ()
main = return ()
