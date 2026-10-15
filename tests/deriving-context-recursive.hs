-- Show (Fix f) would need Show (f (Fix f)), which is not a class applied
-- to type variables, so it is rejected at the declaration, as in GHC,
-- rather than where the instance is used.
module Main where

data Fix f = Fix (f (Fix f)) deriving Show

main :: IO ()
main = print (Fix (Just (Fix Nothing)))
