-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Foldable1
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  provisional
-- Portability :  portable
--
-- Non-empty structures that can be folded, so that folds need no base
-- case.

module Data.Foldable1 (
	Foldable1(..),
	foldr1, foldr1', foldl1, foldl1',
	intercalate1, foldrM1, foldlM1, foldrMapM1, foldlMapM1,
	maximumBy, minimumBy
	) where

import Prelude hiding (foldr1, foldl1, head, last, maximum, minimum)
import qualified Prelude
import Cupid.Prelude (NonEmpty(..))
import Data.Functor.Identity (Identity(..))
import Data.Semigroup (Min(..), Max(..), First(..), Last(..))

-- | Minimal complete definition: 'foldMap1' or 'foldrMap1'.
class Foldable t => Foldable1 t where
	fold1 :: Semigroup m => t m -> m
	fold1 = foldMap1 id

	foldMap1 :: Semigroup m => (a -> m) -> t a -> m
	foldMap1 f = foldrMap1 f (\a m -> f a <> m)

	foldMap1' :: Semigroup m => (a -> m) -> t a -> m
	foldMap1' f = foldlMap1' f (\m a -> m <> f a)

	toNonEmpty :: t a -> NonEmpty a
	toNonEmpty = runNonEmptyDList . foldMap1 singleton

	maximum :: Ord a => t a -> a
	maximum = getMax . foldMap1' Max

	minimum :: Ord a => t a -> a
	minimum = getMin . foldMap1' Min

	head :: t a -> a
	head = getFirst . foldMap1 First

	last :: t a -> a
	last = getLast . foldMap1 Last

	foldrMap1 :: (a -> b) -> (a -> b -> b) -> t a -> b
	foldrMap1 f g xs = case toNonEmpty xs of
		x :| rest -> go x rest
	  where
		go y [] = f y
		go y (z:zs) = g y (go z zs)

	foldlMap1' :: (a -> b) -> (b -> a -> b) -> t a -> b
	foldlMap1' f g xs = case toNonEmpty xs of
		x :| rest -> let z = f x in z `seq` Prelude.foldl (\acc a -> let acc' = g acc a in acc' `seq` acc') z rest

	foldlMap1 :: (a -> b) -> (b -> a -> b) -> t a -> b
	foldlMap1 f g xs = case toNonEmpty xs of
		x :| rest -> Prelude.foldl g (f x) rest

	foldrMap1' :: (a -> b) -> (a -> b -> b) -> t a -> b
	foldrMap1' f g xs = foldrMap1 f g xs

-- A difference list for 'toNonEmpty'.
newtype NonEmptyDList a = NEDL ([a] -> NonEmpty a)

instance Semigroup (NonEmptyDList a) where
	NEDL f <> NEDL g = NEDL (\xs -> case g xs of y :| ys -> f (y : ys))

singleton :: a -> NonEmptyDList a
singleton x = NEDL (x :|)

runNonEmptyDList :: NonEmptyDList a -> NonEmpty a
runNonEmptyDList (NEDL f) = f []

instance Foldable1 NonEmpty where
	foldMap1 f (x :| xs) = go (f x) xs
	  where
		go y [] = y
		go y (z:zs) = y <> go (f z) zs
	toNonEmpty = id

instance Foldable1 Identity where
	foldMap1 f (Identity x) = f x
	toNonEmpty (Identity x) = x :| []

instance Foldable1 ((,) a) where
	foldMap1 f (_, y) = f y
	toNonEmpty (_, y) = y :| []

foldr1 :: Foldable1 t => (a -> a -> a) -> t a -> a
foldr1 = foldrMap1 id

foldr1' :: Foldable1 t => (a -> a -> a) -> t a -> a
foldr1' = foldrMap1' id

foldl1 :: Foldable1 t => (a -> a -> a) -> t a -> a
foldl1 = foldlMap1 id

foldl1' :: Foldable1 t => (a -> a -> a) -> t a -> a
foldl1' = foldlMap1' id

-- | Insert a separator between the elements and combine them.
intercalate1 :: (Foldable1 t, Semigroup m) => m -> t m -> m
intercalate1 sep = foldr1 (\a b -> a <> sep <> b)

foldrM1 :: (Foldable1 t, Monad m) => (a -> a -> m a) -> t a -> m a
foldrM1 f = go . toNonEmpty
  where
	go (e :| es) = case es of
		[] -> return e
		x : xs -> go (x :| xs) >>= f e

foldlM1 :: (Foldable1 t, Monad m) => (a -> a -> m a) -> t a -> m a
foldlM1 f t = case toNonEmpty t of
	x :| xs -> Prelude.foldl (\acc y -> acc >>= \a -> f a y) (return x) xs

foldrMapM1 :: (Foldable1 t, Monad m) => (a -> m b) -> (a -> b -> m b) -> t a -> m b
foldrMapM1 g f = go . toNonEmpty
  where
	go (e :| es) = case es of
		[] -> g e
		x : xs -> go (x :| xs) >>= f e

foldlMapM1 :: (Foldable1 t, Monad m) => (a -> m b) -> (b -> a -> m b) -> t a -> m b
foldlMapM1 g f t = case toNonEmpty t of
	x :| xs -> Prelude.foldl (\acc y -> acc >>= \b -> f b y) (g x) xs

maximumBy :: Foldable1 t => (a -> a -> Ordering) -> t a -> a
maximumBy cmp = foldl1' (\x y -> case cmp x y of GT -> x; _ -> y)

minimumBy :: Foldable1 t => (a -> a -> Ordering) -> t a -> a
minimumBy cmp = foldl1' (\x y -> case cmp x y of GT -> y; _ -> x)
