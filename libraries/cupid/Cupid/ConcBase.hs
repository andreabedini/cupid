-----------------------------------------------------------------------------
-- This implements Concurrent Haskell's "MVar"s as described in the paper
--
--   "Concurrent Haskell"
--   Simon Peyton Jones, Andrew Gordon and Sigbjorn Finne.
--   In Proceedings of the ACM Symposium on Principles of Programming
--   Languages,St Petersburg Beach, Florida, January 1996. 
--   http://www.dcs.gla.ac.uk/fp/authors/Simon_Peyton_Jones/
--     concurrent-haskell.ps
--
-- except that newEmptyMVar is the paper's newMVar.
--
-- Threads are scheduled cooperatively: a context switch happens only in one
-- of the primitives defined in this module.  A program such as
--
--     main = forkIO (write 'a') >> write 'b'
--       where write c = putChar c >> write c
--
-- prints only "bbbb...", because the parent keeps running after forkIO until
-- it blocks or yields.  This suffices for coroutines, but the usual
-- assumptions of fairness do not hold, and Control.Concurrent.Chan's
-- getChanContents fails with "no more threads" when it has to wait:
-- unsafeInterleaveIO runs its action under a scheduler of its own, where no
-- other thread can fill the channel.
-----------------------------------------------------------------------------
module Cupid.ConcBase(
	forkIO,
	MVar,
	newEmptyMVar, newMVar, takeMVar, tryTakeMVar, putMVar, tryPutMVar,
	isEmptyMVar,
        yield
	) where

import Cupid.Prelude(
	IO(..), IOResult(..), threadToIOResult,
	Exception(..), catchException, blockIO)
import Cupid.IORef

----------------------------------------------------------------
-- The interface
----------------------------------------------------------------

forkIO       :: IO () -> IO () -- Spawn a thread
yield        :: IO ()

newEmptyMVar :: IO (MVar a)
newMVar      :: a -> IO (MVar a)
takeMVar     :: MVar a -> IO a
putMVar      :: MVar a -> a -> IO ()
tryPutMVar   :: MVar a -> a -> IO Bool
tryTakeMVar  :: MVar a -> IO (Maybe a)

isEmptyMVar :: MVar a -> IO Bool

----------------------------------------------------------------
-- Implementation
----------------------------------------------------------------

kill :: IO a
kill = IO (\ s -> Cupid_DeadThread)

yield = IO (\ s -> Cupid_YieldThread (s ()))

-- add the continuation to the runnable list, and continue
continueIO :: IOResult -> IO ()
continueIO cc = IO (\ s -> Cupid_ForkThread (s ()) cc)

-- The new thread queues behind the parent, which keeps running, and has its
-- own success and error continuations.
forkIO m = continueIO (threadToIOResult (m `catchException` forkExnHandler))

forkExnHandler :: Exception -> IO a
forkExnHandler e = do
    putStr "\nThread raised exception: "
    putStr (show e)
    putStr "\n"           
    kill

newtype MVar a = MkMVar (IORef (MVarState a)) deriving Eq
data MVarState a
  = Full a [(a,()->IOResult)]
	-- a value and a list of value-thread pairs blocked waiting
	-- to write to the MVar.
	-- The ()-> part of the thread is because blocked threads have
	-- to be functions.
  | Empty [a -> IOResult]
	-- no value, just a list of threads waiting to receive a value

newEmptyMVar = fmap MkMVar (newIORef (Empty []))

newMVar x    = fmap MkMVar (newIORef (Full x []))

takeMVar (MkMVar v) = do
  state <- readIORef v
  case state of
    Full a [] -> do
      writeIORef v (Empty [])
      return a
    Full a ((a',t):ts) -> do
      writeIORef v (Full a' ts)
      continueIO (t ())		-- reschedule t
      return a
    Empty cs ->
      blockIO (\cc -> writeIORef v (Empty (cs ++ [cc])))

-- tryTakeMVar is a non-blocking takeMVar
tryTakeMVar (MkMVar v) = do
  state <- readIORef v
  case state of
    Full a [] -> do
      writeIORef v (Empty [])
      return (Just a)
    Full a ((a',t):ts) -> do
      writeIORef v (Full a' ts)
      continueIO (t ())		-- reschedule t
      return (Just a)
    Empty cs ->
      return Nothing

putMVar (MkMVar v) a = do
  state <- readIORef v
  case state of
    Full a' ts ->
      blockIO (\cc -> writeIORef v (Full a' (ts++[(a,cc)])))
    Empty [] ->
      writeIORef v (Full a [])
    Empty (c:cs) -> do
      writeIORef v (Empty cs)
      continueIO (c a)		-- reschedule the blocked thread

tryPutMVar (MkMVar v) a = do
  state <- readIORef v
  case state of
    Full _ _ ->
      return False
    Empty [] -> do
      writeIORef v (Full a [])
      return True
    Empty (c:cs) -> do
      writeIORef v (Empty cs)
      continueIO (c a)		-- reschedule the blocked thread
      return True

{- 
 Low-level op. for checking whether an MVar is filled-in or not.
 Notice that the boolean value returned  is just a snapshot of
 the state of the MVar. By the time you get to react on its result,
 the MVar may have been filled (or emptied) - so be extremely
 careful when using this operation.  

 Use tryTakeMVar instead if possible.
-}
isEmptyMVar (MkMVar v) = do
  state <- readIORef v
  case state of
    Full _ _ -> return False
    Empty _  -> return True

-----------------------------------------------------------------------------

