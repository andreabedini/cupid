-- An equality is a constraint, not a type.
f :: (a ~ Int) -> Int
f _ = 0

main :: IO ()
main = return ()
