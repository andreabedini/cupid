-- hFileSize on an ordinary small file: the normal path through the new
-- 64-bit bignum constructor.
import System.IO
import System.Directory
main :: IO ()
main = do
  tmp <- fmap (++ "/hfilesize.tmp") getTemporaryDirectory
  writeFile tmp (replicate 12345 'x')
  h <- openBinaryFile tmp ReadMode
  hFileSize h >>= print
  hClose h
  removeFile tmp
