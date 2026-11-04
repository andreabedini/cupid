-- C {} is legal for any constructor without strict fields, whether or not
-- it was declared with field labels, and a compile-time error when one is
-- strict.  For a constructor declared without labels the strict field has
-- no selector to name, and the error must not look for one.
data T = C Int !Bool

main :: IO ()
main = case C {} of
    C _ _ -> putStrLn "not reached"
