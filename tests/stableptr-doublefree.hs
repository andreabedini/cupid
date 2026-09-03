-- Freeing the same stable pointer twice used to thread the free list into
-- itself, so the next two newStablePtr calls handed out the very same
-- pointer (the second print below used to say True).
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
