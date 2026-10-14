-- A left section's operand may be an infix expression (Haskell 2010
-- 3.5), as long as the section's operator applies to the whole of it
-- once fixities are resolved: (a + b +) means \x -> (a + b) + x.
module Main where

main :: IO ()
main = do
    print ((1 + 2 +) 10)                -- infixl 6 after infixl 6
    print ((1 - 2 -) 10)                -- left-associative, not 1 - (2 - 10)
    print ((3 * 4 +) 10)                -- * binds more tightly than +
    print ((- 1 +) 10)                  -- unary minus, infixl 6
    print (("a" ++ "b" `elem`) ["ab"])  -- infixr 5 under infix 4
    print ((Just 1 >>= return >>=) (Just . (+ 1)))
