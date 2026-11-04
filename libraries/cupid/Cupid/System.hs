-----------------------------------------------------------------------------
-- Standard Library: System operations
-----------------------------------------------------------------------------

module Cupid.System (
	getArgs, getProgName, withArgs, withProgName, getEnv,
	system
	) where

import Cupid.Prelude( ExitCode(..), catchException, throw )

-- At the prompt these start as [] and "Cupid"; cupid FILE ARGS sets them to
-- FILE and ARGS.  withArgs and withProgName change them for one action.
primitive getArgs     "primGetArgs"     :: IO [String]
primitive getProgName "primGetProgName" :: IO String

primitive setArgs     "primSetArgs"     :: [String] -> IO ()
primitive setProgName "primSetProgName" :: String -> IO ()

-- Run an action with a value temporarily overridden
-- (a special case of Control.Exception.bracket)
with :: IO a -> (a -> IO ()) -> a -> IO b -> IO b
with getVal setVal newVal act = do
    oldVal <- getVal
    setVal newVal
    r <- act `catchException` \e -> setVal oldVal >> throw e
    setVal oldVal
    return r

withArgs :: [String] -> IO a -> IO a
withArgs = with getArgs setArgs

withProgName :: String -> IO a -> IO a
withProgName = with getProgName setProgName

primitive getEnv            :: String -> IO String

system                      :: String -> IO ExitCode
system s                     = do r <- primSystem s
                                  return (toExitCode r)

primitive primSystem        :: String -> IO Int

toExitCode                  :: Int -> ExitCode
toExitCode 0                 = ExitSuccess
toExitCode n                 = ExitFailure n

-----------------------------------------------------------------------------
