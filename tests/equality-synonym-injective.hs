-- K a ~ K b does not make a and b the same, as K drops its argument.
module Main where

type K a = Int

bad :: (K a ~ K b) => a -> b
bad x = x

main :: IO ()
main = return ()
