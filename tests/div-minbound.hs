-- (minBound::Int) `div` (-1) used to raise SIGFPE and kill the process
-- (and likewise for quot, quotRem and divMod); rem and mod don't overflow
-- and should just give 0.
import Control.Exception

main :: IO ()
main = do
  r1 <- try (evaluate ((minBound::Int) `quot` (-1))) :: IO (Either SomeException Int)
  print r1
  r2 <- try (evaluate ((minBound::Int) `div` (-1))) :: IO (Either SomeException Int)
  print r2
  print ((minBound::Int) `rem` (-1))
  print ((minBound::Int) `mod` (-1))
  r3 <- try (evaluate ((minBound::Int) `quotRem` (-1))) :: IO (Either SomeException (Int,Int))
  print r3
  r4 <- try (evaluate ((minBound::Int) `divMod` (-1))) :: IO (Either SomeException (Int,Int))
  print r4
  -- ordinary cases must still work
  print ((7::Int) `div` 2, (7::Int) `mod` 2, (7::Int) `quot` 2, (7::Int) `rem` 2)
  print (((-7)::Int) `div` 2, ((-7)::Int) `mod` 2, ((-7)::Int) `quot` 2, ((-7)::Int) `rem` 2)
