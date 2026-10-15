-- Standalone deriving declarations, with each strategy: stock (with or
-- without the word), newtype, via a type and anyclass.  A via type stands
-- for the class's last parameter, and a newtype applied to fewer than its
-- parameters is the same at run time as what it wraps when that ends in
-- them, so App, Wrap (Logger String) and Logger String are all one.
module Main where
import Data.Monoid (Sum(..))
import Data.Typeable
import Data.Data

data T a = Leaf | Node (T a) a (T a)
deriving instance Show a => Show (T a)
deriving instance Eq a => Eq (T a)
deriving stock instance Ord a => Ord (T a)

data Colour = Red | Green | Blue
deriving instance Show Colour
deriving instance Enum Colour
deriving instance Bounded Colour
deriving instance Typeable Colour
deriving instance Data Colour

newtype Age = Age Int
deriving newtype instance Show Age
deriving newtype instance Eq Age
deriving newtype instance Num Age

newtype Score = Score Int
deriving via (Sum Int) instance Semigroup Score
deriving via Sum Int instance Monoid Score
deriving instance Show Score

class Describe a where
    describe :: a -> String
    describe _ = "something"
deriving anyclass instance Describe Colour

-- a class of two parameters, derived via a type for its last
class Monad m => MonadLog w m | m -> w where
    logIt :: w -> m ()

newtype Logger w a = Logger (w -> ([w], a))
instance Functor (Logger w) where fmap f (Logger g) = Logger (fmap f . g)
instance Applicative (Logger w) where
    pure x = Logger (const ([], x))
    Logger f <*> Logger g = Logger (\e -> let (a, h) = f e; (b, x) = g e in (a ++ b, h x))
instance Monad (Logger w) where
    Logger g >>= k = Logger (\e -> let (a, x) = g e; Logger h = k x; (b, y) = h e in (a ++ b, y))
instance MonadLog w (Logger w) where
    logIt w = Logger (const ([w], ()))

newtype Wrap f a = Wrap (f a)
instance Functor f => Functor (Wrap f) where fmap f (Wrap x) = Wrap (fmap f x)
instance Applicative f => Applicative (Wrap f) where
    pure = Wrap . pure
    Wrap f <*> Wrap x = Wrap (f <*> x)
instance Monad f => Monad (Wrap f) where Wrap x >>= k = Wrap (x >>= \a -> let Wrap y = k a in y)
instance MonadLog w m => MonadLog w (Wrap m) where logIt = Wrap . logIt

newtype App a = App (Logger String a)
instance Functor App where fmap f (App x) = App (fmap f x)
instance Applicative App where
    pure = App . pure
    App f <*> App x = App (f <*> x)
instance Monad App where App x >>= k = App (x >>= \a -> let App y = k a in y)
deriving via (Wrap (Logger String)) instance MonadLog String App

runApp :: App a -> ([String], a)
runApp (App (Logger g)) = g ""

main :: IO ()
main = do
    print (Node Leaf 1 (Node Leaf 2 Leaf))
    print (Node Leaf 'a' Leaf == Node Leaf 'a' Leaf, compare Leaf (Node Leaf 'x' Leaf))
    print [minBound .. maxBound :: Colour]
    print (typeOf Green, toConstr Blue)
    print (Age 3 + Age 4)
    print (Score 1 <> Score 2, mempty :: Score)
    putStrLn (describe Red)
    print (runApp (logIt "one" >> logIt "two" >> return 3))
