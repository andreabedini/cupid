-- Only the constraint the signature does not give is reported: Show of
-- the pair is reduced by its instances to Show of each component, and
-- the one for the given type variable is discharged.
k :: Show b => b -> a -> String
k y x = show (y, [x])

main :: IO ()
main = putStrLn (k 'y' 'x')
