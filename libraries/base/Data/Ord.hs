{-# OPTIONS_GHC -fno-implicit-prelude #-}
-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Ord
-- Copyright   :  (c) The University of Glasgow 2005
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  libraries@haskell.org
-- Stability   :  stable
-- Portability :  portable
--
-- Orderings
--
-----------------------------------------------------------------------------

module Data.Ord (
   Ord(..),
   Ordering(..),
   Down(..),
   comparing,
   clamp,
 ) where





-- | 
-- > comparing p x y = compare (p x) (p y)
--
-- Useful combinator for use in conjunction with the @xxxBy@ family
-- of functions from "Data.List", for example:
--
-- >   ... sortBy (comparing fst) ...
comparing :: (Ord a) => (b -> a) -> b -> b -> Ordering
comparing p x y = compare (p x) (p y)

-- | Restrict a value to an interval, given by its bounds.
clamp :: Ord a => (a, a) -> a -> a
clamp (low, high) a = min high (max a low)

-- | The dual ordering: @'compare' ('Down' x) ('Down' y) = 'compare' y x@.
newtype Down a = Down { getDown :: a }
	deriving Eq

instance Ord a => Ord (Down a) where
	compare (Down x) (Down y) = compare y x

-- | Shown as an application of the constructor, without the field name.
instance Show a => Show (Down a) where
	showsPrec d (Down x) = showParen (d >= 11) $
		showString "Down " . showsPrec 11 x

instance Functor Down where
	fmap f (Down a) = Down (f a)
