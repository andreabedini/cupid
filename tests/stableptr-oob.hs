-- freeStablePtr on a StablePtr that doesn't come from newStablePtr (e.g.
-- one built by castPtrToStablePtr from an arbitrary Ptr) must be a reported
-- error, not a write through an unchecked array index that kills the process
-- with a signal.
import Foreign
import Foreign.StablePtr

main :: IO ()
main = freeStablePtr (castPtrToStablePtr (nullPtr `plusPtr` 99999999))
