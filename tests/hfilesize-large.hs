-- hFileSize must not lose the high bits of a size past 4 GiB, which is
-- where it used to wrap: the old code converted off_t through a 32-bit
-- Unsigned, so this printed 205032705 (4500000001 - 2^32).
--
-- Nothing is written past the seeks, so on any filesystem with sparse
-- files (ext4, btrfs, xfs, APFS, tmpfs) this costs two blocks, not 4.5 GB.
import System.IO
import System.Directory
main :: IO ()
main = do
  h <- openBinaryFile "hfilesize-large.tmp" WriteMode
  mapM_ (const (hSeek h RelativeSeek 1500000000)) [1 .. 3 :: Int]
  hPutChar h 'x'
  hFileSize h >>= print
  hClose h
  removeFile "hfilesize-large.tmp"
