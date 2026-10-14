-- The Prelude exports Word with its instances, and Data.Bits has the
-- FiniteBits class and the Bits methods and functions of GHC's base.
module Main where

import Data.Bits
import Data.Int
import Data.Word

main :: IO ()
main = do
    print (7 `div` (2 :: Word), [1 .. 3 :: Word], read "42" :: Word, fromIntegral (3 :: Word) + (1 :: Int))
    print (popCount (255 :: Int), popCount (-1 :: Integer), popCount (6 :: Integer))
    print (finiteBitSize (0 :: Word8), finiteBitSize (0 :: Int16), finiteBitSize (0 :: Word64))
    print (countLeadingZeros (1 :: Word16), countTrailingZeros (8 :: Int32), countTrailingZeros (0 :: Word8))
    print (zeroBits :: Int, oneBits :: Word8, bitSizeMaybe (0 :: Integer), bitSizeMaybe (0 :: Int8))
    print (toIntegralSized (300 :: Int) :: Maybe Word8, toIntegralSized (200 :: Int) :: Maybe Word8)
    print ((5 :: Int) .<<. 2, (20 :: Int) .>>. 2, (6 :: Int) .^. 3, unsafeShiftL (1 :: Word32) 4)
