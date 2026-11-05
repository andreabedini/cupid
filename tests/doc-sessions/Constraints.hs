module Constraints where

same :: Eq a => a -> a -> Bool
same x y = x == y

sameInt :: Int -> Int -> Bool
sameInt x y = same x y

sameOrdered :: Ord a => a -> a -> Bool
sameOrdered x y = same x y

sameList :: Eq a => [a] -> [a] -> Bool
sameList xs ys = same xs ys

sameInferred x y = x == y

class Element c e | c -> e where
  first :: c -> e

instance Element [a] a where
  first = head

data Packed = forall a. Show a => Packed a

render :: Packed -> String
render (Packed x) = show x

sameType :: (a ~ b) => a -> b
sameType x = x
