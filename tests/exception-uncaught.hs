-- An exception that nothing catches is reported with displayException,
-- and the program exits with status 1.
module Main where

import Control.Exception

data Oops = Oops Int deriving Show

instance Exception Oops where
    displayException (Oops n) = "oops number " ++ show n

main :: IO ()
main = do
    putStrLn "before"
    _ <- throwIO (Oops 3)
    putStrLn "after"
