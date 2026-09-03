-- bigInt negated INT_MIN in Int arithmetic (undefined behaviour) before
-- widening to the bignum's unsigned long digits, so toInteger of a
-- minBound Int, and anything built from it (like minBound::Int64), came
-- out wrong (e.g. toInteger (minBound::Int) - 1 printed
-- -18446744071562067969 instead of -2147483649).
import Data.Int
import Data.Bits

main :: IO ()
main = do
  print (toInteger (minBound :: Int))
  print (toInteger (minBound :: Int) - 1)
  print (minBound :: Int64)
  print (shiftL (1 :: Int64) 63)
