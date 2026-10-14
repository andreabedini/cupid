-----------------------------------------------------------------------------
-- |
-- Module      :  Control.Applicative
-- Copyright   :  Conor McBride and Ross Paterson 2005
-- License     :  BSD-style (see the LICENSE file in the distribution)
--
-- Maintainer  :  ross@soi.city.ac.uk
-- Stability   :  experimental
-- Portability :  portable
--
-- This module describes a structure intermediate between a functor and
-- a monad: it provides pure expressions and sequencing, but no binding.
-- (Technically, a strong lax monoidal functor.)  For more details, see
-- /Applicative Programming with Effects/,
-- by Conor McBride and Ross Paterson, online at
-- <http://www.soi.city.ac.uk/~ross/papers/Applicative.html>.
--
-- This interface was introduced for parsers by Niklas R&#xF6;jemo, because
-- it admits more sharing than the monadic interface.  The names here are
-- mostly based on recent parsing work by Doaitse Swierstra.
--
-- This class is also useful with instances of the
-- 'Data.Traversable.Traversable' class.

module Control.Applicative (
	-- * Applicative functors
	Applicative(..),
	-- * Alternatives
	Alternative(..),
	-- * Instances
	Const(..), WrappedMonad(..), WrappedArrow(..), ZipList(..),
	-- * Utility functions
	(<$>), (<$), (<**>),
	liftA, liftA3,
	optional
	) where

import Control.Arrow
	(Arrow(arr, (&&&)), (>>>), ArrowZero(zeroArrow), ArrowPlus((<+>)))
import Control.Monad (liftM, ap)
import Cupid.Prelude (Alternative(..), MonadPlus(..))

infixl 4 <**>

-- 'Applicative' is defined in the Prelude, as the superclass of 'Monad'.
-- Its instances should satisfy the following laws:
--
-- [/identity/]
--	@'pure' 'id' '<*>' v = v@
--
-- [/composition/]
--	@'pure' (.) '<*>' u '<*>' v '<*>' w = u '<*>' (v '<*>' w)@
--
-- [/homomorphism/]
--	@'pure' f '<*>' 'pure' x = 'pure' (f x)@
--
-- [/interchange/]
--	@u '<*>' 'pure' y = 'pure' ('$' y) '<*>' u@
--
-- The 'Functor' instance should satisfy
--
-- @
--	'fmap' f x = 'pure' f '<*>' x
-- @

-- 'Alternative' is defined in "Cupid.Prelude" too, as the superclass
-- of 'MonadPlus', with its instances for 'Maybe' and lists.

-- new instances

newtype Const a b = Const { getConst :: a }

instance Functor (Const m) where
	fmap _ (Const v) = Const v

instance Monoid m => Applicative (Const m) where
	pure _ = Const mempty
	Const f <*> Const v = Const (f `mappend` v)

newtype WrappedMonad m a = WrapMonad { unwrapMonad :: m a }

instance Monad m => Functor (WrappedMonad m) where
	fmap f (WrapMonad v) = WrapMonad (liftM f v)

instance Monad m => Applicative (WrappedMonad m) where
	pure = WrapMonad . return
	WrapMonad f <*> WrapMonad v = WrapMonad (f `ap` v)

instance Monad m => Monad (WrappedMonad m) where
	WrapMonad a >>= f = WrapMonad (a >>= unwrapMonad . f)

instance MonadPlus m => Alternative (WrappedMonad m) where
	empty = WrapMonad mzero
	WrapMonad u <|> WrapMonad v = WrapMonad (u `mplus` v)

newtype WrappedArrow a b c = WrapArrow { unwrapArrow :: a b c }

instance Arrow a => Functor (WrappedArrow a b) where
	fmap f (WrapArrow a) = WrapArrow (a >>> arr f)

instance Arrow a => Applicative (WrappedArrow a b) where
	pure x = WrapArrow (arr (const x))
	WrapArrow f <*> WrapArrow v = WrapArrow (f &&& v >>> arr (uncurry id))

instance (ArrowZero a, ArrowPlus a) => Alternative (WrappedArrow a b) where
	empty = WrapArrow zeroArrow
	WrapArrow u <|> WrapArrow v = WrapArrow (u <+> v)

-- | Lists, but with an 'Applicative' functor based on zipping, so that
--
-- @f '<$>' 'ZipList' xs1 '<*>' ... '<*>' 'ZipList' xsn = 'ZipList' (zipWithn f xs1 ... xsn)@
--
newtype ZipList a = ZipList { getZipList :: [a] }

instance Functor ZipList where
	fmap f (ZipList xs) = ZipList (map f xs)

instance Applicative ZipList where
	pure x = ZipList (repeat x)
	ZipList fs <*> ZipList xs = ZipList (zipWith id fs xs)

-- extra functions

-- | A variant of '<*>' with the arguments reversed.
(<**>) :: Applicative f => f a -> f (a -> b) -> f b
(<**>) = liftA2 (flip ($))

-- | Lift a function to actions.
-- This function may be used as a value for `fmap` in a `Functor` instance.
liftA :: Applicative f => (a -> b) -> f a -> f b
liftA f a = pure f <*> a

-- | Lift a ternary function to actions.
liftA3 :: Applicative f => (a -> b -> c -> d) -> f a -> f b -> f c -> f d
liftA3 f a b c = f <$> a <*> b <*> c

-- | One or none.
optional :: Alternative f => f a -> f (Maybe a)
optional v = Just <$> v <|> pure Nothing

