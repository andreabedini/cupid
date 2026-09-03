import System.IO
import System.Directory
import Foreign
import Foreign.C.String
main = do
  h <- openBinaryFile "hputbuf.tmp" WriteMode
  withCStringLen "hello buffer\n" $ \(p,n) -> hPutBuf h p n
  hClose h
  readFile "hputbuf.tmp" >>= putStr
  removeFile "hputbuf.tmp"
