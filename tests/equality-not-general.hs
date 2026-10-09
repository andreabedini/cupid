-- The body is checked with the given applied, and must still be as
-- general as the declared type read through it: here b -> b, not
-- Int -> Int.
f :: (a ~ Int) => a -> b -> b
f x y = x

main :: IO ()
main = print (f 1 2)
