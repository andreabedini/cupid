-- | The types of call stacks, after GHC's module of the same name.
--
-- Cupid records no call stacks: 'HasCallStack' is a class with no
-- parameters and one instance, so it always holds and passes nothing,
-- and every 'CallStack' a program sees is empty unless it builds one
-- itself with 'pushCallStack' or 'fromCallSiteList'.
module GHC.Stack.Types (
    CallStack(..), HasCallStack, SrcLoc(..),
    emptyCallStack, freezeCallStack, fromCallSiteList, getCallStack,
    pushCallStack
  ) where

-- | Holds for every function: a function with @HasCallStack@ in its
-- context may be called anywhere, as in GHC, but receives no stack.
class HasCallStack
instance HasCallStack

-- | A stack of call sites, innermost first.
data CallStack
    = EmptyCallStack
    | PushCallStack [Char] SrcLoc CallStack
    | FreezeCallStack CallStack

instance Show CallStack where
    showsPrec _ = shows . getCallStack

-- | A place in the source of a package.
data SrcLoc = SrcLoc {
    srcLocPackage   :: [Char],
    srcLocModule    :: [Char],
    srcLocFile      :: [Char],
    srcLocStartLine :: Int,
    srcLocStartCol  :: Int,
    srcLocEndLine   :: Int,
    srcLocEndCol    :: Int
  } deriving (Eq, Show)

emptyCallStack :: CallStack
emptyCallStack = EmptyCallStack

-- | A stack that 'pushCallStack' leaves as it is.
freezeCallStack :: CallStack -> CallStack
freezeCallStack = FreezeCallStack

fromCallSiteList :: [([Char], SrcLoc)] -> CallStack
fromCallSiteList = foldr (\(f, l) s -> PushCallStack f l s) EmptyCallStack

-- | The call sites, innermost first, each with the function called.
getCallStack :: CallStack -> [([Char], SrcLoc)]
getCallStack EmptyCallStack          = []
getCallStack (PushCallStack f l s)   = (f, l) : getCallStack s
getCallStack (FreezeCallStack s)     = getCallStack s

pushCallStack :: ([Char], SrcLoc) -> CallStack -> CallStack
pushCallStack (f, l) s = case s of
    FreezeCallStack _ -> s
    _                 -> PushCallStack f l s
