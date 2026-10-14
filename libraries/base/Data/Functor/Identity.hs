-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Functor.Identity
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  experimental
-- Portability :  portable
--
-- The identity functor and monad, the base of monad transformer stacks.

module Data.Functor.Identity (Identity(..)) where

import Control.Monad.Fix (MonadFix(..))
import Control.Monad.Zip (MonadZip(..))
import Data.String (IsString(..))

-- | The identity functor and monad.
newtype Identity a = Identity { runIdentity :: a }
	deriving (Eq, Ord, Bounded)

-- | Shown as an application of the constructor, without the field name.
instance Show a => Show (Identity a) where
	showsPrec d (Identity x) = showParen (d >= 11) $
		showString "Identity " . showsPrec 11 x

instance Read a => Read (Identity a) where
	readsPrec d = readParen (d > 10) $ \r ->
		[ (Identity x, t) | ("Identity", s) <- lex r, (x, t) <- readsPrec 11 s ]

instance Semigroup a => Semigroup (Identity a) where
	Identity a <> Identity b = Identity (a <> b)

instance Monoid a => Monoid (Identity a) where
	mempty = Identity mempty

instance Functor Identity where
	fmap f (Identity a) = Identity (f a)

instance Applicative Identity where
	pure = Identity
	Identity f <*> Identity a = Identity (f a)
	liftA2 f (Identity a) (Identity b) = Identity (f a b)

instance Monad Identity where
	m >>= k = k (runIdentity m)

instance MonadFix Identity where
	mfix f = Identity (let x = runIdentity (f x) in x)

instance MonadZip Identity where
	mzipWith f (Identity a) (Identity b) = Identity (f a b)
	munzip (Identity (a, b)) = (Identity a, Identity b)

instance Foldable Identity where
	foldMap f (Identity a) = f a
	foldr f z (Identity a) = f a z
	length _ = 1
	null _ = False

instance Traversable Identity where
	traverse f (Identity a) = fmap Identity (f a)

instance IsString a => IsString (Identity a) where
	fromString = Identity . fromString
