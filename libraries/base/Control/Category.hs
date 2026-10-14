-----------------------------------------------------------------------------
-- |
-- Module      :  Control.Category
-- License     :  BSD-style (see the LICENSE file in the distribution)
--
-- Stability   :  experimental
-- Portability :  portable
--
-- Categories: composition with an identity, generalising functions.  The
-- superclass of 'Control.Arrow.Arrow'.

module Control.Category (
	Category(..),
	(<<<), (>>>)
	) where

import qualified Prelude

infixr 9 .
infixr 1 >>>, <<<

-- | A class for categories.  Instances should satisfy the laws
--
-- [/right identity/] @f '.' 'id'  =  f@
--
-- [/left identity/]  @'id' '.' f  =  f@
--
-- [/associativity/]  @f '.' (g '.' h)  =  (f '.' g) '.' h@
class Category cat where
	-- | The identity morphism.
	id :: cat a a

	-- | Morphism composition.
	(.) :: cat b c -> cat a b -> cat a c

instance Category (->) where
	id = Prelude.id
	(.) = (Prelude..)

-- | Right-to-left composition.
(<<<) :: Category cat => cat b c -> cat a b -> cat a c
(<<<) = (.)

-- | Left-to-right composition.
(>>>) :: Category cat => cat a b -> cat b c -> cat a c
f >>> g = g . f
