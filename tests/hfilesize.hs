-- hFileSize on an ordinary small file: the normal path through the new
-- 64-bit bignum constructor.
import System.IO
import System.Directory
main :: IO ()
main = do
  writeFile "hfilesize.tmp" (replicate 12345 'x')
  h <- openBinaryFile "hfilesize.tmp" ReadMode
  hFileSize h >>= print
  hClose h
  removeFile "hfilesize.tmp"
