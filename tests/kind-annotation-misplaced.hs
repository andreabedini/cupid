-- A kind annotation anywhere but a declaration head is rejected.
f :: (a :: *) -> a
f x = x

main :: IO ()
main = return ()
