-----------------------------------------------------------------------------
-- |
-- Module      :  Control.Monad.Fail
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  provisional
-- Portability :  portable
--
-- The class of monads that can fail with a message.  It is defined in
-- the Prelude; a @do@ block calls 'fail' when the pattern of one of its
-- generators does not match.

module Control.Monad.Fail (MonadFail(fail)) where
