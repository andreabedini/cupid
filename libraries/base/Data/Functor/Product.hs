-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Functor.Product
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  experimental
-- Portability :  portable
--
-- Products, lifted to functors.

module Data.Functor.Product (Product(..)) where

import Cupid.Prelude (Alternative(..), MonadPlus(..))

-- | Lifted product of functors.
data Product f g a = Pair (f a) (g a)

instance (Eq (f a), Eq (g a)) => Eq (Product f g a) where
	Pair x1 y1 == Pair x2 y2 = x1 == x2 && y1 == y2

instance (Ord (f a), Ord (g a)) => Ord (Product f g a) where
	compare (Pair x1 y1) (Pair x2 y2) = compare x1 x2 <> compare y1 y2

instance (Show (f a), Show (g a)) => Show (Product f g a) where
	showsPrec d (Pair x y) = showParen (d >= 11) $
		showString "Pair " . showsPrec 11 x . showChar ' ' . showsPrec 11 y

instance (Functor f, Functor g) => Functor (Product f g) where
	fmap f (Pair x y) = Pair (fmap f x) (fmap f y)

instance (Foldable f, Foldable g) => Foldable (Product f g) where
	foldMap f (Pair x y) = foldMap f x `mappend` foldMap f y

instance (Traversable f, Traversable g) => Traversable (Product f g) where
	traverse f (Pair x y) = liftA2 Pair (traverse f x) (traverse f y)

instance (Applicative f, Applicative g) => Applicative (Product f g) where
	pure x = Pair (pure x) (pure x)
	Pair f g <*> Pair x y = Pair (f <*> x) (g <*> y)

instance (Alternative f, Alternative g) => Alternative (Product f g) where
	empty = Pair empty empty
	Pair x1 y1 <|> Pair x2 y2 = Pair (x1 <|> x2) (y1 <|> y2)

instance (Monad f, Monad g) => Monad (Product f g) where
	Pair m n >>= f = Pair (m >>= fstP . f) (n >>= sndP . f)
	  where
		fstP (Pair a _) = a
		sndP (Pair _ b) = b

instance (MonadPlus f, MonadPlus g) => MonadPlus (Product f g)
