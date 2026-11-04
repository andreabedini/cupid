-----------------------------------------------------------------------------
-- Lazy State Thread module
-- 
-- Lazy state threads, after the PLDI '94 paper by John Launchbury and Simon
-- Peyton Jones.  lazyToStrictST and strictToLazyST convert to and from the
-- strict ST of Cupid.ST.
-----------------------------------------------------------------------------

module Cupid.LazyST 
	( ST
	, runST
	, unsafeInterleaveST
	, fixST 

	, lazyToStrictST
	, strictToLazyST
	) where

import qualified Cupid.ST as ST
import Control.Monad   

-----------------------------------------------------------------------------

newtype ST s a = ST (State s -> (a, State s))

unST :: ST s a -> State s -> (a, State s)
unST (ST f) = f

runST :: (forall s. ST s a) -> a
runST m = fst (unST m S)

unsafeInterleaveST :: ST s a -> ST s a
unsafeInterleaveST (ST m) = return (fst (m S))

fixST :: (a -> ST s a) -> ST s a
fixST f = ST (\s -> let (x,s') = unST (f x) s in (x,s'))

instance Functor (ST s) where
    fmap = liftM

instance Applicative (ST s) where
    pure a = ST (\s -> (a, s))
    (<*>)  = ap

instance Monad (ST s) where
    ST m >>= f = ST (\S -> let (a,s') = m S in unST (f a) s')

-----------------------------------------------------------------------------

data State s = S

-----------------------------------------------------------------------------

lazyToStrictST :: ST s a -> ST.ST s a
lazyToStrictST (ST m) = ST.ST (\k -> case m S of (a,S) -> k a)

strictToLazyST :: ST.ST s a -> ST s a
strictToLazyST (ST.ST m) = ST (\S -> m delay)

delay :: a -> (a, State s)
delay a = (a,S)

-----------------------------------------------------------------------------
