-- An equality cannot be a superclass.
class (a ~ Int) => C a where
    c :: a -> Int

main :: IO ()
main = return ()
