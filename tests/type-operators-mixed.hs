-- Fixity declarations do not apply to types, so a chain of different
-- type operators must be parenthesised: it is rejected rather than
-- grouped in a way that might not be the one meant.
module Main where

data (f :+: g) p = L1 (f p) | R1 (g p)
data (f :*: g) p = f p :*: g p
newtype K a p = K a

x :: (K Int :+: K Bool :*: K Char) ()
x = L1 (K 1)

main :: IO ()
main = return ()
