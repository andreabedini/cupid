-- Int's +, -, *, negate and the lexer's integer-literal reader relied on
-- signed int overflow (undefined behaviour in C) to wrap the way Haskell's
-- fixed-width Int is defined to.  Check the wraparound is still what
-- GHC gives, and that a literal too large for Int still promotes to
-- Integer correctly (exercising the lexer's overflow path).
main :: IO ()
main = do
  print (maxBound + 1 == (minBound :: Int))
  print (minBound - 1 == (maxBound :: Int))
  print ((maxBound :: Int) * 2)
  print (negate (minBound :: Int) == minBound)
  print (negate (maxBound :: Int))
  -- ordinary arithmetic is unaffected
  print (3 + 4 :: Int, 10 - 3 :: Int, 6 * 7 :: Int, negate 5 :: Int)
  -- a literal too large for Int must still promote to Integer
  print (12345678901234567890 :: Integer)
  print (99999999999999999999999999999999999999999999999999 :: Integer)
