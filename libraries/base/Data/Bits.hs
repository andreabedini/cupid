{-# OPTIONS_GHC -fno-implicit-prelude #-}
-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Bits
-- Copyright   :  (c) The University of Glasgow 2001
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  libraries@haskell.org
-- Stability   :  experimental
-- Portability :  portable
--
--  This module defines bitwise operations for signed and unsigned
--  integers.  Instances of the class 'Bits' for the 'Int' and
--  'Integer' types are available from this module, and instances for
--  explicitly sized integral types are available from the
--  "Data.Int" and "Data.Word" modules.
--
-----------------------------------------------------------------------------

module Data.Bits ( 
  Bits(
    (.&.), (.|.), xor, -- :: a -> a -> a
    complement,        -- :: a -> a
    shift,             -- :: a -> Int -> a
    rotate,            -- :: a -> Int -> a
    bit,               -- :: Int -> a
    setBit,            -- :: a -> Int -> a
    clearBit,          -- :: a -> Int -> a
    complementBit,     -- :: a -> Int -> a
    testBit,           -- :: a -> Int -> Bool
    bitSize,           -- :: a -> Int
    isSigned,          -- :: a -> Bool
    shiftL, shiftR,    -- :: a -> Int -> a
    rotateL, rotateR,  -- :: a -> Int -> a
    zeroBits,          -- :: a
    bitSizeMaybe,      -- :: a -> Maybe Int
    unsafeShiftL, unsafeShiftR, -- :: a -> Int -> a
    popCount           -- :: a -> Int
  ),
  FiniteBits(
    finiteBitSize,     -- :: a -> Int
    countLeadingZeros, -- :: a -> Int
    countTrailingZeros -- :: a -> Int
  ),
  bitDefault, testBitDefault, popCountDefault, toIntegralSized,
  oneBits, (.^.), (.>>.), (.<<.), (!>>.), (!<<.)
 ) where

-- Defines the @Bits@ class containing bit-based operations.
-- See library document for details on the semantics of the
-- individual operations.


                                                                                                                                                                                                        

                                                                                                                                                                                                                                                                                    



                                                                                                                                                                                                                                                                                                                                                       







                                                      












                                                         






























                                                         

















































































import Cupid.Bits


infixl 8 `shift`, `rotate`, `shiftL`, `shiftR`, `rotateL`, `rotateR`
infixl 8 `unsafeShiftL`, `unsafeShiftR`, .>>., .<<., !>>., !<<.
infixl 6 .^.
infixl 7 .&.
infixl 6 `xor`
infixl 5 .|.

{-| 
The 'Bits' class defines bitwise operations over integral types.

* Bits are numbered from 0 with bit 0 being the least
  significant bit.

Minimal complete definition: '.&.', '.|.', 'xor', 'complement',
('shift' or ('shiftL' and 'shiftR')), ('rotate' or ('rotateL' and 'rotateR')),
'bitSize' and 'isSigned'.
-}
class Num a => Bits a where
    -- | Bitwise \"and\"
    (.&.) :: a -> a -> a

    -- | Bitwise \"or\"
    (.|.) :: a -> a -> a

    -- | Bitwise \"xor\"
    xor :: a -> a -> a

    {-| Reverse all the bits in the argument -}
    complement        :: a -> a

    {-| Shift the argument left by the specified number of bits.
	Right shifts (signed) are specified by giving a negative value.

	An instance can define either this unified 'shift' or 'shiftL' and
	'shiftR', depending on which is more convenient for the type in
	question. -}
    shift             :: a -> Int -> a

    x `shift`   i | i<0  = x `shiftR` (-i)
                  | i==0 = x
                  | i>0  = x `shiftL` i

    {-| Rotate the argument left by the specified number of bits.
	Right rotates are specified by giving a negative value.

        For unbounded types like 'Integer', 'rotate' is equivalent to 'shift'.

	An instance can define either this unified 'rotate' or 'rotateL' and
	'rotateR', depending on which is more convenient for the type in
	question. -}
    rotate            :: a -> Int -> a

    x `rotate`  i | i<0  = x `rotateR` (-i)
                  | i==0 = x
                  | i>0  = x `rotateL` i

    {-
    -- Rotation can be implemented in terms of two shifts, but care is
    -- needed for negative values.  This suggested implementation assumes
    -- 2's-complement arithmetic.  It is commented out because it would
    -- require an extra context (Ord a) on the signature of 'rotate'.
    x `rotate`  i | i<0 && isSigned x && x<0
                         = let left = i+bitSize x in
                           ((x `shift` i) .&. complement ((-1) `shift` left))
                           .|. (x `shift` left)
                  | i<0  = (x `shift` i) .|. (x `shift` (i+bitSize x))
                  | i==0 = x
                  | i>0  = (x `shift` i) .|. (x `shift` (i-bitSize x))
    -}

    -- | @bit i@ is a value with the @i@th bit set
    bit               :: Int -> a

    -- | @x \`setBit\` i@ is the same as @x .|. bit i@
    setBit            :: a -> Int -> a

    -- | @x \`clearBit\` i@ is the same as @x .&. complement (bit i)@
    clearBit          :: a -> Int -> a

    -- | @x \`complementBit\` i@ is the same as @x \`xor\` bit i@
    complementBit     :: a -> Int -> a

    -- | Return 'True' if the @n@th bit of the argument is 1
    testBit           :: a -> Int -> Bool

    {-| Return the number of bits in the type of the argument.  The actual
	value of the argument is ignored.  The function 'bitSize' is
	undefined for types that do not have a fixed bitsize, like 'Integer'.
	-}
    bitSize           :: a -> Int

    {-| Return 'True' if the argument is a signed type.  The actual
        value of the argument is ignored -}
    isSigned          :: a -> Bool

    bit i               = 1 `shiftL` i
    x `setBit` i        = x .|. bit i
    x `clearBit` i      = x .&. complement (bit i)
    x `complementBit` i = x `xor` bit i
    x `testBit` i       = (x .&. bit i) /= 0

    {-| Shift the argument left by the specified number of bits
	(which must be non-negative).

	An instance can define either this and 'shiftR' or the unified
	'shift', depending on which is more convenient for the type in
	question. -}
    shiftL            :: a -> Int -> a
    x `shiftL`  i = x `shift`  i

    {-| Shift the argument right (signed) by the specified number of bits
	(which must be non-negative).

	An instance can define either this and 'shiftL' or the unified
	'shift', depending on which is more convenient for the type in
	question. -}
    shiftR            :: a -> Int -> a
    x `shiftR`  i = x `shift`  (-i)

    {-| Rotate the argument left by the specified number of bits
	(which must be non-negative).

	An instance can define either this and 'rotateR' or the unified
	'rotate', depending on which is more convenient for the type in
	question. -}
    rotateL           :: a -> Int -> a
    x `rotateL` i = x `rotate` i

    {-| Rotate the argument right by the specified number of bits
	(which must be non-negative).

	An instance can define either this and 'rotateL' or the unified
	'rotate', depending on which is more convenient for the type in
	question. -}
    rotateR           :: a -> Int -> a
    x `rotateR` i = x `rotate` (-i)

    -- | The value with no bits set.
    zeroBits          :: a
    zeroBits          = clearBit (bit 0) 0

    -- | The number of bits in the type, or 'Nothing' if it has no fixed
    -- size.
    bitSizeMaybe      :: a -> Maybe Int
    bitSizeMaybe x    = Just (bitSize x)

    -- | 'shiftL' for a count that is non-negative and less than the size.
    unsafeShiftL      :: a -> Int -> a
    unsafeShiftL      = shiftL

    -- | 'shiftR' for a count that is non-negative and less than the size.
    unsafeShiftR      :: a -> Int -> a
    unsafeShiftR      = shiftR

    -- | The number of set bits; for a negative number of a type with no
    -- fixed size, minus the number of set bits of its negation.
    popCount          :: a -> Int
    popCount          = popCountDefault

instance Bits Int where



















    (.&.)                  = primAndInt
    (.|.)                  = primOrInt
    xor                    = primXorInt
    complement             = primComplementInt
    shift                  = primShiftInt
    bit                    = primBitInt
    testBit                = primTestInt
    bitSize _              = 4*8










    x `rotate`  i
	| i<0 && x<0       = let left = i+bitSize x in
                             ((x `shift` i) .&. complement ((-1) `shift` left))
                             .|. (x `shift` left)
	| i<0              = (x `shift` i) .|. (x `shift` (i+bitSize x))
	| i==0             = x
	| i>0              = (x `shift` i) .|. (x `shift` (i-bitSize x))



    isSigned _             = True










instance Bits Integer where

























   -- reduce bitwise binary operations to special cases we can handle

   x .&. y   | x<0 && y<0 = complement (complement x `posOr` complement y)
	     | otherwise  = x `posAnd` y
   
   x .|. y   | x<0 || y<0 = complement (complement x `posAnd` complement y)
	     | otherwise  = x `posOr` y
   
   x `xor` y | x<0 && y<0 = complement x `posXOr` complement y
	     | x<0        = complement (complement x `posXOr` y)
	     |        y<0 = complement (x `posXOr` complement y)
	     | otherwise  = x `posXOr` y

   -- assuming infinite 2's-complement arithmetic
   complement a = -1 - a


   shift x i | i >= 0    = x * 2^i
	     | otherwise = x `div` 2^(-i)

   rotate x i = shift x i   -- since an Integer never wraps around

   bitSize _  = error "Data.Bits.bitSize(Integer)"
   bitSizeMaybe _ = Nothing
   popCount n
     | n >= 0    = popCountDefault n
     | otherwise = negate (popCountDefault (negate n))
   isSigned _ = True


-- Crude implementation of bitwise operations on Integers: convert them
-- to finite lists of Ints (least significant first), zip and convert
-- back again.

-- posAnd requires at least one argument non-negative
-- posOr and posXOr require both arguments non-negative

posAnd, posOr, posXOr :: Integer -> Integer -> Integer
posAnd x y   = fromInts $ zipWith (.&.) (toInts x) (toInts y)
posOr x y    = fromInts $ longZipWith (.|.) (toInts x) (toInts y)
posXOr x y   = fromInts $ longZipWith xor (toInts x) (toInts y)

longZipWith :: (a -> a -> a) -> [a] -> [a] -> [a]
longZipWith f xs [] = xs
longZipWith f [] ys = ys
longZipWith f (x:xs) (y:ys) = f x y:longZipWith f xs ys

toInts :: Integer -> [Int]
toInts n
    | n == 0 = []
    | otherwise = mkInt (n `mod` numInts):toInts (n `div` numInts)
  where mkInt n | n > toInteger(maxBound::Int) = fromInteger (n-numInts)
		| otherwise = fromInteger n

fromInts :: [Int] -> Integer
fromInts = foldr catInt 0
    where catInt d n = (if d<0 then n+1 else n)*numInts + toInteger d

numInts = toInteger (maxBound::Int) - toInteger (minBound::Int) + 1

-- | Types with a fixed number of bits.
class Bits b => FiniteBits b where
    -- | The number of bits in the type of the argument.
    finiteBitSize :: b -> Int

    -- | The number of zero bits before the most significant set bit.
    countLeadingZeros :: b -> Int
    countLeadingZeros x = (w - 1) - go (w - 1)
      where
        go i | i < 0       = i
             | testBit x i = i
             | otherwise   = go (i - 1)
        w = finiteBitSize x

    -- | The number of zero bits after the least significant set bit.
    countTrailingZeros :: b -> Int
    countTrailingZeros x = go 0
      where
        go i | i >= w      = i
             | testBit x i = i
             | otherwise   = go (i + 1)
        w = finiteBitSize x

instance FiniteBits Int where
    finiteBitSize = bitSize

-- Word's instances are here, with Int's, so that the Prelude's Word has
-- them wherever Data.Bits is imported.

instance FiniteBits Word where
    finiteBitSize = bitSize

instance Bits Word where
    (.&.)         = primAndWord
    (.|.)         = primOrWord
    xor           = primXorWord
    complement    = primComplementWord
    shift         = primShiftWord
    rotate        = primRotateWord (bitSize (0::Word))
    bit           = primBitWord
    setBit x i    = x .|. bit i
    clearBit x i  = x .&. complement (bit i)
    complementBit x i = x `xor` bit i
    testBit       = primTestWord
    bitSize  _    = bitSize (0::Int)
    isSigned _    = False

primitive primAndWord       :: Word -> Word -> Word
primitive primOrWord        :: Word -> Word -> Word
primitive primXorWord       :: Word -> Word -> Word
primitive primComplementWord:: Word -> Word
primitive primShiftWord     :: Word -> Int -> Word
primitive primRotateWord    :: Int -> Word -> Int -> Word
primitive primBitWord       :: Int -> Word
primitive primTestWord      :: Word -> Int -> Bool

-- | A 'bit' from 'shiftL'.
bitDefault :: (Bits a, Num a) => Int -> a
bitDefault i = 1 `shiftL` i

-- | A 'testBit' from '.&.'.
testBitDefault :: (Bits a, Num a) => a -> Int -> Bool
testBitDefault x i = (x .&. bit i) /= 0

-- | A 'popCount' that clears one bit at a time, for a non-negative value
-- or one of fixed size.
popCountDefault :: (Bits a, Num a) => a -> Int
popCountDefault = go 0
  where
    go c 0 = c
    go c w = c `seq` go (c + 1) (w .&. (w - 1))

-- | Convert between integral types, or 'Nothing' if the value does not
-- fit.
toIntegralSized :: (Integral a, Integral b, Bounded b) => a -> Maybe b
toIntegralSized x
    | toInteger y == toInteger x = Just y
    | otherwise                  = Nothing
  where y = fromIntegral x

-- | The value with every bit set.
oneBits :: Bits a => a
oneBits = complement zeroBits

-- | Infix 'xor'.
(.^.) :: Bits a => a -> a -> a
(.^.) = xor

-- | Infix 'shiftR'.
(.>>.) :: Bits a => a -> Int -> a
(.>>.) = shiftR

-- | Infix 'shiftL'.
(.<<.) :: Bits a => a -> Int -> a
(.<<.) = shiftL

-- | Infix 'unsafeShiftR'.
(!>>.) :: Bits a => a -> Int -> a
(!>>.) = unsafeShiftR

-- | Infix 'unsafeShiftL'.
(!<<.) :: Bits a => a -> Int -> a
(!<<.) = unsafeShiftL
