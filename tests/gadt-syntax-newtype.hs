-- A newtype in GADT syntax still has exactly one constructor.
newtype N a where
    A :: a -> N a
    B :: a -> N a

main :: IO ()
main = return ()
