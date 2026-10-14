-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Functor.Sum
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  experimental
-- Portability :  portable
--
-- Sums, lifted to functors.

module Data.Functor.Sum (Sum(..)) where

-- | Lifted sum of functors.
data Sum f g a = InL (f a) | InR (g a)

instance (Eq (f a), Eq (g a)) => Eq (Sum f g a) where
	InL x == InL y = x == y
	InR x == InR y = x == y
	_     == _     = False

instance (Ord (f a), Ord (g a)) => Ord (Sum f g a) where
	compare (InL x) (InL y) = compare x y
	compare (InL _) (InR _) = LT
	compare (InR _) (InL _) = GT
	compare (InR x) (InR y) = compare x y

instance (Show (f a), Show (g a)) => Show (Sum f g a) where
	showsPrec d (InL x) = showParen (d >= 11) $ showString "InL " . showsPrec 11 x
	showsPrec d (InR y) = showParen (d >= 11) $ showString "InR " . showsPrec 11 y

instance (Functor f, Functor g) => Functor (Sum f g) where
	fmap f (InL x) = InL (fmap f x)
	fmap f (InR y) = InR (fmap f y)

instance (Foldable f, Foldable g) => Foldable (Sum f g) where
	foldMap f (InL x) = foldMap f x
	foldMap f (InR y) = foldMap f y

instance (Traversable f, Traversable g) => Traversable (Sum f g) where
	traverse f (InL x) = InL <$> traverse f x
	traverse f (InR y) = InR <$> traverse f y
