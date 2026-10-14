-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Functor.Contravariant
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  provisional
-- Portability :  portable
--
-- Contravariant functors: those that consume values rather than
-- produce them.

module Data.Functor.Contravariant (
	Contravariant(..),
	phantom,
	(>$<), (>$$<), ($<),
	Predicate(..), Comparison(..), Equivalence(..),
	defaultComparison, defaultEquivalence, comparisonEquivalence,
	Op(..)
	) where

import Control.Applicative (Const(..))
import Data.Functor.Compose (Compose(..))
import qualified Data.Functor.Product as P
import qualified Data.Functor.Sum as S

infixl 4 >$, >$<, >$$<, $<

-- | A functor whose map goes the other way.  Minimal complete
-- definition: 'contramap'.
class Contravariant f where
	contramap :: (a' -> a) -> f a -> f a'

	-- | Replace every value consumed by a constant.
	(>$) :: b -> f b -> f a
	(>$) = contramap . const

-- | A functor that is both covariant and contravariant holds no values
-- of its parameter.
phantom :: (Functor f, Contravariant f) => f a -> f b
phantom x = () >$ fmap (const ()) x

-- | An infix 'contramap'.
(>$<) :: Contravariant f => (a -> b) -> f b -> f a
(>$<) = contramap

-- | 'contramap' with its arguments flipped.
(>$$<) :: Contravariant f => f b -> (a -> b) -> f a
(>$$<) = flip contramap

-- | '>$' with its arguments flipped.
($<) :: Contravariant f => f b -> b -> f a
($<) = flip (>$)

instance Contravariant (Const a) where
	contramap _ (Const a) = Const a

instance (Functor f, Contravariant g) => Contravariant (Compose f g) where
	contramap f (Compose fga) = Compose (fmap (contramap f) fga)

instance (Contravariant f, Contravariant g) => Contravariant (P.Product f g) where
	contramap f (P.Pair a b) = P.Pair (contramap f a) (contramap f b)

instance (Contravariant f, Contravariant g) => Contravariant (S.Sum f g) where
	contramap f (S.InL xs) = S.InL (contramap f xs)
	contramap f (S.InR ys) = S.InR (contramap f ys)

-- | A predicate on @a@.
newtype Predicate a = Predicate { getPredicate :: a -> Bool }

instance Contravariant Predicate where
	contramap f (Predicate p) = Predicate (p . f)

instance Semigroup (Predicate a) where
	Predicate p <> Predicate q = Predicate (\a -> p a && q a)

instance Monoid (Predicate a) where
	mempty = Predicate (const True)

-- | A total ordering on @a@.
newtype Comparison a = Comparison { getComparison :: a -> a -> Ordering }

instance Contravariant Comparison where
	contramap f (Comparison c) = Comparison (\a b -> c (f a) (f b))

instance Semigroup (Comparison a) where
	Comparison p <> Comparison q = Comparison (\a b -> p a b <> q a b)

instance Monoid (Comparison a) where
	mempty = Comparison (\_ _ -> EQ)

-- | The 'Comparison' of an 'Ord' instance.
defaultComparison :: Ord a => Comparison a
defaultComparison = Comparison compare

-- | An equivalence relation on @a@.
newtype Equivalence a = Equivalence { getEquivalence :: a -> a -> Bool }

instance Contravariant Equivalence where
	contramap f (Equivalence e) = Equivalence (\a b -> e (f a) (f b))

instance Semigroup (Equivalence a) where
	Equivalence p <> Equivalence q = Equivalence (\a b -> p a b && q a b)

instance Monoid (Equivalence a) where
	mempty = Equivalence (\_ _ -> True)

-- | The 'Equivalence' of an 'Eq' instance.
defaultEquivalence :: Eq a => Equivalence a
defaultEquivalence = Equivalence (==)

-- | Elements equivalent when they compare 'EQ'.
comparisonEquivalence :: Comparison a -> Equivalence a
comparisonEquivalence (Comparison p) = Equivalence (\a b -> p a b == EQ)

-- | The arrow type with its arguments the other way round.
newtype Op a b = Op { getOp :: b -> a }

instance Contravariant (Op a) where
	contramap f g = Op (getOp g . f)

instance Semigroup a => Semigroup (Op a b) where
	Op p <> Op q = Op (\a -> p a <> q a)

instance Monoid a => Monoid (Op a b) where
	mempty = Op (const mempty)
