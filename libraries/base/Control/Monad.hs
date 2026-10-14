{-# OPTIONS_GHC -fno-implicit-prelude #-}
-----------------------------------------------------------------------------
-- |
-- Module      :  Control.Monad
-- Copyright   :  (c) The University of Glasgow 2001
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  libraries@haskell.org
-- Stability   :  provisional
-- Portability :  portable
--
-- The 'Functor', 'Monad' and 'MonadPlus' classes,
-- with some useful operations on monads.

module Control.Monad
    (
    -- * Functor and monad classes

      Functor(fmap, (<$))
    , Monad((>>=), (>>), return)
    , MonadFail(fail)
    , MonadPlus(mzero, mplus)

    -- * Functions

    -- ** Naming conventions
    -- $naming

    -- ** Basic functions from the "Prelude"

    , mapM          -- :: (Traversable t, Monad m) => (a -> m b) -> t a -> m (t b)
    , mapM_         -- :: (Foldable t, Monad m) => (a -> m b) -> t a -> m ()
    , forM          -- :: (Traversable t, Monad m) => t a -> (a -> m b) -> m (t b)
    , forM_         -- :: (Foldable t, Monad m) => t a -> (a -> m b) -> m ()
    , sequence      -- :: (Traversable t, Monad m) => t (m a) -> m (t a)
    , sequence_     -- :: (Foldable t, Monad m) => t (m a) -> m ()
    , (=<<)         -- :: (Monad m) => (a -> m b) -> m a -> m b
    , (>=>)         -- :: (Monad m) => (a -> m b) -> (b -> m c) -> (a -> m c)
    , (<=<)         -- :: (Monad m) => (b -> m c) -> (a -> m b) -> (a -> m c)
    , forever       -- :: (Applicative f) => f a -> f b
    , void          -- :: (Functor f) => f a -> f ()

    -- ** Generalisations of list functions

    , join          -- :: (Monad m) => m (m a) -> m a
    , msum          -- :: (Foldable t, MonadPlus m) => t (m a) -> m a
    , mfilter       -- :: (MonadPlus m) => (a -> Bool) -> m a -> m a
    , filterM       -- :: (Applicative m) => (a -> m Bool) -> [a] -> m [a]
    , mapAndUnzipM  -- :: (Applicative m) => (a -> m (b,c)) -> [a] -> m ([b], [c])
    , zipWithM      -- :: (Applicative m) => (a -> b -> m c) -> [a] -> [b] -> m [c]
    , zipWithM_     -- :: (Applicative m) => (a -> b -> m c) -> [a] -> [b] -> m ()
    , foldM         -- :: (Monad m) => (a -> b -> m a) -> a -> [b] -> m a 
    , foldM_        -- :: (Monad m) => (a -> b -> m a) -> a -> [b] -> m ()
    , replicateM    -- :: (Applicative m) => Int -> m a -> m [a]
    , replicateM_   -- :: (Applicative m) => Int -> m a -> m ()

    -- ** Conditional execution of monadic expressions

    , guard         -- :: (Alternative f) => Bool -> f ()
    , when          -- :: (Applicative f) => Bool -> f () -> f ()
    , unless        -- :: (Applicative f) => Bool -> f () -> f ()

    -- ** Monadic lifting operators

    , liftM         -- :: (Monad m) => (a -> b) -> (m a -> m b)
    , liftM2        -- :: (Monad m) => (a -> b -> c) -> (m a -> m b -> m c)
    , liftM3        -- :: ...
    , liftM4        -- :: ...
    , liftM5        -- :: ...

    , ap            -- :: (Monad m) => m (a -> b) -> m a -> m b

    -- ** Strict monadic functions

    , (<$!>)        -- :: (Monad m) => (a -> b) -> m a -> m b

    ) where

import Data.Maybe
import Cupid.Prelude (Alternative(..), MonadPlus(..), forM, forM_, msum)

infixr 1 >=>, <=<
infixl 4 <$!>

-- 'MonadPlus', the class of monads that also support choice and failure,
-- is defined in "Cupid.Prelude", with 'Alternative' as its superclass.

-- -----------------------------------------------------------------------------
-- Functions mandated by the Prelude

-- | @'guard' b@ is @'pure' ()@ if @b@ is 'True',
-- and 'empty' if @b@ is 'False'.
guard           :: (Alternative f) => Bool -> f ()
guard True      =  pure ()
guard False     =  empty

-- | This generalizes the list-based 'filter' function.
filterM          :: (Applicative m) => (a -> m Bool) -> [a] -> m [a]
filterM p        = foldr (\x -> liftA2 (\flg -> if flg then (x:) else id) (p x))
                         (pure [])

-- | Left-to-right Kleisli composition of monads.
(>=>)       :: Monad m => (a -> m b) -> (b -> m c) -> (a -> m c)
f >=> g     = \x -> f x >>= g

-- | Right-to-left Kleisli composition of monads.
(<=<)       :: Monad m => (b -> m c) -> (a -> m b) -> (a -> m c)
(<=<)       = flip (>=>)

-- | @'forever' act@ repeats the action infinitely.
forever     :: (Applicative f) => f a -> f b
forever a   = let a' = a *> a' in a'

-- | @'void' value@ discards or ignores the result of evaluation.
void        :: Functor f => f a -> f ()
void x      = () <$ x

-- -----------------------------------------------------------------------------
-- Other monad functions

-- | The 'join' function is the conventional monad join operator. It is used to
-- remove one level of monadic structure, projecting its bound argument into the
-- outer level.
join              :: (Monad m) => m (m a) -> m a
join x            =  x >>= id

-- | Direct 'MonadPlus' equivalent of 'filter'.
mfilter           :: (MonadPlus m) => (a -> Bool) -> m a -> m a
mfilter p ma      = do { a <- ma; if p a then return a else mzero }

-- | The 'mapAndUnzipM' function maps its first argument over a list, returning
-- the result as a pair of lists. This function is mainly used with complicated
-- data structures or a state-transforming monad.
mapAndUnzipM      :: (Applicative m) => (a -> m (b,c)) -> [a] -> m ([b], [c])
mapAndUnzipM f xs =  unzip <$> traverse f xs

-- | The 'zipWithM' function generalizes 'zipWith' to arbitrary applicative functors.
zipWithM          :: (Applicative m) => (a -> b -> m c) -> [a] -> [b] -> m [c]
zipWithM f xs ys  =  sequenceA (zipWith f xs ys)

-- | 'zipWithM_' is the extension of 'zipWithM' which ignores the final result.
zipWithM_         :: (Applicative m) => (a -> b -> m c) -> [a] -> [b] -> m ()
zipWithM_ f xs ys =  foldr (*>) (pure ()) (zipWith f xs ys)

{- | The 'foldM' function is analogous to 'foldl', except that its result is
encapsulated in a monad. Note that 'foldM' works from left-to-right over
the list arguments. This could be an issue where '(>>)' and the `folded
function' are not commutative.

>	foldM f a1 [x1, x2, ..., xm ]

==  

>	do
>	  a2 <- f a1 x1
>	  a3 <- f a2 x2
>	  ...
>	  f am xm

If right-to-left evaluation is required, the input list should be reversed.
-}

foldM             :: (Monad m) => (a -> b -> m a) -> a -> [b] -> m a
foldM _ a []      =  return a
foldM f a (x:xs)  =  f a x >>= \fax -> foldM f fax xs

-- | Like 'foldM', but discards the result.
foldM_            :: (Monad m) => (a -> b -> m a) -> a -> [b] -> m ()
foldM_ f a xs     = foldM f a xs >> return ()

-- | @'replicateM' n act@ performs the action @n@ times,
-- gathering the results.
replicateM        :: (Applicative m) => Int -> m a -> m [a]
replicateM n x    = loop n
  where loop k | k <= 0    = pure []
               | otherwise = liftA2 (:) x (loop (k - 1))

-- | Like 'replicateM', but discards the result.
replicateM_       :: (Applicative m) => Int -> m a -> m ()
replicateM_ n x   = loop n
  where loop k | k <= 0    = pure ()
               | otherwise = x *> loop (k - 1)

{- | Conditional execution of applicative expressions. For example, 

>	when debug (putStr "Debugging\n")

will output the string @Debugging\\n@ if the Boolean value @debug@ is 'True',
and otherwise do nothing.
-}

when              :: (Applicative f) => Bool -> f () -> f ()
when p s          =  if p then s else pure ()

-- | The reverse of 'when'.

unless            :: (Applicative f) => Bool -> f () -> f ()
unless p s        =  if p then pure () else s

-- | Promote a function to a monad.
liftM   :: (Monad m) => (a1 -> r) -> m a1 -> m r
liftM f m1              = do { x1 <- m1; return (f x1) }

-- | Promote a function to a monad, scanning the monadic arguments from
-- left to right.  For example,
--
-- >    liftM2 (+) [0,1] [0,2] = [0,2,1,3]
-- >    liftM2 (+) (Just 1) Nothing = Nothing
--
liftM2  :: (Monad m) => (a1 -> a2 -> r) -> m a1 -> m a2 -> m r
liftM2 f m1 m2          = do { x1 <- m1; x2 <- m2; return (f x1 x2) }

-- | Promote a function to a monad, scanning the monadic arguments from
-- left to right (cf. 'liftM2').
liftM3  :: (Monad m) => (a1 -> a2 -> a3 -> r) -> m a1 -> m a2 -> m a3 -> m r
liftM3 f m1 m2 m3       = do { x1 <- m1; x2 <- m2; x3 <- m3; return (f x1 x2 x3) }

-- | Promote a function to a monad, scanning the monadic arguments from
-- left to right (cf. 'liftM2').
liftM4  :: (Monad m) => (a1 -> a2 -> a3 -> a4 -> r) -> m a1 -> m a2 -> m a3 -> m a4 -> m r
liftM4 f m1 m2 m3 m4    = do { x1 <- m1; x2 <- m2; x3 <- m3; x4 <- m4; return (f x1 x2 x3 x4) }

-- | Promote a function to a monad, scanning the monadic arguments from
-- left to right (cf. 'liftM2').
liftM5  :: (Monad m) => (a1 -> a2 -> a3 -> a4 -> a5 -> r) -> m a1 -> m a2 -> m a3 -> m a4 -> m a5 -> m r
liftM5 f m1 m2 m3 m4 m5 = do { x1 <- m1; x2 <- m2; x3 <- m3; x4 <- m4; x5 <- m5; return (f x1 x2 x3 x4 x5) }

{- | In many situations, the 'liftM' operations can be replaced by uses of
'ap', which promotes function application. 

>	return f `ap` x1 `ap` ... `ap` xn

is equivalent to 

>	liftMn f x1 x2 ... xn

-}

ap                :: (Monad m) => m (a -> b) -> m a -> m b
ap                =  liftM2 id

-- | Strict version of '<$>'.
(<$!>)            :: Monad m => (a -> b) -> m a -> m b
f <$!> m          = do { x <- m; return $! f x }

{- $naming

The functions in this library use the following naming conventions: 

* A postfix \'@M@\' always stands for a function in the Kleisli category:
  The monad type constructor @m@ is added to function results
  (modulo currying) and nowhere else.  So, for example, 

>  filter  ::              (a ->   Bool) -> [a] ->   [a]
>  filterM :: (Monad m) => (a -> m Bool) -> [a] -> m [a]

* A postfix \'@_@\' changes the result type from @(m a)@ to @(m ())@.
  Thus, for example: 

>  sequence  :: Monad m => [m a] -> m [a] 
>  sequence_ :: Monad m => [m a] -> m () 

* A prefix \'@m@\' generalizes an existing function to a monadic form.
  Thus, for example: 

>  sum  :: Num a       => [a]   -> a
>  msum :: MonadPlus m => [m a] -> m a

-}
