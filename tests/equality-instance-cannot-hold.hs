-- A wanted equality from an instance's context that cannot hold, reported
-- at the instance.
class Describe a where
    describe :: a -> String

instance (a ~ Char) => Describe [a] where
    describe s = "string " ++ s

main :: IO ()
main = putStrLn (describe [True])
