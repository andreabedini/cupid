-- Data.Functor.Classes lifts Eq, Ord, Read and Show to type
-- constructors, as monad transformer libraries need; with it come
-- Control.Monad.IO.Class, Data.Functor.Contravariant, Data.Bifoldable,
-- Data.Bitraversable and Data.Foldable1.
module Main where

import Control.Monad.IO.Class
import Data.Bifoldable
import Data.Bitraversable
import qualified Data.Foldable1 as F1
import Data.Functor.Classes
import Data.Functor.Compose
import Data.Functor.Contravariant
import Data.Functor.Identity
import Data.List.NonEmpty (NonEmpty(..))

-- A transformer-like wrapper whose instances come from the lifted classes.
newtype Wrap f a = Wrap (f a)

instance (Eq1 f, Eq a) => Eq (Wrap f a) where
    Wrap x == Wrap y = eq1 x y

instance (Ord1 f, Ord a) => Ord (Wrap f a) where
    compare (Wrap x) (Wrap y) = compare1 x y

instance (Show1 f, Show a) => Show (Wrap f a) where
    showsPrec d (Wrap x) = showsUnaryWith showsPrec1 "Wrap" d x

instance (Read1 f, Read a) => Read (Wrap f a) where
    readsPrec = readsData (readsUnaryWith readsPrec1 "Wrap" Wrap)

newtype App a = App { runApp :: IO a }

instance Functor App where fmap f (App m) = App (fmap f m)
instance Applicative App where
    pure = App . pure
    App f <*> App x = App (f <*> x)
instance Monad App where App m >>= k = App (m >>= runApp . k)
instance MonadIO App where liftIO = App

main :: IO ()
main = do
    print (Wrap (Just 1) == Wrap (Just 1), Wrap [1, 2] < Wrap [1, 3 :: Int])
    print (Wrap (Just (-3 :: Int)), Wrap [Left 'a', Right True])
    print (read "Wrap (Just 5)" :: Wrap Maybe Int, read "Wrap [1,2]" :: Wrap [] Int)
    print (Wrap (Compose (Just [1 :: Int])), Wrap (Identity 'x'), Wrap (1 :| [2 :: Int]))
    print (liftCompare compare (Just 1) Nothing, liftEq (==) "ab" "ab")
    print (getPredicate (contramap length (Predicate even)) "four", getOp (contramap show (Op length)) (12345 :: Int))
    print (bifoldMap show (: []) (1 :: Int, 'c'), bisum (3, 4), bilength (Left 'x' :: Either Char Int))
    r <- bitraverse (\x -> print x >> return (x + 1)) (return . show) (1 :: Int, True)
    print r
    print (F1.fold1 ("ab" :| ["cd"]), F1.head (5 :| [6 :: Int]), F1.intercalate1 ", " ("x" :| ["y", "z"]))
    runApp (liftIO (putStrLn "lifted into App"))
