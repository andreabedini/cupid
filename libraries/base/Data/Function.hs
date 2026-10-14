-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Function
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  stable
-- Portability :  portable
--
-- Simple combinators working solely on and with functions.

module Data.Function (
	-- * "Prelude" re-exports
	id, const, (.), flip, ($),
	-- * Other combinators
	(&), on, fix, applyWhen
	) where

infixl 0 `on`
infixl 1 &

-- | @'fix' f@ is the least fixed point of the function @f@.
fix :: (a -> a) -> a
fix f = let x = f x in x

-- | @(op \`on\` f) x y = f x \`op\` f y@.
on :: (b -> b -> c) -> (a -> b) -> a -> a -> c
(.*.) `on` f = \x y -> f x .*. f y

-- | Reverse application: @x '&' f = f x@.
(&) :: a -> (a -> b) -> b
x & f = f x

-- | Apply a function when the condition holds.
applyWhen :: Bool -> (a -> a) -> a -> a
applyWhen True  f x = f x
applyWhen False _ x = x
