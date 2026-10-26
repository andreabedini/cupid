-- Only a first line beginning with #! is read as a blank line.  Further
-- down, #! is an operator symbol, which cannot start a declaration.
module Main where

#! /usr/local/bin/runcupid

main :: IO ()
main = putStrLn "ran"
