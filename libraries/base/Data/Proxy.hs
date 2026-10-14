-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Proxy
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  experimental
-- Portability :  portable
--
-- A value standing for a type, to pass a type as an argument.  Cupid's
-- kinds are not polymorphic, so a 'Proxy' stands for a type of kind
-- 'Data.Kind.Type' only: @Proxy :: Proxy Int@, not @Proxy :: Proxy Maybe@.

module Data.Proxy (Proxy(..), asProxyTypeOf) where

-- | A proxy for the type @t@, carrying no value of it.
data Proxy t = Proxy
	deriving (Eq, Ord, Show, Read, Bounded)

instance Enum (Proxy t) where
	fromEnum _ = 0
	toEnum 0 = Proxy
	toEnum _ = error "Proxy.toEnum: 0 expected"

instance Semigroup (Proxy t) where
	_ <> _ = Proxy

instance Monoid (Proxy t) where
	mempty = Proxy

instance Functor Proxy where
	fmap _ _ = Proxy

instance Applicative Proxy where
	pure _ = Proxy
	_ <*> _ = Proxy

instance Monad Proxy where
	_ >>= _ = Proxy

instance Foldable Proxy where
	foldMap _ _ = mempty
	foldr _ z _ = z
	length _ = 0
	null _ = True

instance Traversable Proxy where
	traverse _ _ = pure Proxy

-- | 'const' with its second argument's type fixed by the proxy.
asProxyTypeOf :: a -> proxy a -> a
asProxyTypeOf = const
