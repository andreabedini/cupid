-- C {} is legal for any constructor without strict fields, whether or not
-- it was declared with field labels, and a compile-time error when one is
-- strict.  For a constructor declared without labels the strict field has
-- no selector to name, and looking for one was an internal error.
data T = C Int !Bool

main :: IO ()
main = case C {} of
    C _ _ -> putStrLn "not reached"
