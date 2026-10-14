-- Word's Bits and FiniteBits instances come with Data.Bits, so the
-- Prelude's Word has them without importing Data.Word.
module Main where

import Data.Bits

main :: IO ()
main = print (popCount (7 :: Word), (5 :: Word) .&. 3, shiftL (1 :: Word) 4, testBit (2 :: Word) 1)
