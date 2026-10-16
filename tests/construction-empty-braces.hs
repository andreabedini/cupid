-- C {} is legal for a constructor without strict fields, whether or not it
-- was declared with field labels: the fields are left undefined.  The
-- strict case is construction-strict-unlabelled.
data U = U Int Bool
data R = R { f :: Int, g :: Bool }
data N = N

main :: IO ()
main = do
    case U {} of
        U _ _ -> putStrLn "unlabelled"
    case R {} of
        R _ _ -> putStrLn "labelled"
    case N {} of
        N -> putStrLn "nullary"
