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

import Data.Functor.Classes

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

instance (Eq1 f, Eq1 g) => Eq1 (Sum f g) where
	liftEq eq (InL x1) (InL x2) = liftEq eq x1 x2
	liftEq _ (InL _) (InR _) = False
	liftEq _ (InR _) (InL _) = False
	liftEq eq (InR y1) (InR y2) = liftEq eq y1 y2

instance (Ord1 f, Ord1 g) => Ord1 (Sum f g) where
	liftCompare comp (InL x1) (InL x2) = liftCompare comp x1 x2
	liftCompare _ (InL _) (InR _) = LT
	liftCompare _ (InR _) (InL _) = GT
	liftCompare comp (InR y1) (InR y2) = liftCompare comp y1 y2

instance (Read1 f, Read1 g) => Read1 (Sum f g) where
	liftReadsPrec rp rl = readsData $
		readsUnaryWith (liftReadsPrec rp rl) "InL" InL `mappend`
		readsUnaryWith (liftReadsPrec rp rl) "InR" InR

instance (Show1 f, Show1 g) => Show1 (Sum f g) where
	liftShowsPrec sp sl d (InL x) = showsUnaryWith (liftShowsPrec sp sl) "InL" d x
	liftShowsPrec sp sl d (InR y) = showsUnaryWith (liftShowsPrec sp sl) "InR" d y
