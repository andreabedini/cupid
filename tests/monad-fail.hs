-- fail is the method of MonadFail, not of Monad.  A do block calls it
-- only when a pattern can fail to match, and then needs a MonadFail
-- instance; Maybe, lists and IO have one.  Control.Monad.Fail exports
-- the class too.
module Main where

import qualified Control.Monad.Fail as Fail
import System.IO.Error (catchIOError)

firsts :: [Maybe Int] -> [Int]
firsts xs = do
    Just x <- xs
    return x

pairs :: Maybe [Int]
pairs = do
    (x:_) <- Just []
    return [x]

failIO :: IO Int
failIO = do
    [x] <- return [1, 2]
    return x

newtype Parser a = Parser { runParser :: String -> Maybe (a, String) }

instance Functor Parser where
    fmap f (Parser p) = Parser $ \s -> fmap (\(a, r) -> (f a, r)) (p s)

instance Applicative Parser where
    pure a = Parser $ \s -> Just (a, s)
    Parser pf <*> Parser pa = Parser $ \s -> case pf s of
        Nothing -> Nothing
        Just (f, r) -> fmap (\(a, r') -> (f a, r')) (pa r)

instance Monad Parser where
    Parser p >>= k = Parser $ \s -> p s >>= \(a, r) -> runParser (k a) r

instance Fail.MonadFail Parser where
    fail _ = Parser (const Nothing)

item :: Parser Char
item = Parser $ \s -> case s of
    (c:cs) -> Just (c, cs)
    [] -> Nothing

digit :: Parser Char
digit = do
    c <- item
    if c `elem` "0123456789" then return c else fail "not a digit"

main :: IO ()
main = do
    print (firsts [Just 1, Nothing, Just 3])
    print pairs
    r <- fmap show failIO `catchIOError` \e -> return ("IO failed: " ++ show e)
    putStrLn r
    print (runParser digit "7up", runParser digit "up")
