-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Bifoldable
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  provisional
-- Portability :  portable
--
-- Structures of two kinds of elements that can be folded.

module Data.Bifoldable (
	Bifoldable(..),
	bifoldr', bifoldl', bifoldrM, bifoldlM,
	bitraverse_, bifor_, bisequence_, bisum, biproduct,
	biconcat, biconcatMap, biany, biall, bielem, binull, bilength,
	bitoList
	) where

import Control.Applicative (Const(..))
import Data.Monoid

-- | Minimal complete definition: 'bifoldMap' or 'bifoldr'.
class Bifoldable p where
	bifold :: Monoid m => p m m -> m
	bifold = bifoldMap id id

	bifoldMap :: Monoid m => (a -> m) -> (b -> m) -> p a b -> m
	bifoldMap f g = bifoldr (mappend . f) (mappend . g) mempty

	bifoldr :: (a -> c -> c) -> (b -> c -> c) -> c -> p a b -> c
	bifoldr f g z t = appEndo (bifoldMap (Endo . f) (Endo . g) t) z

	bifoldl :: (c -> a -> c) -> (c -> b -> c) -> c -> p a b -> c
	bifoldl f g z t = bifoldr (\x k acc -> k (f acc x)) (\y k acc -> k (g acc y)) id t z

instance Bifoldable (,) where
	bifoldMap f g ~(a, b) = f a `mappend` g b

instance Bifoldable Const where
	bifoldMap f _ (Const a) = f a

instance Bifoldable ((,,) x) where
	bifoldMap f g ~(_, a, b) = f a `mappend` g b

instance Bifoldable ((,,,) x y) where
	bifoldMap f g ~(_, _, a, b) = f a `mappend` g b

instance Bifoldable Either where
	bifoldMap f _ (Left a) = f a
	bifoldMap _ g (Right b) = g b

bifoldr' :: Bifoldable t => (a -> c -> c) -> (b -> c -> c) -> c -> t a b -> c
bifoldr' f g z0 xs = bifoldl (\k x z -> k $! f x z) (\k x z -> k $! g x z) id xs z0

bifoldl' :: Bifoldable t => (a -> b -> a) -> (a -> c -> a) -> a -> t b c -> a
bifoldl' f g z0 xs = bifoldr (\x k z -> k $! f z x) (\x k z -> k $! g z x) id xs z0

bifoldrM :: (Bifoldable t, Monad m) => (a -> c -> m c) -> (b -> c -> m c) -> c -> t a b -> m c
bifoldrM f g z0 xs = bifoldl (\k x z -> f x z >>= k) (\k x z -> g x z >>= k) return xs z0

bifoldlM :: (Bifoldable t, Monad m) => (a -> b -> m a) -> (a -> c -> m a) -> a -> t b c -> m a
bifoldlM f g z0 xs = bifoldr (\x k z -> f z x >>= k) (\x k z -> g z x >>= k) return xs z0

bitraverse_ :: (Bifoldable t, Applicative f) => (a -> f c) -> (b -> f d) -> t a b -> f ()
bitraverse_ f g = bifoldr ((*>) . f) ((*>) . g) (pure ())

bifor_ :: (Bifoldable t, Applicative f) => t a b -> (a -> f c) -> (b -> f d) -> f ()
bifor_ t f g = bitraverse_ f g t

bisequence_ :: (Bifoldable t, Applicative f) => t (f a) (f b) -> f ()
bisequence_ = bifoldr (*>) (*>) (pure ())

bisum :: (Bifoldable t, Num a) => t a a -> a
bisum = getSum . bifoldMap Sum Sum

biproduct :: (Bifoldable t, Num a) => t a a -> a
biproduct = getProduct . bifoldMap Product Product

biconcat :: Bifoldable t => t [a] [a] -> [a]
biconcat = bifold

biconcatMap :: Bifoldable t => (a -> [c]) -> (b -> [c]) -> t a b -> [c]
biconcatMap = bifoldMap

biany :: Bifoldable t => (a -> Bool) -> (b -> Bool) -> t a b -> Bool
biany p q = getAny . bifoldMap (Any . p) (Any . q)

biall :: Bifoldable t => (a -> Bool) -> (b -> Bool) -> t a b -> Bool
biall p q = getAll . bifoldMap (All . p) (All . q)

bielem :: (Bifoldable t, Eq a) => a -> t a a -> Bool
bielem x = biany (== x) (== x)

binull :: Bifoldable t => t a b -> Bool
binull = bifoldr (\_ _ -> False) (\_ _ -> False) True

bilength :: Bifoldable t => t a b -> Int
bilength = bifoldl' (\c _ -> c + 1) (\c _ -> c + 1) 0

bitoList :: Bifoldable t => t a a -> [a]
bitoList = bifoldr (:) (:) []
