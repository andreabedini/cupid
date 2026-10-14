-----------------------------------------------------------------------------
-- |
-- Module      :  Data.String
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  stable
-- Portability :  portable
--
-- The 'String' type and the class of string-like types.  Cupid has no
-- overloaded string literals, so 'fromString' must be called explicitly.

module Data.String (
	String,
	IsString(..),
	-- * Functions on strings
	lines, words, unlines, unwords
	) where

import Data.Functor.Const (Const(..))
import Data.Functor.Identity (Identity(..))

-- | Types that can be built from a 'String'.
class IsString a where
	fromString :: String -> a

instance (a ~ Char) => IsString [a] where
	fromString xs = xs

instance IsString a => IsString (Const a b) where
	fromString = Const . fromString

instance IsString a => IsString (Identity a) where
	fromString = Identity . fromString
