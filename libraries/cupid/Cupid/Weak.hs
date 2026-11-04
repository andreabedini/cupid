-- The (key,value) form of weak pointers.
--
-- Programmers should call runFinalizer at regular intervals so that
-- finalizers are scheduled for execution; finalizerWaiting and
-- runAllFinalizers give further control.

module Cupid.Weak(Weak,
	    mkWeak, deRefWeak, finalize, replaceFinalizer,
	    runFinalizer, finalizerWaiting, runAllFinalizers ) where

data Weak a

primitive mkWeak    :: k -> v -> Maybe (IO ()) -> IO (Weak v)
primitive deRefWeak :: Weak v -> IO (Maybe v)
primitive replaceFinalizer :: Weak v -> Maybe (IO ()) -> IO (Maybe (IO ()))
primitive finalize  :: Weak v -> IO ()
primitive weakPtrEq :: Weak a -> Weak a -> Bool

instance Eq (Weak a) where
  (==) = weakPtrEq

primitive runFinalizer     :: IO ()
primitive finalizerWaiting :: IO Bool

runAllFinalizers    :: IO ()
runAllFinalizers     = do waiting <- finalizerWaiting
			  if waiting then do runFinalizer
					     runAllFinalizers
				     else return ()
