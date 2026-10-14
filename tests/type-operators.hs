-- A constructor operator may name a type: it can be declared infix or
-- with its operands in parentheses, used infix in any type, given a
-- fixity, exported with its constructors and printed back infix.
module Main (main, (:+:)(..), (:*:)(..)) where

infixr 5 :+:
infixr 6 :*:

data (f :+: g) p = L1 (f p) | R1 (g p)
data (f :*: g) p = f p :*: g p
newtype K a p = K a
data a :| b = a :| b deriving (Eq, Show)
type a :-> b = a -> b

class Size f where
    size :: f p -> Int

instance (Size f, Size g) => Size (f :+: g) where
    size (L1 x) = size x
    size (R1 y) = size y

instance (Size f, Size g) => Size (f :*: g) where
    size (a :*: b) = size a + size b

instance Size (K a) where
    size _ = 1

triple :: (K Int :*: K Bool :*: K Char) ()
triple = K 1 :*: K True :*: K 'c'

either3 :: ((K Int :*: K Bool :*: K Char) :+: K ()) ()
either3 = L1 triple

twice :: Int :-> Int
twice = (* 2)

main :: IO ()
main = do
    print (size triple)
    print (size either3)
    print (size (R1 (K ()) `asTypeOf` either3))
    print ((1 :: Int) :| 'x')
    print (twice 21)
