-- toInteger of a minBound Int, and anything built from it (like
-- minBound::Int64), must come out right, so bigInt must not negate INT_MIN
-- in Int arithmetic (undefined behaviour) before widening to the bignum's
-- unsigned long digits: toInteger (minBound::Int) - 1 must print -2147483649,
-- not -18446744071562067969.
import Data.Int
import Data.Bits

main :: IO ()
main = do
  print (toInteger (minBound :: Int))
  print (toInteger (minBound :: Int) - 1)
  print (minBound :: Int64)
  print (shiftL (1 :: Int64) 63)
