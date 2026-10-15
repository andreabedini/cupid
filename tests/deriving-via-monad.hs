-- Deriving a class whose parameter is a type constructor, such as Monad, via
-- a type: the instance is for the newtype applied to all but its last
-- parameter, Monad Parser and Monad (Lifting t m).
module Main where
import Data.Kind (Type)

newtype StateT s m a = StateT { runStateT :: s -> m (a, s) }
instance Functor m => Functor (StateT s m) where
    fmap f (StateT g) = StateT (fmap (\(a, s) -> (f a, s)) . g)
instance Monad m => Applicative (StateT s m) where
    pure a = StateT (\s -> return (a, s))
    StateT f <*> StateT g = StateT (\s -> do { (h, s') <- f s; (a, s'') <- g s'; return (h a, s'') })
instance Monad m => Monad (StateT s m) where
    StateT g >>= k = StateT (\s -> g s >>= \(a, s') -> runStateT (k a) s')

modify' :: Monad m => (s -> s) -> StateT s m ()
modify' f = StateT (\s -> return ((), f s))
get' :: Monad m => StateT s m s
get' = StateT (\s -> return (s, s))

newtype Parser a = Parser (StateT String Maybe a)
    deriving (Functor, Applicative, Monad) via StateT String Maybe

newtype Lifting (t :: (Type -> Type) -> Type -> Type) (m :: Type -> Type) a
    = Lifting (t m a)
    deriving (Functor, Applicative, Monad) via (t m)

item :: Parser Char
item = Parser (StateT (\s -> case s of { c:cs -> Just (c, cs); [] -> Nothing }))

runParser :: Parser a -> String -> Maybe (a, String)
runParser (Parser p) = runStateT p

lifted :: Lifting (StateT Int) Maybe Int
lifted = Lifting (modify' (+1)) >> Lifting get' >>= \n -> return (n * 10)

main :: IO ()
main = do
    print (runParser ((,) <$> item <*> (item >> item)) "abcd")
    print (case lifted of Lifting s -> runStateT s 4)
