-- alignPtr p 0 (and any non-positive alignment) used to raise SIGFPE
-- (dividing by the alignment) and kill the process.
import Foreign
import Control.Exception

main :: IO ()
main = do
  r0 <- try (evaluate (alignPtr nullPtr 0)) :: IO (Either SomeException (Ptr ()))
  print r0
  rNeg <- try (evaluate (alignPtr nullPtr (-8))) :: IO (Either SomeException (Ptr ()))
  print rNeg
  -- ordinary alignment still works
  print (alignPtr (nullPtr `plusPtr` 3) 4 == nullPtr `plusPtr` 4)
  print (alignPtr (nullPtr `plusPtr` 8) 4 == nullPtr `plusPtr` 8)
