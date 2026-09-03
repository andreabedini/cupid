-- freeStablePtr on a StablePtr that doesn't come from newStablePtr (e.g.
-- one built by castPtrToStablePtr from an arbitrary Ptr) used to write
-- straight through an unchecked array index and crash the process with a
-- signal.  It's now a reported (non-signal) error instead.
import Foreign
import Foreign.StablePtr

main :: IO ()
main = freeStablePtr (castPtrToStablePtr (nullPtr `plusPtr` 99999999))
