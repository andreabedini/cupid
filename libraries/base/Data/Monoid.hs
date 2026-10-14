-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Monoid
-- Copyright   :  (c) Andy Gill 2001,
-- 		  (c) Oregon Graduate Institute of Science and Technology, 2001
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  libraries@haskell.org
-- Stability   :  experimental
-- Portability :  portable
--
-- The Monoid class with various general-purpose instances.
--
--	  Inspired by the paper
--	  /Functional Programming with Overloading and
--	      Higher-Order Polymorphism/, 
--	    Mark P Jones (<http://www.cse.ogi.edu/~mpj/>)
--		  Advanced School of Functional Programming, 1995.
-----------------------------------------------------------------------------

module Data.Monoid (
	Monoid(..),
	(<>),
	Dual(..),
	Endo(..),
	All(..),
	Any(..),
	Sum(..),
	Product(..),
	First(..),
	Last(..),
	Alt(..),
	Ap(..)
  ) where

import Prelude
import Cupid.Prelude (Alternative(..), Endo(..))

-- ---------------------------------------------------------------------------
-- 'Monoid' is defined in the Prelude, with 'Semigroup' as its superclass,
-- and so are its instances for lists, functions, tuples, 'Ordering',
-- 'Maybe' and 'IO'.  A minimal complete definition supplies '<>' in the
-- 'Semigroup' instance and 'mempty', and these should satisfy the monoid
-- laws.

-- | The dual of a monoid, obtained by swapping the arguments of 'mappend'.
newtype Dual a = Dual { getDual :: a }
	deriving (Eq, Ord, Read, Show, Bounded)

instance Semigroup a => Semigroup (Dual a) where
	Dual x <> Dual y = Dual (y <> x)

instance Monoid a => Monoid (Dual a) where
	mempty = Dual mempty

-- 'Endo', the monoid of endomorphisms under composition, is defined in
-- "Cupid.Prelude" too, for the default methods of 'Foldable'.

-- | Boolean monoid under conjunction.
newtype All = All { getAll :: Bool }
	deriving (Eq, Ord, Read, Show, Bounded)

instance Semigroup All where
	All x <> All y = All (x && y)

instance Monoid All where
	mempty = All True

-- | Boolean monoid under disjunction.
newtype Any = Any { getAny :: Bool }
	deriving (Eq, Ord, Read, Show, Bounded)

instance Semigroup Any where
	Any x <> Any y = Any (x || y)

instance Monoid Any where
	mempty = Any False

-- | Monoid under addition.
newtype Sum a = Sum { getSum :: a }
	deriving (Eq, Ord, Read, Show, Bounded)

instance Num a => Semigroup (Sum a) where
	Sum x <> Sum y = Sum (x + y)

instance Num a => Monoid (Sum a) where
	mempty = Sum 0

-- | Monoid under multiplication.
newtype Product a = Product { getProduct :: a }
	deriving (Eq, Ord, Read, Show, Bounded)

instance Num a => Semigroup (Product a) where
	Product x <> Product y = Product (x * y)

instance Num a => Monoid (Product a) where
	mempty = Product 1

-- | Maybe monoid returning the leftmost non-'Nothing' value.
newtype First a = First { getFirst :: Maybe a }
	deriving (Eq, Ord, Read, Show)

instance Semigroup (First a) where
	First Nothing <> b = b
	a             <> _ = a

instance Monoid (First a) where
	mempty = First Nothing

-- | Maybe monoid returning the rightmost non-'Nothing' value.
newtype Last a = Last { getLast :: Maybe a }
	deriving (Eq, Ord, Read, Show)

instance Semigroup (Last a) where
	a <> Last Nothing = a
	_ <> b            = b

instance Monoid (Last a) where
	mempty = Last Nothing

-- | Monoid under '<|>'.
newtype Alt f a = Alt { getAlt :: f a }

instance Alternative f => Semigroup (Alt f a) where
	Alt x <> Alt y = Alt (x <|> y)

instance Alternative f => Monoid (Alt f a) where
	mempty = Alt empty

-- | The monoid of an applicative functor's results.
newtype Ap f a = Ap { getAp :: f a }

instance (Applicative f, Semigroup a) => Semigroup (Ap f a) where
	Ap x <> Ap y = Ap (liftA2 (<>) x y)

instance (Applicative f, Monoid a) => Monoid (Ap f a) where
	mempty = Ap (pure mempty)
