-----------------------------------------------------------------------------
-- |
-- Module      :  Control.Monad.Zip
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  experimental
-- Portability :  portable
--
-- Monads that can be zipped.

module Control.Monad.Zip (MonadZip(..)) where

import Control.Monad (liftM, liftM2)
import Cupid.Prelude (NonEmpty(..))

-- | Monads with a zip.  Minimal complete definition: 'mzip' or
-- 'mzipWith'.
class Monad m => MonadZip m where
	mzip :: m a -> m b -> m (a, b)
	mzip = mzipWith (,)

	mzipWith :: (a -> b -> c) -> m a -> m b -> m c
	mzipWith f ma mb = liftM (uncurry f) (mzip ma mb)

	munzip :: m (a, b) -> (m a, m b)
	munzip mab = (liftM fst mab, liftM snd mab)

instance MonadZip [] where
	mzip     = zip
	mzipWith = zipWith
	munzip   = unzip

instance MonadZip Maybe where
	mzipWith = liftM2

instance MonadZip NonEmpty where
	mzipWith f (a :| as) (b :| bs) = f a b :| zipWith f as bs
	munzip xs = (fmap fst xs, fmap snd xs)
