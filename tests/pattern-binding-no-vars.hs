-- A top-level pattern binding may bind no variables at all.  The Report
-- accepts such a binding and gives it no effect.  When its pattern is a
-- numeric literal it leaves a Num constraint to default, and the defaulting
-- step must not read a first variable that the binding does not have.

1 = 2
(3) = 4
1.5 = 2.5
(5, 'x') = (6, 'y')
True = False

main :: IO ()
main = putStrLn "loaded"
