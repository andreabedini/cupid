-- A kind given on a binder that the constructors contradict: f is used
-- applied, so it cannot have kind *.
data T (f :: *) = MkT (f Int)

main :: IO ()
main = return ()
