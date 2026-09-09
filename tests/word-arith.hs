-- Word is unsigned, so div/quot and mod/rem coincide: primDivWord and
-- primQuotWord are one C function, and so are primModWord and primRemWord.
-- Check that where a signed implementation would diverge, with the high bit
-- set.  Also the two operations that used to reach for undefined behaviour:
-- negate on 0x80000000, and bit at and past the word size.
import Data.Bits
import Data.Word

main :: IO ()
main = do
  let hi = 2147483648 :: Word -- 0x80000000
      mx = maxBound :: Word   -- 0xffffffff
  print (mx, hi)
  print (negate hi, negate (0 :: Word), negate (1 :: Word))
  print (bit 0 :: Word, bit 31 :: Word, bit 32 :: Word, bit 64 :: Word)
  print (mx `div` 7, mx `quot` 7, mx `mod` 7, mx `rem` 7)
  print (hi `div` 3, hi `quot` 3, hi `mod` 3, hi `rem` 3)
  print (map (\(a, b) -> a `div` b == a `quot` b && a `mod` b == a `rem` b)
             [(mx, 3), (hi, 7), (hi, mx), (mx, hi), (1, mx)])
