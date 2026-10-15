-- deriving newtype for a class of type constructors, such as Monad, derives
-- via what the newtype wraps less the parameters the class leaves off:
-- App m via StateT Int m, Wrap via Maybe.
module Main where

newtype StateT s m a = StateT { runStateT :: s -> m (a, s) }

instance Functor m => Functor (StateT s m) where
    fmap f (StateT g) = StateT (\s -> fmap (\(a, s') -> (f a, s')) (g s))

instance Monad m => Applicative (StateT s m) where
    pure a = StateT (\s -> return (a, s))
    StateT mf <*> StateT mx =
        StateT (\s -> mf s >>= \(f, s') -> mx s' >>= \(x, s'') -> return (f x, s''))

instance Monad m => Monad (StateT s m) where
    StateT m >>= k = StateT (\s -> m s >>= \(a, s') -> runStateT (k a) s')

newtype App m a = App (StateT Int m a)
    deriving newtype (Functor, Applicative, Monad)

newtype Wrap a = Wrap (Maybe a)
    deriving newtype (Functor, Applicative, Monad, Show)

tick :: Monad m => App m Int
tick = App (StateT (\n -> return (n, n + 1)))

runApp :: App m a -> Int -> m (a, Int)
runApp (App m) = runStateT m

main :: IO ()
main = do
    r <- runApp (do { a <- tick; b <- tick; return (a + b) }) 10
    print r
    print (fmap (+ 1) (Wrap (Just 1)), Wrap (Just 2) >>= \x -> Wrap (Just (x * 3)))
