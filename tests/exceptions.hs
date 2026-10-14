-- Control.Exception has the Exception class of GHC's base: an empty
-- instance makes a type an exception, catch and try take the type of the
-- exception to handle and let others pass, and the interpreter's own
-- errors are caught as ErrorCall, ArithException and the rest.
module Main where

import Control.Exception
import Data.IORef

data BankError = InsufficientFunds Int | AccountClosed
    deriving Show

instance Exception BankError

newtype Other = Other String deriving Show

instance Exception Other where
    displayException (Other s) = "other: " ++ s

withdraw :: Int -> Int -> IO Int
withdraw balance amount
    | amount > balance = throwIO (InsufficientFunds (amount - balance))
    | otherwise = return (balance - amount)

main :: IO ()
main = do
    r <- try (withdraw 10 25)
    print (r :: Either BankError Int)
    withdraw 10 3 >>= print
    catch (throwIO AccountClosed) (\e -> putStrLn ("caught " ++ show (e :: BankError)))
    -- A handler for one type lets another through to an outer one.
    catch (catch (throwIO (Other "x")) (\e -> print (e :: BankError)))
          (\e -> putStrLn ("outer: " ++ displayException (e :: Other)))
    -- The interpreter's errors are exceptions too.
    r1 <- try (evaluate (error "boom" :: Int))
    print (r1 :: Either ErrorCall Int)
    r2 <- try (evaluate (1 `div` (0 :: Int)))
    print (r2 :: Either ArithException Int)
    r3 <- try (evaluate (head ([] :: [Int])))
    putStrLn (either (\e -> "SomeException: " ++ show (e :: SomeException)) show r3)
    -- throw from pure code, and fromException through SomeException.
    r4 <- try (evaluate (if True then throw (InsufficientFunds 1) else 0 :: Int))
    case r4 of
        Left se -> print (fromException se :: Maybe BankError, fromException se :: Maybe Other)
        Right _ -> putStrLn "no exception"
    -- catches picks the first handler of the right type.
    let handlers = [Handler (\e -> return ("arith " ++ show (e :: ArithException))),
                    Handler (\e -> return ("bank " ++ show (e :: BankError)))]
    catches (throwIO AccountClosed) handlers >>= putStrLn
    catches (evaluate (1 `mod` (0 :: Int)) >> return "") handlers >>= putStrLn
    -- bracket and finally run the release however the use ends.
    log <- newIORef []
    let note s = modifyIORef log (s :)
    _ <- try (bracket (note "acquire" >> return 7)
                      (\_ -> note "release")
                      (\x -> note ("use " ++ show x) >> throwIO AccountClosed)) :: IO (Either BankError ())
    (note "body" `finally` note "finally") `onException` note "not run"
    readIORef log >>= print . reverse
    -- An IOError is an IOException.
    r5 <- try (ioError (userError "disk full"))
    print (r5 :: Either IOException ())
    -- handle, catchJust and mapException.
    handle (\e -> putStrLn ("handled " ++ show (e :: ErrorCall))) (evaluate (error "late") >> return ())
    catchJust (\e -> case e of InsufficientFunds n -> Just n; _ -> Nothing)
              (throwIO (InsufficientFunds 42)) print
    r6 <- try (evaluate (mapException (\(ErrorCall s) -> Other s) (error "mapped" :: Int)))
    print (r6 :: Either Other Int)
