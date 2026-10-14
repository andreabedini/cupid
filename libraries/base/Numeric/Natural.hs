-----------------------------------------------------------------------------
-- |
-- Module      :  Numeric.Natural
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  provisional
-- Portability :  portable
--
-- The natural numbers, of unbounded size.  An operation whose result
-- would be negative raises an 'Underflow' error.

module Numeric.Natural (Natural, minusNaturalMaybe) where

import Data.Ix (Ix(..))

-- | A natural number: a non-negative 'Integer'.
newtype Natural = Natural Integer
	deriving (Eq, Ord)

underflow :: a
underflow = error "arithmetic underflow"

natural :: Integer -> Natural
natural n
	| n < 0     = underflow
	| otherwise = Natural n

instance Show Natural where
	showsPrec d (Natural n) = showsPrec d n

instance Read Natural where
	readsPrec d s = [ (Natural n, r) | (n, r) <- readsPrec d s, n >= 0 ]

instance Num Natural where
	Natural a + Natural b = Natural (a + b)
	Natural a - Natural b = natural (a - b)
	Natural a * Natural b = Natural (a * b)
	negate (Natural 0)    = Natural 0
	negate _              = underflow
	abs n                 = n
	signum (Natural n)    = Natural (signum n)
	fromInteger           = natural

instance Real Natural where
	toRational (Natural n) = toRational n

instance Enum Natural where
	succ (Natural n) = Natural (n + 1)
	pred (Natural n) = natural (n - 1)
	toEnum           = natural . toInteger
	fromEnum (Natural n) = fromInteger n
	enumFrom (Natural n) = map Natural (enumFrom n)
	enumFromThen (Natural a) (Natural b)
		| b >= a    = map Natural (enumFromThen a b)
		| otherwise = map Natural (enumFromThenTo a b 0)
	enumFromTo (Natural a) (Natural b) = map Natural (enumFromTo a b)
	enumFromThenTo (Natural a) (Natural b) (Natural c) =
		map Natural (enumFromThenTo a b c)

instance Integral Natural where
	toInteger (Natural n) = n
	quotRem (Natural a) (Natural b) = let (q, r) = quotRem a b in (Natural q, Natural r)
	divMod (Natural a) (Natural b) = let (q, r) = divMod a b in (Natural q, Natural r)

instance Ix Natural where
	range (Natural a, Natural b) = map Natural (range (a, b))
	index (Natural a, Natural b) (Natural i) = index (a, b) i
	inRange (Natural a, Natural b) (Natural i) = inRange (a, b) i

-- | Subtraction, or 'Nothing' when the result would be negative.
minusNaturalMaybe :: Natural -> Natural -> Maybe Natural
minusNaturalMaybe (Natural a) (Natural b)
	| a >= b    = Just (Natural (a - b))
	| otherwise = Nothing
