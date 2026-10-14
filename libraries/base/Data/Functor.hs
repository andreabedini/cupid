-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Functor
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  provisional
-- Portability :  portable
--
-- Functors: uniform action over a parameterized type.  The class is
-- defined in the Prelude.

module Data.Functor (
	Functor(..),
	(<$>), ($>), (<&>),
	void
	) where

import Control.Monad (void)

infixl 4 $>
infixl 1 <&>

-- | Flipped version of '<$'.
($>) :: Functor f => f a -> b -> f b
($>) = flip (<$)

-- | Flipped version of '<$>'.
(<&>) :: Functor f => f a -> (a -> b) -> f b
as <&> f = f <$> as
