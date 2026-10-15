-- Every type is Typeable under its own name, so deriving Typeable is
-- ignored whatever the strategy: deriving it via or as newtype of the
-- type a newtype wraps must not give the newtype that type's name, which
-- would let cast coerce between them.
module Main where

import Data.Typeable

newtype A = A Int deriving Typeable via Int
newtype B = B Int deriving newtype Typeable
newtype C = C Int
newtype D = D Int

deriving via Int instance Typeable C
deriving newtype instance Typeable D

main :: IO ()
main = do
    print (typeOf (A 1), typeOf (B 2), typeOf (C 3), typeOf (D 4))
    print (typeOf (A 1) == typeOf (1 :: Int), typeOf (C 3) == typeOf (3 :: Int))
    print (cast (A 1) :: Maybe Int, cast (C 3) :: Maybe Int)
