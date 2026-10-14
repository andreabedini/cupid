{-# OPTIONS_GHC -fno-implicit-prelude #-}
-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Tuple
-- Copyright   :  (c) The University of Glasgow 2001
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  libraries@haskell.org
-- Stability   :  experimental
-- Portability :  portable
--
-- The tuple data types, and associated functions.
--
-----------------------------------------------------------------------------

module Data.Tuple
  ( fst		-- :: (a,b) -> a
  , snd		-- :: (a,b) -> a
  , curry	-- :: ((a, b) -> c) -> a -> b -> c
  , uncurry	-- :: (a -> b -> c) -> ((a, b) -> c)
  , swap	-- :: (a,b) -> (b,a)
















  )
    where




























default ()		-- Double isn't available yet





























































































































































































-- ---------------------------------------------------------------------------
-- Standard functions over tuples

-- | Swap the components of a pair.
swap :: (a,b) -> (b,a)
swap (a,b) = (b,a)
