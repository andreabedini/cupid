-- The Prelude's folds are those of Foldable and its mapM and sequence
-- those of Traversable, as in GHC's base, so they work on any structure
-- with an instance.  Data.List, Data.Foldable, Data.Traversable and
-- Control.Monad export the same functions, so importing them all
-- unqualified leaves no name ambiguous.
module Main where

import Data.List
import Data.Foldable
import Data.Traversable
import Control.Monad
import Data.Array (listArray)
import qualified Data.Map as Map

data Tree a = Leaf | Node (Tree a) a (Tree a)

instance Functor Tree where
    fmap _ Leaf = Leaf
    fmap f (Node l x r) = Node (fmap f l) (f x) (fmap f r)

-- foldMap alone gives every other method.
instance Foldable Tree where
    foldMap _ Leaf = mempty
    foldMap f (Node l x r) = foldMap f l <> f x <> foldMap f r

instance Traversable Tree where
    traverse _ Leaf = pure Leaf
    traverse f (Node l x r) = Node <$> traverse f l <*> f x <*> traverse f r

tree :: Tree Int
tree = Node (Node Leaf 1 Leaf) 5 (Node (Node Leaf 3 Leaf) 4 Leaf)

main :: IO ()
main = do
    print (toList tree, length tree, sum tree, product tree)
    print (maximum tree, minimum tree, elem 3 tree, null tree, null Leaf)
    print (foldr (:) [] tree, foldl (flip (:)) [] tree, foldr1 (-) tree)
    print (length (Just 'x'), sum (Right 3 :: Either String Int), length ("k", 'v'))
    print (concat (Just [1, 2]), concatMap show (Just 42), and (Just False))
    print (sum (listArray (0, 2) [1, 2, 3 :: Int]), length (Map.fromList [(1, 'a'), (2, 'b')]))
    print (fmap toList (traverse (\x -> if x > 0 then Just x else Nothing) tree))
    print (sequenceA [Just 1, Just 2], sequence [Just 1, Nothing])
    print (traverse (\x -> [x, x * 10]) [1, 2])
    t <- mapM (\x -> do { print x; return (x * 2) }) tree
    print (toList t)
    forM_ (Just "forM_ over Maybe") putStrLn
    for_ [1, 2, 3 :: Int] print
    traverse_ print (Right 'r' :: Either () Char)
    print (mapAccumL (\acc x -> (acc + x, acc)) 0 [1, 2, 3])
    print (mapAccumR (\acc x -> (acc + x, acc)) 0 [1, 2, 3])
    print (find even tree, maximumBy (\a b -> compare (snd a) (snd b)) [(1, 'a'), (2, 'c'), (3, 'b')])
    print (foldl' (+) 0 [1 .. 100], msum [Nothing, Just 1, Just 2], asum [[], [3], [4]])
    r <- foldlM (\acc x -> do { putStrLn ("add " ++ show x); return (acc + x) }) 0 (Just 7)
    print (r, foldMap (\x -> [x, x]) tree)
