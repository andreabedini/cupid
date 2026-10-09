-- The two sides of an equality must have one kind.
f :: (Int ~ Maybe) => a -> Int
f _ = 0

main :: IO ()
main = return ()
