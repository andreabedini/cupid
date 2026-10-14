-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Traversable
-- Copyright   :  Conor McBride and Ross Paterson 2005
-- License     :  BSD-style (see the LICENSE file in the distribution)
--
-- Maintainer  :  ross@soi.city.ac.uk
-- Stability   :  experimental
-- Portability :  portable
--
-- Class of data structures that can be traversed from left to right,
-- performing an action on each element.
--
-- See also
--
--  * /Applicative Programming with Effects/,
--    by Conor McBride and Ross Paterson, online at
--    <http://www.soi.city.ac.uk/~ross/papers/Applicative.html>.
--
--  * /The Essence of the Iterator Pattern/,
--    by Jeremy Gibbons and Bruno Oliveira,
--    in /Mathematically-Structured Functional Programming/, 2006, and online at
--    <http://web.comlab.ox.ac.uk/oucl/work/jeremy.gibbons/publications/#iterator>.
--
-- The class is defined in the Prelude, which exports it, so its 'mapM'
-- and 'sequence' are the Prelude's.

module Data.Traversable (
	Traversable(..),
	for,
	forM,
	mapAccumL,
	mapAccumR,
	fmapDefault,
	foldMapDefault,
	) where

import Prelude
import Cupid.Prelude (Traversable(..), forM, mapAccumL, mapAccumR)
import Control.Applicative (Const(..))

-- 'Traversable' is defined in the Prelude, with its instances for lists,
-- 'Maybe', 'Either' and pairs; the instance for arrays is in "Data.Array".

-- | 'for' is 'traverse' with its arguments flipped.
for :: (Traversable t, Applicative f) => t a -> (a -> f b) -> f (t b)
for = flip traverse

-- | This function may be used as a value for `fmap` in a `Functor` instance.
fmapDefault :: Traversable t => (a -> b) -> t a -> t b
fmapDefault f = getId . traverse (Id . f)

-- | This function may be used as a value for `Data.Foldable.foldMap`
-- in a `Foldable` instance.
foldMapDefault :: (Traversable t, Monoid m) => (a -> m) -> t a -> m
foldMapDefault f = getConst . traverse (Const . f)

-- local instances

newtype Id a = Id { getId :: a }

instance Functor Id where
	fmap f (Id x) = Id (f x)

instance Applicative Id where
	pure = Id
	Id f <*> Id x = Id (f x)
