-- The foreign import marshalling of a StablePtr argument/result (used by
-- e.g. Storable's peek/poke for StablePtr, via Hugs/Storable.hs's
-- readStablePtrOffPtr/writeStablePtrOffPtr) popped the argument without
-- evaluating it first, and pushed the C-side result as a raw Cell number
-- instead of a boxed Int -- both at odds with how newStablePtr represents
-- a StablePtr.  Round-trip one through C and check it still identifies
-- the same value.
import Foreign
import Foreign.StablePtr

main :: IO ()
main = do
  sp1 <- newStablePtr (42 :: Int)
  buf <- mallocBytes 8 :: IO (Ptr (StablePtr Int))
  poke buf (id sp1)
  sp2 <- peek buf
  v <- deRefStablePtr sp2
  print v
  print (castStablePtrToPtr sp1 == castStablePtrToPtr sp2)
  freeStablePtr sp1
  free buf
