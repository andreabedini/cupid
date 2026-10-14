-- | Call stacks, after GHC's module of the same name.
--
-- Cupid records no call stacks (see "GHC.Stack.Types"), so code written
-- for GHC with @HasCallStack@ in its types loads and runs, but
-- 'callStack' is always empty and an 'error' call reports only its
-- message.
module GHC.Stack (
    CallStack, HasCallStack, SrcLoc(..),
    callStack, emptyCallStack, freezeCallStack, fromCallSiteList,
    getCallStack, popCallStack, prettyCallStack, prettySrcLoc,
    pushCallStack, withFrozenCallStack,
    currentCallStack, whoCreated, errorWithStackTrace
  ) where

import GHC.Stack.Types
import Data.List (intercalate)

-- | The stack of the current call: always empty in Cupid.
callStack :: HasCallStack => CallStack
callStack = EmptyCallStack

-- | The stack without its innermost call site.
popCallStack :: CallStack -> CallStack
popCallStack s = case s of
    EmptyCallStack        -> s
    PushCallStack _ _ s'  -> s'
    FreezeCallStack _     -> s

-- | Runs a computation with the call stack frozen, which in Cupid,
-- where nothing is recorded, is running it.
withFrozenCallStack :: HasCallStack => a -> a
withFrozenCallStack x = x

prettySrcLoc :: SrcLoc -> String
prettySrcLoc l = concat [srcLocFile l, ":", show (srcLocStartLine l), ":",
                         show (srcLocStartCol l), " in ", srcLocPackage l,
                         ":", srcLocModule l]

-- | The stack as GHC prints it under an error message, or the empty
-- string for an empty stack.
prettyCallStack :: CallStack -> String
prettyCallStack s = case getCallStack s of
    []    -> ""
    sites -> intercalate "\n"
                 ("CallStack (from HasCallStack):"
                  : [ "  " ++ f ++ ", called at " ++ prettySrcLoc l
                    | (f, l) <- sites ])

-- | The cost-centre stack, which Cupid does not keep: always empty.
currentCallStack :: IO [String]
currentCallStack = return []

-- | Where a value was made, which Cupid does not record: always empty.
whoCreated :: a -> IO [String]
whoCreated _ = return []

errorWithStackTrace :: String -> a
errorWithStackTrace = error
