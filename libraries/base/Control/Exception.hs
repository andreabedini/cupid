-----------------------------------------------------------------------------
-- |
-- Module      :  Control.Exception
-- Copyright   :  (c) The University of Glasgow 2001
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  libraries@haskell.org
-- Stability   :  experimental
-- Portability :  non-portable (extended exceptions)
--
-- This module provides support for raising and catching both built-in
-- and user-defined exceptions, with the 'Exception' class of GHC's base:
-- any type with 'Typeable' and 'Show', which every type built from type
-- constructors has, can be made an exception by an empty instance.
--
-- Underneath is the exception type of "Cupid.Prelude", whose
-- constructors the interpreter raises for its own errors and the top
-- level reports: 'throw' gives it the built-in exceptions as its own
-- constructors and carries any other in 'UserException', and 'catch'
-- turns it back into a 'SomeException'.
--
-- In addition to exceptions thrown by 'IO' operations, exceptions may
-- be thrown by pure code (imprecise exceptions) or by external events
-- (asynchronous exceptions), but may only be caught in the 'IO' monad.
-- For more details, see:
--
--  * /A semantics for imprecise exceptions/, by Simon Peyton Jones,
--    Alastair Reid, Tony Hoare, Simon Marlow, Fergus Henderson,
--    in /PLDI'99/.
--
--  * /Asynchronous exceptions in Haskell/, by Simon Marlow, Simon Peyton
--    Jones, Andy Moran and John Reppy, in /PLDI'01/.
--
-----------------------------------------------------------------------------

module Control.Exception (
	-- * The Exception class
	SomeException(..),
	Exception(..),
	-- ** Exceptions of the interpreter and the libraries
	IOException,
	ArithException(..),
	ArrayException(..),
	AssertionFailed(..),
	SomeAsyncException(..),
	AsyncException(..),
	asyncExceptionToException, asyncExceptionFromException,
	NonTermination(..),
	BlockedIndefinitelyOnMVar(..),
	Deadlock(..),
	NoMethodError(..),
	PatternMatchFail(..),
	RecConError(..),
	RecSelError(..),
	RecUpdError(..),
	ErrorCall(..),
	TypeError(..),
	-- * Throwing exceptions
	throw,		-- :: Exception e => e -> a
	throwIO,	-- :: Exception e => e -> IO a
	ioError,	-- :: IOError -> IO a
	-- * Catching exceptions
	catch,		-- :: Exception e => IO a -> (e -> IO a) -> IO a
	catches, Handler(..),
	catchJust,
	handle,
	handleJust,
	try,		-- :: Exception e => IO a -> IO (Either e a)
	tryJust,
	evaluate,	-- :: a -> IO a
	mapException,
	-- * Asynchronous exceptions
	-- | Cupid's threads are not preemptive and cannot be interrupted by
	-- another, so masking does nothing.
	mask, mask_,
	uninterruptibleMask, uninterruptibleMask_,
	MaskingState(..), getMaskingState,
	interruptible, allowInterrupt,
	block, unblock,
	-- * Assertions
	assert,		-- :: Bool -> a -> a
	-- * Utilities
	bracket,	-- :: IO a -> (a -> IO b) -> (a -> IO c) -> IO c
	bracket_,	-- :: IO a -> IO b -> IO c -> IO c
	bracketOnError,
	finally,	-- :: IO a -> IO b -> IO a
	onException,	-- :: IO a -> IO b -> IO a
	-- * The exceptions of the 2006 library
	throwDyn, catchDyn
  ) where

import Prelude
import qualified Cupid.Prelude as Old
import Cupid.Prelude (IOException, ArithException(..), ArrayException(..),
	AsyncException(..), ExitCode(..), Dynamic)
import qualified Cupid.Exception as Old (catchException, evaluate)
import Data.Dynamic (toDyn, fromDynamic)
import Data.Typeable (Typeable, cast)
import System.IO.Unsafe (unsafePerformIO)

-- ---------------------------------------------------------------------------
-- The Exception class

-- | Any exception, the root of the hierarchy of exception types.
data SomeException = forall e . Exception e => SomeException e

instance Show SomeException where
	showsPrec p (SomeException e) = showsPrec p e

-- | The class of types that can be thrown and caught.  An empty instance
-- makes a type an exception at the top of the hierarchy; 'toException'
-- and 'fromException' can be defined to put it lower.
class (Typeable e, Show e) => Exception e where
	toException   :: e -> SomeException
	fromException :: SomeException -> Maybe e
	-- | The message for an exception that nothing catches.
	displayException :: e -> String

	toException = SomeException
	fromException (SomeException e) = cast e
	displayException = show

instance Exception SomeException where
	toException se = se
	fromException = Just
	displayException (SomeException e) = displayException e

-- ---------------------------------------------------------------------------
-- The exceptions of the interpreter and the libraries

-- They show as the exception type underneath shows them, so that a caught
-- exception reads as an uncaught one is reported.
showTagged :: String -> String -> ShowS
showTagged tag msg =
	showString tag . (if null msg then id else showString ": " . showString msg)

instance Exception IOException
instance Exception ArithException
instance Exception ArrayException
instance Exception ExitCode

-- | Raised by 'assert'.
newtype AssertionFailed = AssertionFailed String

instance Show AssertionFailed where
	showsPrec _ (AssertionFailed err) = showTagged "assertion failed" err

instance Exception AssertionFailed

-- | The superclass of asynchronous exceptions.
data SomeAsyncException = forall e . Exception e => SomeAsyncException e

instance Show SomeAsyncException where
	showsPrec p (SomeAsyncException e) = showsPrec p e

instance Exception SomeAsyncException

-- | 'toException' for an asynchronous exception.
asyncExceptionToException :: Exception e => e -> SomeException
asyncExceptionToException = toException . SomeAsyncException

-- | 'fromException' for an asynchronous exception.
asyncExceptionFromException :: Exception e => SomeException -> Maybe e
asyncExceptionFromException x = do
	SomeAsyncException a <- fromException x
	cast a

instance Exception AsyncException where
	toException = asyncExceptionToException
	fromException = asyncExceptionFromException

-- | An expression that the interpreter found to loop.
data NonTermination = NonTermination

instance Show NonTermination where
	showsPrec _ NonTermination = showString "<<loop>>"

instance Exception NonTermination

-- | A thread waits on an 'MVar' that nothing will fill.
data BlockedIndefinitelyOnMVar = BlockedIndefinitelyOnMVar

instance Show BlockedIndefinitelyOnMVar where
	showsPrec _ BlockedIndefinitelyOnMVar =
		showString "thread blocked indefinitely in an MVar operation"

instance Exception BlockedIndefinitelyOnMVar

-- | Every thread is blocked.
data Deadlock = Deadlock

instance Show Deadlock where
	showsPrec _ Deadlock = showString "<<deadlock>>"

instance Exception Deadlock

-- | A class method with no definition was called.
newtype NoMethodError = NoMethodError String

instance Show NoMethodError where
	showsPrec _ (NoMethodError err) = showTagged "undefined member" err

instance Exception NoMethodError

-- | No pattern matched.
newtype PatternMatchFail = PatternMatchFail String

instance Show PatternMatchFail where
	showsPrec _ (PatternMatchFail err) = showTagged "pattern match failure" err

instance Exception PatternMatchFail

-- | A record was built without a strict field.
newtype RecConError = RecConError String

instance Show RecConError where
	showsPrec _ (RecConError err) = showTagged "undefined field" err

instance Exception RecConError

-- | A field was selected from a constructor without it.
newtype RecSelError = RecSelError String

instance Show RecSelError where
	showsPrec _ (RecSelError err) = showTagged "select of missing field" err

instance Exception RecSelError

-- | A field was updated in a constructor without it.
newtype RecUpdError = RecUpdError String

instance Show RecUpdError where
	showsPrec _ (RecUpdError err) = showTagged "update of missing field" err

instance Exception RecUpdError

-- | Raised by 'error'.
newtype ErrorCall = ErrorCall String
	deriving (Eq, Ord)

instance Show ErrorCall where
	showsPrec _ (ErrorCall err) = showString err

instance Exception ErrorCall

-- | A type error deferred to run time.  Cupid reports type errors when
-- it loads a module, so it never raises one.
newtype TypeError = TypeError String

instance Show TypeError where
	showsPrec _ (TypeError err) = showString err

instance Exception TypeError

-- An exception of the 2006 library's throwDyn, which no other type fits.
newtype DynException = DynException Dynamic

instance Show DynException where
	showsPrec _ _ = showString "unknown exception"

instance Exception DynException

-- ---------------------------------------------------------------------------
-- The exception type underneath

-- What 'throw' raises: the interpreter's own exceptions as themselves, so
-- that the top level reports them and the interpreter's handlers know
-- them, and any other in UserException.
toOld :: SomeException -> Old.Exception
toOld se
	| Just (ErrorCall s) <- fromException se         = Old.ErrorCall s
	| Just e <- fromException se                      = Old.IOException e
	| Just e <- fromException se                      = Old.ArithException e
	| Just e <- fromException se                      = Old.ArrayException e
	| Just e <- fromException se                      = Old.AsyncException e
	| Just c <- fromException se                      = Old.ExitException c
	| Just (AssertionFailed s) <- fromException se   = Old.AssertionFailed s
	| Just (PatternMatchFail s) <- fromException se  = Old.PatternMatchFail s
	| Just (NoMethodError s) <- fromException se     = Old.NoMethodError s
	| Just (RecConError s) <- fromException se       = Old.RecConError s
	| Just (RecSelError s) <- fromException se       = Old.RecSelError s
	| Just (RecUpdError s) <- fromException se       = Old.RecUpdError s
	| Just NonTermination <- fromException se        = Old.NonTermination
	| Just Deadlock <- fromException se              = Old.Deadlock
	| Just BlockedIndefinitelyOnMVar <- fromException se = Old.BlockedOnDeadMVar
	| Just (DynException d) <- fromException se      = Old.DynException d
	| otherwise = Old.UserException (displaySome se) (toDyn se)
  where
	displaySome (SomeException e) = displayException e

-- What 'catch' is given, the inverse of toOld.
fromOld :: Old.Exception -> SomeException
fromOld ex = case ex of
	Old.ArithException e    -> toException e
	Old.ArrayException e    -> toException e
	Old.AssertionFailed s   -> toException (AssertionFailed s)
	Old.AsyncException e    -> toException e
	Old.BlockedOnDeadMVar   -> toException BlockedIndefinitelyOnMVar
	Old.Deadlock            -> toException Deadlock
	Old.DynException d      -> toException (DynException d)
	Old.ErrorCall s         -> toException (ErrorCall s)
	Old.ExitException c     -> toException c
	Old.IOException e       -> toException e
	Old.NoMethodError s     -> toException (NoMethodError s)
	Old.NonTermination      -> toException NonTermination
	Old.PatternMatchFail s  -> toException (PatternMatchFail s)
	Old.RecConError s       -> toException (RecConError s)
	Old.RecSelError s       -> toException (RecSelError s)
	Old.RecUpdError s       -> toException (RecUpdError s)
	Old.UserException s d   -> case fromDynamic d of
		Just se -> se
		Nothing -> toException (ErrorCall s)

-- ---------------------------------------------------------------------------
-- Throwing exceptions

-- | Throw an exception from pure code.
throw :: Exception e => e -> a
throw e = Old.throw (toOld (toException e))

-- | Throw an exception in the 'IO' monad, ordered with respect to the
-- other actions, unlike @'evaluate' . 'throw'@.
throwIO :: Exception e => e -> IO a
throwIO e = Old.IO (\_ -> throw e)

-- ---------------------------------------------------------------------------
-- Catching exceptions

-- | Run an action, handing an exception of type @e@ that it throws to the
-- handler; one of any other type passes through.
catch :: Exception e => IO a -> (e -> IO a) -> IO a
catch act h = Old.catchException act $ \old ->
	case fromException (fromOld old) of
		Just e  -> h e
		Nothing -> Old.throw old

-- | A handler for 'catches'.
data Handler a = forall e . Exception e => Handler (e -> IO a)

instance Functor Handler where
	fmap f (Handler h) = Handler (fmap f . h)

-- | Catch with the first of several handlers whose type the exception has.
catches :: IO a -> [Handler a] -> IO a
catches act handlers = Old.catchException act $ \old ->
	let se = fromOld old
	    try1 (Handler h) rest = case fromException se of
		Just e  -> h e
		Nothing -> rest
	in foldr try1 (Old.throw old) handlers

-- | 'catch' with a predicate that selects the exceptions to handle.
catchJust :: Exception e => (e -> Maybe b) -> IO a -> (b -> IO a) -> IO a
catchJust p act handler = catch act handler'
  where
	handler' e = case p e of
		Nothing -> throwIO e
		Just b  -> handler b

-- | 'catch' with its arguments flipped.
handle :: Exception e => (e -> IO a) -> IO a -> IO a
handle = flip catch

-- | 'catchJust' with its handler first.
handleJust :: Exception e => (e -> Maybe b) -> (b -> IO a) -> IO a -> IO a
handleJust p = flip (catchJust p)

-- | Run an action, returning the exception of type @e@ it throws, if any.
try :: Exception e => IO a -> IO (Either e a)
try act = catch (act >>= \v -> return (Right v)) (\e -> return (Left e))

-- | 'try' with a predicate that selects the exceptions to return.
tryJust :: Exception e => (e -> Maybe b) -> IO a -> IO (Either b a)
tryJust p act = do
	r <- try act
	case r of
		Right v -> return (Right v)
		Left e  -> case p e of
			Nothing -> throwIO e
			Just b  -> return (Left b)

-- | Evaluate a value to weak head normal form, raising any exception
-- then, in the 'IO' monad.
evaluate :: a -> IO a
evaluate = Old.evaluate

-- | Apply a function to the exception, of type @e1@, that a value raises.
mapException :: (Exception e1, Exception e2) => (e1 -> e2) -> a -> a
mapException f v = unsafePerformIO (catch (evaluate v) (\x -> throwIO (f x)))

-- ---------------------------------------------------------------------------
-- Asynchronous exceptions

-- | Whether asynchronous exceptions can interrupt the current thread.
data MaskingState = Unmasked | MaskedInterruptible | MaskedUninterruptible
	deriving (Eq, Show)

-- | Run an action with asynchronous exceptions masked, passing it a
-- function that unmasks them again.  Nothing can interrupt one of
-- Cupid's threads, so this only runs the action.
mask :: ((IO a -> IO a) -> IO b) -> IO b
mask io = io id

mask_ :: IO a -> IO a
mask_ io = mask (\_ -> io)

uninterruptibleMask :: ((IO a -> IO a) -> IO b) -> IO b
uninterruptibleMask = mask

uninterruptibleMask_ :: IO a -> IO a
uninterruptibleMask_ = mask_

getMaskingState :: IO MaskingState
getMaskingState = return Unmasked

interruptible :: IO a -> IO a
interruptible act = act

allowInterrupt :: IO ()
allowInterrupt = return ()

-- | The 2006 names of 'mask_' and of unmasking.
block, unblock :: IO a -> IO a
block act = act
unblock act = act

-- ---------------------------------------------------------------------------
-- Assertions

-- | @'assert' p x@ is @x@ if @p@ holds; Cupid does not check assertions.
assert :: Bool -> a -> a
assert _ x = x

-- ---------------------------------------------------------------------------
-- Utilities

-- | Acquire a resource, use it, and release it however the use ends.
bracket :: IO a -> (a -> IO b) -> (a -> IO c) -> IO c
bracket before after thing = mask $ \restore -> do
	a <- before
	r <- restore (thing a) `onException` after a
	_ <- after a
	return r

-- | Run an action, and then the other however the first ends.
finally :: IO a -> IO b -> IO a
a `finally` sequel = mask $ \restore -> do
	r <- restore a `onException` sequel
	_ <- sequel
	return r

-- | 'bracket' with the resource unused.
bracket_ :: IO a -> IO b -> IO c -> IO c
bracket_ before after thing = bracket before (const after) (const thing)

-- | 'bracket' that releases the resource only if the use throws.
bracketOnError :: IO a -> (a -> IO b) -> (a -> IO c) -> IO c
bracketOnError before after thing = mask $ \restore -> do
	a <- before
	restore (thing a) `onException` after a

-- | Run the second action if the first throws, and throw again.
onException :: IO a -> IO b -> IO a
onException io what = Old.catchException io $ \old -> do
	_ <- what
	Old.throw old

-- ---------------------------------------------------------------------------
-- The exceptions of the 2006 library

-- | Throw a value of any 'Typeable' type.
throwDyn :: Typeable e => e -> b
throwDyn e = Old.throw (Old.DynException (toDyn e))

-- | Catch a value that 'throwDyn' threw.
catchDyn :: Typeable e => IO a -> (e -> IO a) -> IO a
catchDyn m k = Old.catchException m h
  where
	h ex@(Old.DynException dyn) = case fromDynamic dyn of
		Just e  -> k e
		Nothing -> Old.throw ex
	h ex = Old.throw ex
