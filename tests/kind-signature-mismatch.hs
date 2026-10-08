-- A standalone kind signature that does not fit the declaration.
type T :: * -> *
data T a b = MkT a b

main :: IO ()
main = return ()
