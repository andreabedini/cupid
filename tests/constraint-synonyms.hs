-- A type synonym for a constraint: a class, a tuple of them, or () given
-- the kind Constraint.  In a context it stands for its parts.
module Main where

import GHC.Stack (HasCallStack)
import Data.Kind (Constraint)
import Control.Monad.IO.Class (MonadIO(..))

type Partial = HasCallStack
type Trivial = (() :: Constraint)
type ShowEq a = (Show a, Eq a)
type ShowOrd a = (ShowEq a, Ord a)           -- a synonym of a synonym
type MonadIOFail m = (MonadIO m, MonadFail m)

type Unit = ()                               -- still the unit type
type Wrapped a = (Maybe a :: Type)           -- a type, with its kind

headP :: Partial => [a] -> a
headP (x:_) = x
headP []    = error "headP: empty list"

three :: Trivial => Int
three = 3

firstRepeat :: ShowEq a => [a] -> String
firstRepeat (x:y:_) | x == y = show x
firstRepeat xs               = show (length xs) ++ " elements"

biggest :: ShowOrd a => [a] -> String
biggest = show . maximum

addOne :: MonadIOFail m => m (Maybe Int) -> m Int
addOne m = do
    Just x <- m
    liftIO (print x)
    return (x + 1)

unit :: Unit
unit = ()

wrapped :: Wrapped Char
wrapped = Just 'w'

main :: IO ()
main = do
    print (headP "abc", three, unit, wrapped)
    putStrLn (firstRepeat [True, True])
    putStrLn (firstRepeat "ab")
    putStrLn (biggest [3, 1, 2 :: Int])
    addOne (return (Just 41)) >>= print
