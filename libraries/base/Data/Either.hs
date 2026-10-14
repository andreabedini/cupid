{-# OPTIONS_GHC -fno-implicit-prelude #-}
-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Either
-- Copyright   :  (c) The University of Glasgow 2001
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  libraries@haskell.org
-- Stability   :  experimental
-- Portability :  portable
--
-- The Either type, and associated operations.
--
-----------------------------------------------------------------------------

module Data.Either (
   Either(..),
   either,	-- :: (a -> c) -> (b -> c) -> Either a b -> c
   lefts,	-- :: [Either a b] -> [a]
   rights,	-- :: [Either a b] -> [b]
   isLeft,	-- :: Either a b -> Bool
   isRight,	-- :: Either a b -> Bool
   fromLeft,	-- :: a -> Either a b -> a
   fromRight,	-- :: b -> Either a b -> b
   partitionEithers -- :: [Either a b] -> ([a],[b])
 ) where

-- | The 'Left' values of a list, in order.
lefts :: [Either a b] -> [a]
lefts x = [a | Left a <- x]

-- | The 'Right' values of a list, in order.
rights :: [Either a b] -> [b]
rights x = [a | Right a <- x]

-- | The 'Left' values and the 'Right' values of a list.
partitionEithers :: [Either a b] -> ([a],[b])
partitionEithers = foldr (either left right) ([],[])
  where
    left  a ~(l, r) = (a:l, r)
    right a ~(l, r) = (l, a:r)

isLeft :: Either a b -> Bool
isLeft (Left  _) = True
isLeft (Right _) = False

isRight :: Either a b -> Bool
isRight (Left  _) = False
isRight (Right _) = True

-- | The value of a 'Left', or the default.
fromLeft :: a -> Either a b -> a
fromLeft _ (Left a) = a
fromLeft a _        = a

-- | The value of a 'Right', or the default.
fromRight :: b -> Either a b -> b
fromRight _ (Right b) = b
fromRight b _         = b
