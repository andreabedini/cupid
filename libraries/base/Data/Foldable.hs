-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Foldable
-- Copyright   :  Ross Paterson 2005
-- License     :  BSD-style (see the LICENSE file in the distribution)
--
-- Maintainer  :  ross@soi.city.ac.uk
-- Stability   :  experimental
-- Portability :  portable
--
-- Class of data structures that can be folded to a summary value.
--
-- The class and most of these functions are defined in the Prelude,
-- which exports them, as do "Control.Monad" and "Data.List" where they
-- generalize list functions of the same names, so importing those
-- modules as well causes no ambiguity.

module Data.Foldable (
	-- * Folds
	Foldable(..),
	-- ** Special biased folds
	foldrM,
	foldlM,
	-- ** Folding actions
	-- *** Applicative actions
	traverse_,
	for_,
	sequenceA_,
	asum,
	-- *** Monadic actions
	mapM_,
	forM_,
	sequence_,
	msum,
	-- ** Specialized folds
	concat,
	concatMap,
	and,
	or,
	any,
	all,
	maximumBy,
	minimumBy,
	-- ** Searches
	notElem,
	find
	) where

import Prelude
import Cupid.Prelude (Foldable(..), Alternative(..), MonadPlus(..),
	find, maximumBy, minimumBy, forM_, msum)

-- 'Foldable' is defined in the Prelude, with its instances for lists,
-- 'Maybe', 'Either' and pairs; the instance for arrays is in "Data.Array".

-- | Monadic fold over the elements of a structure,
-- associating to the right, i.e. from right to left.
foldrM :: (Foldable t, Monad m) => (a -> b -> m b) -> b -> t a -> m b
foldrM f z xs = foldl f' return xs z
  where f' k x z = f x z >>= k

-- | Monadic fold over the elements of a structure,
-- associating to the left, i.e. from left to right.
foldlM :: (Foldable t, Monad m) => (a -> b -> m a) -> a -> t b -> m a
foldlM f z xs = foldr f' return xs z
  where f' x k z = f z x >>= k

-- | Map each element of a structure to an action, evaluate
-- these actions from left to right, and ignore the results.
traverse_ :: (Foldable t, Applicative f) => (a -> f b) -> t a -> f ()
traverse_ f = foldr ((*>) . f) (pure ())

-- | 'for_' is 'traverse_' with its arguments flipped.
for_ :: (Foldable t, Applicative f) => t a -> (a -> f b) -> f ()
for_ = flip traverse_

-- | Evaluate each action in the structure from left to right,
-- and ignore the results.
sequenceA_ :: (Foldable t, Applicative f) => t (f a) -> f ()
sequenceA_ = foldr (*>) (pure ())

-- | The sum of a collection of actions, generalizing 'concat'.
asum :: (Foldable t, Alternative f) => t (f a) -> f a
asum = foldr (<|>) empty
