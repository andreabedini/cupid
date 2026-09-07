module Cuddles.StablePtr( StablePtr, module Cuddles.StablePtr ) where

import Cuddles.Prelude( StablePtr, Ptr )

-- recently renamed
newStablePtr = makeStablePtr 

primitive makeStablePtr      :: a -> IO (StablePtr a)
primitive deRefStablePtr     :: StablePtr a -> IO a
primitive freeStablePtr      :: StablePtr a -> IO ()
primitive castStablePtrToPtr :: StablePtr a -> Ptr ()
primitive castPtrToStablePtr :: Ptr () -> StablePtr a

