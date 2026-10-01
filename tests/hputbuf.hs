import System.IO
import System.Directory
import Foreign
import Foreign.C.String
main = do
  tmp <- fmap (++ "/hputbuf.tmp") getTemporaryDirectory
  h <- openBinaryFile tmp WriteMode
  withCStringLen "hello buffer\n" $ \(p,n) -> hPutBuf h p n
  hClose h
  readFile tmp >>= putStr
  removeFile tmp
