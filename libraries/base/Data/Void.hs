-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Void
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  provisional
-- Portability :  portable
--
-- A type with no values.

module Data.Void (Void, absurd, vacuous) where

-- | The type with no values but bottom.
data Void

instance Eq Void where
	_ == _ = True

instance Ord Void where
	compare _ _ = EQ

instance Show Void where
	showsPrec _ = absurd

instance Semigroup Void where
	a <> _ = a

-- | Since there are no values of 'Void', anything follows from one.
absurd :: Void -> a
absurd a = a `seq` error "Data.Void.absurd"

-- | A functor over 'Void' holds no values, so it holds any type.
vacuous :: Functor f => f Void -> f a
vacuous = fmap absurd
