-- A tuple with components left out is a function of the missing ones,
-- in order; with all of them left out it is the tuple constructor, and
-- with none a tuple, as before.
module Main where

main :: IO ()
main = do
    print ((,True) 'a')
    print ((1 :: Int,) "b")
    print ((1 :: Int,,3 :: Int,) 'x' 'y')
    print (map (,0 :: Int) "ab")
    print (zipWith3 (,,) [1 :: Int, 2] "cd" [True, False])
    print ((1 :: Int, 'z'), ())
