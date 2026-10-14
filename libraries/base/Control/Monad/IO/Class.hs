-----------------------------------------------------------------------------
-- |
-- Module      :  Control.Monad.IO.Class
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  experimental
-- Portability :  portable
--
-- The class of monads into which 'IO' computations can be lifted.

module Control.Monad.IO.Class (MonadIO(..)) where

-- | Monads in which 'IO' computations may be embedded, such as any stack
-- of monad transformers over 'IO'.  Instances should satisfy
--
-- * @'liftIO' . 'return' = 'return'@
--
-- * @'liftIO' (m >>= f) = 'liftIO' m >>= ('liftIO' . f)@
class Monad m => MonadIO m where
	-- | Lift a computation from the 'IO' monad.
	liftIO :: IO a -> m a

instance MonadIO IO where
	liftIO = id
