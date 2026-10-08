-- A constructor must build the type being declared.
data T a where
    C :: a -> Maybe a

main :: IO ()
main = return ()
