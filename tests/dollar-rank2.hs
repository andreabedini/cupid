-- f $ x is type checked as f x, so f may take a polymorphic argument:
-- runST $ do ... and C $ \sk fk -> ... for a constructor with a rank-2
-- field are accepted, as logict and much other library code expects.
module Main where

import Control.Monad.ST
import Data.STRef

newtype Church a = Church { runChurch :: forall r. (a -> r -> r) -> r -> r }

instance Functor Church where
    fmap f c = Church $ \cons nil -> runChurch c (cons . f) nil

fromList :: [a] -> Church a
fromList xs = Church $ \cons nil -> foldr cons nil xs

sumST :: [Int] -> Int
sumST xs = runST $ do
    r <- newSTRef 0
    mapM_ (\x -> modifySTRef r (+ x)) xs
    readSTRef r

main :: IO ()
main = do
    print (sumST [1 .. 10])
    print (runChurch (fmap (* 2) (fromList [1, 2, 3 :: Int])) (:) [])
    print (($) negate 3 :: Int)
    print (map ($ 2) [(+ 1), (* 3) :: Int -> Int])
