import Control.Exception (try, evaluate, SomeException)

count :: Int -> IO Int
count 0 = return 0
count n = do
    r <- try (evaluate (error "boom")) :: IO (Either SomeException Int)
    case r of
        Left _  -> do
            rest <- count (n - 1)
            return (rest + 1)
        Right _ -> count (n - 1)

main :: IO ()
main = do
    n <- count 5000
    print n
