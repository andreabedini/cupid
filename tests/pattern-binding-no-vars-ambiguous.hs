-- A top-level pattern binding that binds no variables, with a constraint
-- that cannot be defaulted, is reported as unresolved overloading.  With
-- no variable to name, the report names the pattern.

True = mempty == mempty

main :: IO ()
main = putStrLn "not reached"
