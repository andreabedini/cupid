#! /usr/local/bin/runcuddles
-- A line beginning with #! is read as a blank line, so an executable
-- script is an ordinary module after it.
module Main where

main :: IO ()
main = putStrLn "ran"
