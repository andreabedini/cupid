{-# OPTIONS_GHC -fno-implicit-prelude #-}
-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Bool
-- Copyright   :  (c) The University of Glasgow 2001
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  libraries@haskell.org
-- Stability   :  experimental
-- Portability :  portable
--
-- The 'Bool' type and related functions.
--
-----------------------------------------------------------------------------

module Data.Bool (
   -- * Booleans
   Bool(..),
   -- ** Operations 
   (&&),	-- :: Bool -> Bool -> Bool
   (||),	-- :: Bool -> Bool -> Bool
   not,		-- :: Bool -> Bool
   otherwise,	-- :: Bool
   bool,	-- :: a -> a -> Bool -> a
  ) where

-- | Case analysis for 'Bool': @'bool' f t p@ is @t@ if @p@ holds and
-- @f@ otherwise.
bool :: a -> a -> Bool -> a
bool f _ False = f
bool _ t True  = t
