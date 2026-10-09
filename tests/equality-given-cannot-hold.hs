-- A given equality that cannot hold is reported at the signature.
f :: (Int ~ Bool) => Int
f = 3

main :: IO ()
main = print f
