-- deriving Data gives a type the instance of Data.Data's class, so that
-- generic functions over its constructors and fields work on it.
module Main where

import Data.Data

data Shape = Circle Double | Rect { width :: Double, height :: Double } | Dot
    deriving (Show, Data)

data Expr a = Lit a | Expr a :+: Expr a | Neg (Expr a)
    deriving (Show, Data, Typeable)

newtype Name = Name String deriving (Show, Data)

-- Every Int anywhere in a value, found generically.
ints :: Data a => a -> [Int]
ints x = case cast x of
    Just n  -> [n]
    Nothing -> concat (gmapQ ints x)

-- Add one to every Int anywhere in a value.
bump :: Data a => a -> a
bump x = case cast x of
    Just n  -> fromJust (cast (n + 1 :: Int))
    Nothing -> gmapT bump x
  where fromJust (Just y) = y

main :: IO ()
main = do
    print (toConstr (Circle 1), toConstr (Rect 2 3), toConstr Dot)
    print (dataTypeName (dataTypeOf Dot), map showConstr (dataTypeConstrs (dataTypeOf Dot)))
    print (constrFields (toConstr (Rect 1 2)), constrFixity (toConstr (Lit 1 :+: Lit (2 :: Int))))
    print (constrIndex (toConstr (Rect 1 2)), toConstr (Name "n"))
    print (ints (Neg (Lit 3 :+: Lit 4) :: Expr Int), ints [Just (1 :: Int), Nothing, Just 2])
    print (bump (Lit 1 :+: Neg (Lit (2 :: Int))))
    print (fromConstr (indexConstr (dataTypeOf Dot) 3) :: Shape)
    print (gmapQ (\d -> show (typeOf d == typeOf (0 :: Double))) (Rect 1 2))
    print (fromConstrB (fromConstr (toConstr (0 :: Double))) (toConstr (Circle 9)) :: Shape)
