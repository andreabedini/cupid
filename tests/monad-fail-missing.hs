-- A refutable pattern in a do block needs a MonadFail instance, and
-- Either has none, so this is rejected rather than failing at run time.
module Main where

firstJust :: Either String (Maybe Int) -> Either String Int
firstJust m = do
    Just x <- m
    return x

main :: IO ()
main = print (firstJust (Right (Just 1)))
