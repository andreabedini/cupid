{-# OPTIONS_GHC -cpp -fffi #-}
-----------------------------------------------------------------------------
-- |
-- Module      :  System.Process.Internals
-- Copyright   :  (c) The University of Glasgow 2004
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  libraries@haskell.org
-- Stability   :  experimental
-- Portability :  portable
--
-- Operations for creating and interacting with sub-processes.
--
-----------------------------------------------------------------------------

-- #hide
module System.Process.Internals (
	 pPrPr_disableITimers, c_execvpe,
	ignoreSignal, defaultSignal,
	withFilePathException, withCEnvironment
  ) where

import Cupid.SysConfig
import Prelude -- necessary to get dependencies right

import System.Posix.Types ( CPid )
import System.IO 	( Handle )

import System.Exit	( ExitCode )
import Data.Maybe	( fromMaybe )
import Cupid.Exception	( Exception(..), IOException(..) )

import Control.Concurrent
import System.IO.Error ( catchIOError )
import Foreign.C
import Foreign

{-# CFILES cbits/execvpe.c  #-}

 

 
 
 
 
 
 
 
 
 

 

 

 

-- ----------------------------------------------------------------------------

-- this function disables the itimer, which would otherwise cause confusing
-- signals to be sent to the new process.
foreign import ccall unsafe "HsBase.h pPrPr_disableITimers"
  pPrPr_disableITimers :: IO ()

foreign import ccall unsafe "HsBase.h execvpe"
  c_execvpe :: CString -> Ptr CString -> Ptr CString -> IO CInt

ignoreSignal  = const_SIG_IGN :: CLong
defaultSignal = const_SIG_DFL :: CLong

-- ----------------------------------------------------------------------------
-- Utils

withFilePathException :: FilePath -> IO a -> IO a
withFilePathException fpath act = catchIOError act mapEx
  where
    mapEx (IOError h iot fun str _) = ioError (IOError h iot fun str (Just fpath))

withCEnvironment :: [(String,String)] -> (Ptr CString  -> IO a) -> IO a
withCEnvironment env act =
  let env' = map (\(name, val) -> name ++ ('=':val)) env 
  in withMany withCString env' (\pEnv -> withArray0 nullPtr pEnv act)

