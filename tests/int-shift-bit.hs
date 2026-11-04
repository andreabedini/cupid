-- shiftL/shiftR/bit on Int must check the range: shifting by >= the bit width
-- must give 0 (or -1, for an arithmetic right shift of a negative number), as
-- a fixed-width Int should, not nonsense such as shiftL 1 32 == 1 or
-- shiftL 1 40 == 256.
import Data.Bits
import Data.Int

main :: IO ()
main = do
  print (shiftL (1::Int) 0, shiftL (1::Int) 31, shiftL (1::Int) 32, shiftL (1::Int) 40)
  print (shiftR ((-1)::Int) 0, shiftR ((-1)::Int) 32, shiftR ((-1)::Int) 40)
  print (shiftR (16::Int) 32, shiftR (16::Int) 40)
  print (shiftL ((-1)::Int) 1, shiftR (minBound::Int) 31)
  print (bit 0 :: Int, bit 31 :: Int, bit 32 :: Int, bit (-1) :: Int)
  print (shiftL (1::Int64) 63)
