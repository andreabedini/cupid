{-# OPTIONS_NHC98 -prelude #-}
-----------------------------------------------------------------------------
-- |
-- Module      :  Control.Monad.Instances
-- Copyright   :  (c) The University of Glasgow 2001
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Maintainer  :  libraries@haskell.org
-- Stability   :  provisional
-- Portability :  portable
--
-- Formerly the 'Functor' and 'Monad' instances for @(->) r@ and the
-- 'Functor' instances for @(,) a@ and @'Either' a@, which are now in the
-- Prelude.  The module is kept so that code importing it still loads.

module Control.Monad.Instances (Functor(..),Monad(..)) where
