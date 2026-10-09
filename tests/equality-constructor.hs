-- An equality in a constructor's context would make a GADT; rejected
-- until the solver has given equalities.
data T a where
    C :: (a ~ Int) => Int -> T a

main :: IO ()
main = return ()
