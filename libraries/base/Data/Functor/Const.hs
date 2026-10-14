-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Functor.Const
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  experimental
-- Portability :  portable
--
-- The constant functor, which "Control.Applicative" defines; this
-- module adds its instances for the other classes.

module Data.Functor.Const (Const(..)) where

import Control.Applicative (Const(..))

instance Eq a => Eq (Const a b) where
	Const a == Const b = a == b

instance Ord a => Ord (Const a b) where
	compare (Const a) (Const b) = compare a b

-- | Shown as an application of the constructor, without the field name.
instance Show a => Show (Const a b) where
	showsPrec d (Const x) = showParen (d >= 11) $
		showString "Const " . showsPrec 11 x

instance Semigroup a => Semigroup (Const a b) where
	Const a <> Const b = Const (a <> b)

instance Monoid a => Monoid (Const a b) where
	mempty = Const mempty

instance Foldable (Const m) where
	foldMap _ _ = mempty
	foldr _ z _ = z
	length _ = 0
	null _ = True

instance Traversable (Const m) where
	traverse _ (Const m) = pure (Const m)
