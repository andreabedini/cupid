-- Applicative is a superclass of Monad, as in GHC's base: a Monad
-- instance needs Functor and Applicative instances, return defaults to
-- pure, and <$>, <*>, *>, <*, <$ and liftA2 come from the Prelude.
module Main where

newtype State s a = State { runState :: s -> (a, s) }

instance Functor (State s) where
    fmap f (State g) = State $ \s -> let (a, s') = g s in (f a, s')

instance Applicative (State s) where
    pure a = State $ \s -> (a, s)
    State f <*> State g = State $ \s ->
        let (h, s') = f s
            (a, s'') = g s'
        in (h a, s'')

-- No return: it is pure.
instance Monad (State s) where
    State g >>= k = State $ \s -> let (a, s') = g s in runState (k a) s'

get :: State s s
get = State $ \s -> (s, s)

put :: s -> State s ()
put s = State $ \_ -> ((), s)

tick :: State Int Int
tick = do
    n <- get
    put (n + 1)
    return n

-- A tuple pattern cannot fail, so this needs no MonadFail instance.
pairUp :: State Int (Int, Int)
pairUp = do
    (a, b) <- (,) <$> tick <*> tick
    pure (b, a)

main :: IO ()
main = do
    print (runState (tick *> tick *> tick) 0)
    print (runState (tick <* tick) 10)
    print (runState (liftA2 (+) tick tick) 5)
    print (runState pairUp 1)
    print (fst (runState ("done" <$ tick) 0))
    print ((+) <$> Just 1 <*> Just 2, Just 1 *> Nothing :: Maybe Int)
    print (pure 3 :: [Int], [(+ 1), (* 2)] <*> [10, 20 :: Int])
    print (((+) <$> (* 2) <*> (+ 10)) (3 :: Int))
    print (Right 1 >>= \x -> if x > 0 then Left "positive" else Right (x :: Int))
