-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Bitraversable
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  provisional
-- Portability :  portable
--
-- Structures of two kinds of elements that can be traversed.

module Data.Bitraversable (
	Bitraversable(..),
	bisequenceA, bisequence, bimapM, bifor, biforM,
	bimapDefault, bifoldMapDefault
	) where

import Control.Applicative (Const(..))
import Data.Bifunctor
import Data.Bifoldable
import Data.Functor.Identity (Identity(..))

-- | Minimal complete definition: 'bitraverse'.
class (Bifunctor t, Bifoldable t) => Bitraversable t where
	bitraverse :: Applicative f => (a -> f c) -> (b -> f d) -> t a b -> f (t c d)

instance Bitraversable (,) where
	bitraverse f g ~(a, b) = liftA2 (,) (f a) (g b)

instance Bitraversable ((,,) x) where
	bitraverse f g ~(x, a, b) = liftA2 ((,,) x) (f a) (g b)

instance Bitraversable ((,,,) x y) where
	bitraverse f g ~(x, y, a, b) = liftA2 ((,,,) x y) (f a) (g b)

instance Bitraversable Either where
	bitraverse f _ (Left a) = Left <$> f a
	bitraverse _ g (Right b) = Right <$> g b

instance Bitraversable Const where
	bitraverse f _ (Const a) = Const <$> f a

instance Bifunctor Const where
	bimap f _ (Const a) = Const (f a)

bisequenceA :: (Bitraversable t, Applicative f) => t (f a) (f b) -> f (t a b)
bisequenceA = bitraverse id id

bisequence :: (Bitraversable t, Applicative f) => t (f a) (f b) -> f (t a b)
bisequence = bisequenceA

bimapM :: (Bitraversable t, Applicative f) => (a -> f c) -> (b -> f d) -> t a b -> f (t c d)
bimapM = bitraverse

bifor :: (Bitraversable t, Applicative f) => t a b -> (a -> f c) -> (b -> f d) -> f (t c d)
bifor t f g = bitraverse f g t

biforM :: (Bitraversable t, Applicative f) => t a b -> (a -> f c) -> (b -> f d) -> f (t c d)
biforM = bifor

-- | A 'bimap' for any 'Bitraversable'.
bimapDefault :: Bitraversable t => (a -> b) -> (c -> d) -> t a c -> t b d
bimapDefault f g = runIdentity . bitraverse (Identity . f) (Identity . g)

-- | A 'bifoldMap' for any 'Bitraversable'.
bifoldMapDefault :: (Bitraversable t, Monoid m) => (a -> m) -> (b -> m) -> t a b -> m
bifoldMapDefault f g = getConst . bitraverse (Const . f) (Const . g)
