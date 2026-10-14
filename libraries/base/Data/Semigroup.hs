-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Semigroup
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  provisional
-- Portability :  portable
--
-- Semigroups: types with an associative operation, '<>'.  The class and
-- its instances for the Prelude's types are defined in the Prelude, as
-- the superclass of 'Monoid'; this module adds the semigroups that are
-- not monoids and the functions on them.  Its 'First' and 'Last' keep
-- the first and last value, unlike those of "Data.Monoid", which keep
-- the first and last 'Just'.

module Data.Semigroup (
	Semigroup(..),
	stimesMonoid, stimesIdempotent, stimesIdempotentMonoid,
	mtimesDefault,
	-- * Semigroups
	Min(..), Max(..), First(..), Last(..), WrappedMonoid(..),
	-- * Re-exported monoids from "Data.Monoid"
	Dual(..), Endo(..), All(..), Any(..), Sum(..), Product(..),
	-- * Difference lists of a semigroup
	diff, cycle1,
	-- * ArgMin, ArgMax
	Arg(..), ArgMin, ArgMax,
	-- * Non-empty lists
	NonEmpty(..)
  ) where

import Prelude
import Cupid.Prelude (Semigroup(..), NonEmpty(..))
import Data.Monoid (Dual(..), Endo(..), All(..), Any(..), Sum(..), Product(..))

-- | A 'stimes' for a monoid, which allows a count of zero.
stimesMonoid :: (Integral b, Monoid a) => b -> a -> a
stimesMonoid n x0 = case compare n 0 of
	LT -> error "stimesMonoid: negative multiplier"
	EQ -> mempty
	GT -> f x0 n
  where
	f x y
	  | even y    = f (x `mappend` x) (y `quot` 2)
	  | y == 1    = x
	  | otherwise = g (x `mappend` x) (y `quot` 2) x
	g x y z
	  | even y    = g (x `mappend` x) (y `quot` 2) z
	  | y == 1    = x `mappend` z
	  | otherwise = g (x `mappend` x) (y `quot` 2) (x `mappend` z)

-- | A 'stimes' for an idempotent semigroup, where @x '<>' x = x@.
stimesIdempotent :: Integral b => b -> a -> a
stimesIdempotent n x
	| n <= 0    = error "stimesIdempotent: positive multiplier expected"
	| otherwise = x

-- | A 'stimes' for an idempotent monoid, which allows a count of zero.
stimesIdempotentMonoid :: (Integral b, Monoid a) => b -> a -> a
stimesIdempotentMonoid n x = case compare n 0 of
	LT -> error "stimesIdempotentMonoid: negative multiplier"
	EQ -> mempty
	GT -> x

-- | Repeat a value @n@ times, @n@ being zero or more.
mtimesDefault :: (Integral b, Monoid a) => b -> a -> a
mtimesDefault n x
	| n == 0    = mempty
	| otherwise = stimes n x

-- | The semigroup of the least value.
newtype Min a = Min { getMin :: a }
	deriving (Eq, Ord, Read, Show, Bounded)

instance Ord a => Semigroup (Min a) where
	Min a <> Min b = Min (a `min` b)
	stimes = stimesIdempotent

instance (Ord a, Bounded a) => Monoid (Min a) where
	mempty = maxBound

instance Functor Min where
	fmap f (Min a) = Min (f a)

-- | The semigroup of the greatest value.
newtype Max a = Max { getMax :: a }
	deriving (Eq, Ord, Read, Show, Bounded)

instance Ord a => Semigroup (Max a) where
	Max a <> Max b = Max (a `max` b)
	stimes = stimesIdempotent

instance (Ord a, Bounded a) => Monoid (Max a) where
	mempty = minBound

instance Functor Max where
	fmap f (Max a) = Max (f a)

-- | The semigroup that keeps the first value.
newtype First a = First { getFirst :: a }
	deriving (Eq, Ord, Read, Show, Bounded)

instance Semigroup (First a) where
	a <> _ = a
	stimes = stimesIdempotent

instance Functor First where
	fmap f (First a) = First (f a)

-- | The semigroup that keeps the last value.
newtype Last a = Last { getLast :: a }
	deriving (Eq, Ord, Read, Show, Bounded)

instance Semigroup (Last a) where
	_ <> b = b
	stimes = stimesIdempotent

instance Functor Last where
	fmap f (Last a) = Last (f a)

-- | A monoid used as a semigroup.
newtype WrappedMonoid m = WrapMonoid { unwrapMonoid :: m }
	deriving (Eq, Ord, Read, Show, Bounded)

instance Monoid m => Semigroup (WrappedMonoid m) where
	WrapMonoid a <> WrapMonoid b = WrapMonoid (a `mappend` b)

instance Monoid m => Monoid (WrappedMonoid m) where
	mempty = WrapMonoid mempty

-- | A difference list: '<>' on the result is function composition.
diff :: Semigroup m => m -> Endo m
diff = Endo . (<>)

-- | The infinite repetition of a value: @xs '<>' xs '<>' ...@.
cycle1 :: Semigroup m => m -> m
cycle1 xs = xs' where xs' = xs <> xs'

-- | A value tagged with a key, compared on the key only, for use with
-- 'Min' and 'Max'.
data Arg a b = Arg a b
	deriving (Read, Show)

type ArgMin a b = Min (Arg a b)
type ArgMax a b = Max (Arg a b)

instance Functor (Arg a) where
	fmap f (Arg x a) = Arg x (f a)

instance Eq a => Eq (Arg a b) where
	Arg a _ == Arg b _ = a == b

instance Ord a => Ord (Arg a b) where
	Arg a _ `compare` Arg b _ = compare a b
	min x@(Arg a _) y@(Arg b _)
	  | a <= b    = x
	  | otherwise = y
	max x@(Arg a _) y@(Arg b _)
	  | a >= b    = x
	  | otherwise = y
