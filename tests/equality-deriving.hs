-- Deriving cannot use an instance whose context has an equality.
data T a = T a

instance (a ~ Int) => Show (T a) where
    show (T n) = show n

data U a = U (T a) deriving Show

main :: IO ()
main = print (U (T 1))
