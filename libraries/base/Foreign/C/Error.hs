{-# OPTIONS_GHC -fno-implicit-prelude -#include "HsBase.h" #-}
-----------------------------------------------------------------------------
-- |
-- Module      :  Foreign.C.Error
-- Copyright   :  (c) The FFI task force 2001
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  ffi@haskell.org
-- Stability   :  provisional
-- Portability :  portable
--
-- C-specific Marshalling support: Handling of C \"errno\" error codes.
--
-----------------------------------------------------------------------------

module Foreign.C.Error (

  -- * Haskell representations of @errno@ values

  Errno(..),		-- instance: Eq

  -- ** Common @errno@ symbols
  -- | Different operating systems and\/or C libraries often support
  -- different values of @errno@.  This module defines the common values,
  -- but due to the open definition of 'Errno' users may add definitions
  -- which are not predefined.
  eOK, e2BIG, eACCES, eADDRINUSE, eADDRNOTAVAIL, eADV, eAFNOSUPPORT, eAGAIN, 
  eALREADY, eBADF, eBADMSG, eBADRPC, eBUSY, eCHILD, eCOMM, eCONNABORTED, 
  eCONNREFUSED, eCONNRESET, eDEADLK, eDESTADDRREQ, eDIRTY, eDOM, eDQUOT, 
  eEXIST, eFAULT, eFBIG, eFTYPE, eHOSTDOWN, eHOSTUNREACH, eIDRM, eILSEQ, 
  eINPROGRESS, eINTR, eINVAL, eIO, eISCONN, eISDIR, eLOOP, eMFILE, eMLINK, 
  eMSGSIZE, eMULTIHOP, eNAMETOOLONG, eNETDOWN, eNETRESET, eNETUNREACH, 
  eNFILE, eNOBUFS, eNODATA, eNODEV, eNOENT, eNOEXEC, eNOLCK, eNOLINK, 
  eNOMEM, eNOMSG, eNONET, eNOPROTOOPT, eNOSPC, eNOSR, eNOSTR, eNOSYS, 
  eNOTBLK, eNOTCONN, eNOTDIR, eNOTEMPTY, eNOTSOCK, eNOTTY, eNXIO, 
  eOPNOTSUPP, ePERM, ePFNOSUPPORT, ePIPE, ePROCLIM, ePROCUNAVAIL, 
  ePROGMISMATCH, ePROGUNAVAIL, ePROTO, ePROTONOSUPPORT, ePROTOTYPE, 
  eRANGE, eREMCHG, eREMOTE, eROFS, eRPCMISMATCH, eRREMOTE, eSHUTDOWN, 
  eSOCKTNOSUPPORT, eSPIPE, eSRCH, eSRMNT, eSTALE, eTIME, eTIMEDOUT, 
  eTOOMANYREFS, eTXTBSY, eUSERS, eWOULDBLOCK, eXDEV,

  -- ** 'Errno' functions
                        -- :: Errno
  isValidErrno,		-- :: Errno -> Bool

  -- access to the current thread's "errno" value
  --
  getErrno,             -- :: IO Errno
  resetErrno,           -- :: IO ()

  -- conversion of an "errno" value into IO error
  --
  errnoToIOError,       -- :: String       -- location
                        -- -> Errno        -- errno
                        -- -> Maybe Handle -- handle
                        -- -> Maybe String -- filename
                        -- -> IOError

  -- throw current "errno" value
  --
  throwErrno,           -- ::                String               -> IO a

  -- ** Guards for IO operations that may fail

  throwErrnoIf,         -- :: (a -> Bool) -> String -> IO a       -> IO a
  throwErrnoIf_,        -- :: (a -> Bool) -> String -> IO a       -> IO ()
  throwErrnoIfRetry,    -- :: (a -> Bool) -> String -> IO a       -> IO a
  throwErrnoIfRetry_,   -- :: (a -> Bool) -> String -> IO a       -> IO ()
  throwErrnoIfMinus1,   -- :: Num a 
			-- =>                String -> IO a       -> IO a
  throwErrnoIfMinus1_,  -- :: Num a 
			-- =>                String -> IO a       -> IO ()
  throwErrnoIfMinus1Retry,  
			-- :: Num a 
			-- =>                String -> IO a       -> IO a
  throwErrnoIfMinus1Retry_,  
			-- :: Num a 
			-- =>                String -> IO a       -> IO ()
  throwErrnoIfNull,	-- ::                String -> IO (Ptr a) -> IO (Ptr a)
  throwErrnoIfNullRetry,-- ::                String -> IO (Ptr a) -> IO (Ptr a)

  throwErrnoIfRetryMayBlock, 
  throwErrnoIfRetryMayBlock_,
  throwErrnoIfMinus1RetryMayBlock,
  throwErrnoIfMinus1RetryMayBlock_,  
  throwErrnoIfNullRetryMayBlock
) where

-- this is were we get the CONST_XXX definitions from that configure
-- calculated for us
--
 

 
 
 
 
 
 
 
 
 

 

 

 

-- system dependent imports
-- ------------------------

-- GHC allows us to get at the guts inside IO errors/exceptions
--

-- regular imports
-- ---------------

import Cuddles.SysConfig
import Foreign.Storable
import Foreign.Ptr
import Foreign.C.Types
import Foreign.C.String
import Foreign.Marshal.Error 	( void )
import Data.Maybe

import System.IO		( Handle )
import System.IO.Error		( IOError, ioError )
import System.IO.Unsafe		( unsafePerformIO )

{-# CFILES cbits/PrelIOUtils.c #-}

-- "errno" type
-- ------------

-- | Haskell representation for @errno@ values.
-- The implementation is deliberately exposed, to allow users to add
-- their own definitions of 'Errno' values.

newtype Errno = Errno CInt

instance Eq Errno where
  errno1@(Errno no1) == errno2@(Errno no2) 
    | isValidErrno errno1 && isValidErrno errno2 = no1 == no2
    | otherwise					 = False

-- common "errno" symbols
--
eOK, e2BIG, eACCES, eADDRINUSE, eADDRNOTAVAIL, eADV, eAFNOSUPPORT, eAGAIN, 
  eALREADY, eBADF, eBADMSG, eBADRPC, eBUSY, eCHILD, eCOMM, eCONNABORTED, 
  eCONNREFUSED, eCONNRESET, eDEADLK, eDESTADDRREQ, eDIRTY, eDOM, eDQUOT, 
  eEXIST, eFAULT, eFBIG, eFTYPE, eHOSTDOWN, eHOSTUNREACH, eIDRM, eILSEQ, 
  eINPROGRESS, eINTR, eINVAL, eIO, eISCONN, eISDIR, eLOOP, eMFILE, eMLINK, 
  eMSGSIZE, eMULTIHOP, eNAMETOOLONG, eNETDOWN, eNETRESET, eNETUNREACH, 
  eNFILE, eNOBUFS, eNODATA, eNODEV, eNOENT, eNOEXEC, eNOLCK, eNOLINK, 
  eNOMEM, eNOMSG, eNONET, eNOPROTOOPT, eNOSPC, eNOSR, eNOSTR, eNOSYS, 
  eNOTBLK, eNOTCONN, eNOTDIR, eNOTEMPTY, eNOTSOCK, eNOTTY, eNXIO, 
  eOPNOTSUPP, ePERM, ePFNOSUPPORT, ePIPE, ePROCLIM, ePROCUNAVAIL, 
  ePROGMISMATCH, ePROGUNAVAIL, ePROTO, ePROTONOSUPPORT, ePROTOTYPE, 
  eRANGE, eREMCHG, eREMOTE, eROFS, eRPCMISMATCH, eRREMOTE, eSHUTDOWN, 
  eSOCKTNOSUPPORT, eSPIPE, eSRCH, eSRMNT, eSTALE, eTIME, eTIMEDOUT, 
  eTOOMANYREFS, eTXTBSY, eUSERS, eWOULDBLOCK, eXDEV		       :: Errno
--
-- the cCONST_XXX identifiers are cpp symbols whose value is computed by
-- configure 
--
eOK             = Errno 0
e2BIG           = Errno (const_E2BIG)
eACCES		= Errno (const_EACCES)
eADDRINUSE	= Errno (const_EADDRINUSE)
eADDRNOTAVAIL	= Errno (const_EADDRNOTAVAIL)
eADV		= Errno (const_EADV)
eAFNOSUPPORT	= Errno (const_EAFNOSUPPORT)
eAGAIN		= Errno (const_EAGAIN)
eALREADY	= Errno (const_EALREADY)
eBADF		= Errno (const_EBADF)
eBADMSG		= Errno (const_EBADMSG)
eBADRPC		= Errno (const_EBADRPC)
eBUSY		= Errno (const_EBUSY)
eCHILD		= Errno (const_ECHILD)
eCOMM		= Errno (const_ECOMM)
eCONNABORTED	= Errno (const_ECONNABORTED)
eCONNREFUSED	= Errno (const_ECONNREFUSED)
eCONNRESET	= Errno (const_ECONNRESET)
eDEADLK		= Errno (const_EDEADLK)
eDESTADDRREQ	= Errno (const_EDESTADDRREQ)
eDIRTY		= Errno (const_EDIRTY)
eDOM		= Errno (const_EDOM)
eDQUOT		= Errno (const_EDQUOT)
eEXIST		= Errno (const_EEXIST)
eFAULT		= Errno (const_EFAULT)
eFBIG		= Errno (const_EFBIG)
eFTYPE		= Errno (const_EFTYPE)
eHOSTDOWN	= Errno (const_EHOSTDOWN)
eHOSTUNREACH	= Errno (const_EHOSTUNREACH)
eIDRM		= Errno (const_EIDRM)
eILSEQ		= Errno (const_EILSEQ)
eINPROGRESS	= Errno (const_EINPROGRESS)
eINTR		= Errno (const_EINTR)
eINVAL		= Errno (const_EINVAL)
eIO		= Errno (const_EIO)
eISCONN		= Errno (const_EISCONN)
eISDIR		= Errno (const_EISDIR)
eLOOP		= Errno (const_ELOOP)
eMFILE		= Errno (const_EMFILE)
eMLINK		= Errno (const_EMLINK)
eMSGSIZE	= Errno (const_EMSGSIZE)
eMULTIHOP	= Errno (const_EMULTIHOP)
eNAMETOOLONG	= Errno (const_ENAMETOOLONG)
eNETDOWN	= Errno (const_ENETDOWN)
eNETRESET	= Errno (const_ENETRESET)
eNETUNREACH	= Errno (const_ENETUNREACH)
eNFILE		= Errno (const_ENFILE)
eNOBUFS		= Errno (const_ENOBUFS)
eNODATA		= Errno (const_ENODATA)
eNODEV		= Errno (const_ENODEV)
eNOENT		= Errno (const_ENOENT)
eNOEXEC		= Errno (const_ENOEXEC)
eNOLCK		= Errno (const_ENOLCK)
eNOLINK		= Errno (const_ENOLINK)
eNOMEM		= Errno (const_ENOMEM)
eNOMSG		= Errno (const_ENOMSG)
eNONET		= Errno (const_ENONET)
eNOPROTOOPT	= Errno (const_ENOPROTOOPT)
eNOSPC		= Errno (const_ENOSPC)
eNOSR		= Errno (const_ENOSR)
eNOSTR		= Errno (const_ENOSTR)
eNOSYS		= Errno (const_ENOSYS)
eNOTBLK		= Errno (const_ENOTBLK)
eNOTCONN	= Errno (const_ENOTCONN)
eNOTDIR		= Errno (const_ENOTDIR)
eNOTEMPTY	= Errno (const_ENOTEMPTY)
eNOTSOCK	= Errno (const_ENOTSOCK)
eNOTTY		= Errno (const_ENOTTY)
eNXIO		= Errno (const_ENXIO)
eOPNOTSUPP	= Errno (const_EOPNOTSUPP)
ePERM		= Errno (const_EPERM)
ePFNOSUPPORT	= Errno (const_EPFNOSUPPORT)
ePIPE		= Errno (const_EPIPE)
ePROCLIM	= Errno (const_EPROCLIM)
ePROCUNAVAIL	= Errno (const_EPROCUNAVAIL)
ePROGMISMATCH	= Errno (const_EPROGMISMATCH)
ePROGUNAVAIL	= Errno (const_EPROGUNAVAIL)
ePROTO		= Errno (const_EPROTO)
ePROTONOSUPPORT = Errno (const_EPROTONOSUPPORT)
ePROTOTYPE	= Errno (const_EPROTOTYPE)
eRANGE		= Errno (const_ERANGE)
eREMCHG		= Errno (const_EREMCHG)
eREMOTE		= Errno (const_EREMOTE)
eROFS		= Errno (const_EROFS)
eRPCMISMATCH	= Errno (const_ERPCMISMATCH)
eRREMOTE	= Errno (const_ERREMOTE)
eSHUTDOWN	= Errno (const_ESHUTDOWN)
eSOCKTNOSUPPORT = Errno (const_ESOCKTNOSUPPORT)
eSPIPE		= Errno (const_ESPIPE)
eSRCH		= Errno (const_ESRCH)
eSRMNT		= Errno (const_ESRMNT)
eSTALE		= Errno (const_ESTALE)
eTIME		= Errno (const_ETIME)
eTIMEDOUT	= Errno (const_ETIMEDOUT)
eTOOMANYREFS	= Errno (const_ETOOMANYREFS)
eTXTBSY		= Errno (const_ETXTBSY)
eUSERS		= Errno (const_EUSERS)
eWOULDBLOCK	= Errno (const_EWOULDBLOCK)
eXDEV		= Errno (const_EXDEV)

-- | Yield 'True' if the given 'Errno' value is valid on the system.
-- This implies that the 'Eq' instance of 'Errno' is also system dependent
-- as it is only defined for valid values of 'Errno'.
--
isValidErrno               :: Errno -> Bool
--
-- the configure script sets all invalid "errno"s to -1
--
isValidErrno (Errno errno)  = errno /= -1

-- access to the current thread's "errno" value
-- --------------------------------------------

-- | Get the current value of @errno@ in the current thread.
--
getErrno :: IO Errno

-- We must call a C function to get the value of errno in general.  On
-- threaded systems, errno is hidden behind a C macro so that each OS
-- thread gets its own copy.
getErrno = do e <- get_errno; return (Errno e)
foreign import ccall unsafe "HsBase.h __hscore_get_errno" get_errno :: IO CInt

-- | Reset the current thread\'s @errno@ value to 'eOK'.
--
resetErrno :: IO ()

-- Again, setting errno has to be done via a C function.
resetErrno = set_errno 0
foreign import ccall unsafe "HsBase.h __hscore_set_errno" set_errno :: CInt -> IO ()

-- throw current "errno" value
-- ---------------------------

-- | Throw an 'IOError' corresponding to the current value of 'getErrno'.
--
throwErrno     :: String	-- ^ textual description of the error location
	       -> IO a
throwErrno loc  =
  do
    errno <- getErrno
    ioError (errnoToIOError loc errno Nothing Nothing)

-- guards for IO operations that may fail
-- --------------------------------------

-- | Throw an 'IOError' corresponding to the current value of 'getErrno'
-- if the result value of the 'IO' action meets the given predicate.
--
throwErrnoIf    :: (a -> Bool)	-- ^ predicate to apply to the result value
				-- of the 'IO' operation
		-> String	-- ^ textual description of the location
		-> IO a		-- ^ the 'IO' operation to be executed
		-> IO a
throwErrnoIf pred loc f  = 
  do
    res <- f
    if pred res then throwErrno loc else return res

-- | as 'throwErrnoIf', but discards the result of the 'IO' action after
-- error handling.
--
throwErrnoIf_   :: (a -> Bool) -> String -> IO a -> IO ()
throwErrnoIf_ pred loc f  = void $ throwErrnoIf pred loc f

-- | as 'throwErrnoIf', but retry the 'IO' action when it yields the
-- error code 'eINTR' - this amounts to the standard retry loop for
-- interrupted POSIX system calls.
--
throwErrnoIfRetry            :: (a -> Bool) -> String -> IO a -> IO a
throwErrnoIfRetry pred loc f  = 
  do
    res <- f
    if pred res
      then do
	err <- getErrno
	if err == eINTR
	  then throwErrnoIfRetry pred loc f
	  else throwErrno loc
      else return res

-- | as 'throwErrnoIfRetry', but checks for operations that would block and
-- executes an alternative action before retrying in that case.
--
throwErrnoIfRetryMayBlock
		:: (a -> Bool)	-- ^ predicate to apply to the result value
				-- of the 'IO' operation
		-> String	-- ^ textual description of the location
		-> IO a		-- ^ the 'IO' operation to be executed
		-> IO b		-- ^ action to execute before retrying if
				-- an immediate retry would block
		-> IO a
throwErrnoIfRetryMayBlock pred loc f on_block  = 
  do
    res <- f
    if pred res
      then do
	err <- getErrno
	if err == eINTR
	  then throwErrnoIfRetryMayBlock pred loc f on_block
          else if err == eWOULDBLOCK || err == eAGAIN
	         then do on_block; throwErrnoIfRetryMayBlock pred loc f on_block
                 else throwErrno loc
      else return res

-- | as 'throwErrnoIfRetry', but discards the result.
--
throwErrnoIfRetry_            :: (a -> Bool) -> String -> IO a -> IO ()
throwErrnoIfRetry_ pred loc f  = void $ throwErrnoIfRetry pred loc f

-- | as 'throwErrnoIfRetryMayBlock', but discards the result.
--
throwErrnoIfRetryMayBlock_ :: (a -> Bool) -> String -> IO a -> IO b -> IO ()
throwErrnoIfRetryMayBlock_ pred loc f on_block 
  = void $ throwErrnoIfRetryMayBlock pred loc f on_block

-- | Throw an 'IOError' corresponding to the current value of 'getErrno'
-- if the 'IO' action returns a result of @-1@.
--
throwErrnoIfMinus1 :: Num a => String -> IO a -> IO a
throwErrnoIfMinus1  = throwErrnoIf (== -1)

-- | as 'throwErrnoIfMinus1', but discards the result.
--
throwErrnoIfMinus1_ :: Num a => String -> IO a -> IO ()
throwErrnoIfMinus1_  = throwErrnoIf_ (== -1)

-- | Throw an 'IOError' corresponding to the current value of 'getErrno'
-- if the 'IO' action returns a result of @-1@, but retries in case of
-- an interrupted operation.
--
throwErrnoIfMinus1Retry :: Num a => String -> IO a -> IO a
throwErrnoIfMinus1Retry  = throwErrnoIfRetry (== -1)

-- | as 'throwErrnoIfMinus1', but discards the result.
--
throwErrnoIfMinus1Retry_ :: Num a => String -> IO a -> IO ()
throwErrnoIfMinus1Retry_  = throwErrnoIfRetry_ (== -1)

-- | as 'throwErrnoIfMinus1Retry', but checks for operations that would block.
--
throwErrnoIfMinus1RetryMayBlock :: Num a => String -> IO a -> IO b -> IO a
throwErrnoIfMinus1RetryMayBlock  = throwErrnoIfRetryMayBlock (== -1)

-- | as 'throwErrnoIfMinus1RetryMayBlock', but discards the result.
--
throwErrnoIfMinus1RetryMayBlock_ :: Num a => String -> IO a -> IO b -> IO ()
throwErrnoIfMinus1RetryMayBlock_  = throwErrnoIfRetryMayBlock_ (== -1)

-- | Throw an 'IOError' corresponding to the current value of 'getErrno'
-- if the 'IO' action returns 'nullPtr'.
--
throwErrnoIfNull :: String -> IO (Ptr a) -> IO (Ptr a)
throwErrnoIfNull  = throwErrnoIf (== nullPtr)

-- | Throw an 'IOError' corresponding to the current value of 'getErrno'
-- if the 'IO' action returns 'nullPtr',
-- but retry in case of an interrupted operation.
--
throwErrnoIfNullRetry :: String -> IO (Ptr a) -> IO (Ptr a)
throwErrnoIfNullRetry  = throwErrnoIfRetry (== nullPtr)

-- | as 'throwErrnoIfNullRetry', but checks for operations that would block.
--
throwErrnoIfNullRetryMayBlock :: String -> IO (Ptr a) -> IO b -> IO (Ptr a)
throwErrnoIfNullRetryMayBlock  = throwErrnoIfRetryMayBlock (== nullPtr)

-- conversion of an "errno" value into IO error
-- --------------------------------------------

-- | Construct a Haskell 98 I\/O error based on the given 'Errno' value.
-- The optional information can be used to improve the accuracy of
-- error messages.
--
errnoToIOError	:: String	-- ^ the location where the error occurred
		-> Errno	-- ^ the error number
		-> Maybe Handle	-- ^ optional handle associated with the error
		-> Maybe String	-- ^ optional filename associated with the error
		-> IOError
errnoToIOError loc errno maybeHdl maybeName = unsafePerformIO $ do
    str <- strerror errno >>= peekCString
    return (userError (loc ++ ": " ++ str ++ maybe "" (": "++) maybeName))

foreign import ccall unsafe "string.h" strerror :: Errno -> IO (Ptr CChar)
