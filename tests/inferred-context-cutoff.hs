-- With no signature, the context of main is inferred: C [Char] needs
-- C [[Char]], which needs C [[[Char]]], and so on.  The reduction stops
-- at the -c limit and is reported, as it is when a signature is given,
-- rather than running until the C stack overflows.
module Main where

class C a where
    c :: a -> Int

instance C [[a]] => C [a] where
    c _ = 0

main = print (c "x")
