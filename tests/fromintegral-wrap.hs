-- fromIntegral from an out-of-range Integer to a fixed-width type must wrap
-- (taking the low bits), not raise "arithmetic overflow": that is what
-- Haskell's Int8/16/32/64 and Word8/16/32/64 are defined to do.
import Data.Int
import Data.Word

main :: IO ()
main = do
  print (fromIntegral (2^40 :: Integer) :: Int)
  print (fromIntegral (2^40 :: Integer) :: Int32)
  print (fromIntegral (2^40 :: Integer) :: Int16)
  print (fromIntegral (2^40 :: Integer) :: Int8)
  print (fromIntegral (2^40 :: Integer) :: Word32)
  print (fromIntegral (2^40 :: Integer) :: Word16)
  print (fromIntegral (2^40 :: Integer) :: Word8)
  print (fromIntegral ((-1) :: Integer) :: Word32)
  print (fromIntegral (2^32 + 5 :: Integer) :: Int)
