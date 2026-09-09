-- A run of dashes followed by a symbol character is an operator, not a
-- comment (Haskell 2010 2.3), even at the start of a layout block that
-- `where' has just opened.  `--|' below is therefore a syntax error and
-- must not be taken for a comment.
module Main where

main :: IO ()
main = print foo
  where
    --| an operator, not a comment
    foo = (1 :: Int)
