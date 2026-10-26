-- -i names a header for the C that fficupid generates.  runcupid and
-- cupid generate no C, so they warn about it as about any other unknown
-- option, rather than accepting it and doing nothing with it.
module Main where

main :: IO ()
main = putStrLn "ran"
