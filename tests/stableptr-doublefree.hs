-- Freeing the same stable pointer twice must not thread the free list into
-- itself, or the next two newStablePtr calls would hand out the very same
-- pointer and the print below would say True.
import Foreign
import Foreign.StablePtr

main :: IO ()
main = do
  p <- newStablePtr ()
  freeStablePtr p
  freeStablePtr p
  q <- newStablePtr "x"
  r <- newStablePtr "y"
  print (castStablePtrToPtr q == castStablePtrToPtr r)
