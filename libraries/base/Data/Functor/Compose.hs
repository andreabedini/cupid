-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Functor.Compose
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  experimental
-- Portability :  portable
--
-- Composition of functors.

module Data.Functor.Compose (Compose(..)) where

import Cupid.Prelude (Alternative(..))

infixr 9 `Compose`

-- | Right-to-left composition of functors.
newtype Compose f g a = Compose { getCompose :: f (g a) }

instance Eq (f (g a)) => Eq (Compose f g a) where
	Compose x == Compose y = x == y

instance Ord (f (g a)) => Ord (Compose f g a) where
	compare (Compose x) (Compose y) = compare x y

-- | Shown as an application of the constructor, without the field name.
instance Show (f (g a)) => Show (Compose f g a) where
	showsPrec d (Compose x) = showParen (d >= 11) $
		showString "Compose " . showsPrec 11 x

instance (Functor f, Functor g) => Functor (Compose f g) where
	fmap f (Compose x) = Compose (fmap (fmap f) x)

instance (Foldable f, Foldable g) => Foldable (Compose f g) where
	foldMap f (Compose t) = foldMap (foldMap f) t

instance (Traversable f, Traversable g) => Traversable (Compose f g) where
	traverse f (Compose t) = Compose <$> traverse (traverse f) t

instance (Applicative f, Applicative g) => Applicative (Compose f g) where
	pure x = Compose (pure (pure x))
	Compose f <*> Compose x = Compose (liftA2 (<*>) f x)
	liftA2 f (Compose x) (Compose y) = Compose (liftA2 (liftA2 f) x y)

instance (Alternative f, Applicative g) => Alternative (Compose f g) where
	empty = Compose empty
	Compose x <|> Compose y = Compose (x <|> y)
