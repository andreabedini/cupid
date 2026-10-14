-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Bifunctor
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  provisional
-- Portability :  portable
--
-- Functors of two arguments.

module Data.Bifunctor (Bifunctor(..)) where

-- | A type constructor of two arguments that is a functor in each.
-- Minimal complete definition: 'bimap', or 'first' and 'second'.
class Bifunctor p where
	bimap  :: (a -> b) -> (c -> d) -> p a c -> p b d
	first  :: (a -> b) -> p a c -> p b c
	second :: (b -> c) -> p a b -> p a c

	bimap f g = first f . second g
	first f   = bimap f id
	second    = bimap id

instance Bifunctor (,) where
	bimap f g ~(a, b) = (f a, g b)

instance Bifunctor ((,,) x1) where
	bimap f g ~(x1, a, b) = (x1, f a, g b)

instance Bifunctor ((,,,) x1 x2) where
	bimap f g ~(x1, x2, a, b) = (x1, x2, f a, g b)

instance Bifunctor Either where
	bimap f _ (Left a)  = Left (f a)
	bimap _ g (Right b) = Right (g b)
