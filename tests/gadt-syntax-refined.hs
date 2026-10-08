-- A constructor whose result refines the type's arguments is a GADT
-- proper, which is not supported.
data Expr a where
    IntE  :: Int -> Expr Int
    BoolE :: Bool -> Expr Bool

main :: IO ()
main = return ()
