-- Bit indices and shift counts outside the width of the type, which the
-- primitives must not pass to C's shifts: testBit is False there, a shift
-- by minBound must not overflow when negated, and rotating by a multiple
-- of the width is the identity.  Pointer arithmetic on nullPtr is done on
-- the address, not as C pointer arithmetic.
module Main where

import Data.Bits
import Foreign.Ptr

main :: IO ()
main = do
    print (testBit (1 :: Int) 32, testBit (1 :: Int) (-1), testBit (minBound :: Int) 31)
    print (testBit (1 :: Word) 32, testBit (1 :: Word) (-1), testBit (maxBound :: Word) 31)
    print (shift (1 :: Int) minBound, shift (-1 :: Int) minBound, shift (-8 :: Int) (-1))
    print (shift (1 :: Word) minBound, shift (8 :: Word) (-1))
    print (rotate (1 :: Word) 0, rotate (1 :: Word) 32, rotate (1 :: Word) (-32))
    print (rotate (1 :: Word) 1, rotate (1 :: Word) (-1))
    print ((nullPtr `plusPtr` 3) `minusPtr` (nullPtr :: Ptr ()))
