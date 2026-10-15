-- A derived context may hold a class applied to distinct type variables,
-- however they are applied to each other: Show (f a), Show (f (g a)),
-- Show (f a b).
module Main where

data T f a = T (f a) deriving (Show, Read, Eq, Ord)
data P f g a = P (f a) (g a) deriving (Show, Eq, Ord)
data C f g a = C (f (g a)) deriving Show
data B f a b = B (f a b) deriving Show
data R f a = R (f a) (R f a) | E deriving Show
data A a = A (B' a) deriving Show
data B' a = B' a (A [a]) | N deriving Show
newtype W f a = W (f a) deriving newtype Show

main :: IO ()
main = do
    print (T (Just 1), read "T (Just 2)" :: T Maybe Int, T [1] == T [1], compare (T "b") (T "a"))
    print (P (Just 1) [2], P [1] [2] < P [1] [3])
    print (C [Just 'x'], B (Left 3 :: Either Int Bool), R [1] (R [2] E))
    print (A (B' 1 (A N)), W (Just True))
