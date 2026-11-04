-- hFileSize must keep the high bits of a size past 4 GiB.  Converting off_t
-- through a 32-bit type would make this print 205032705 (4500000001 - 2^32)
-- instead of 4500000001.
--
-- Nothing is written past the seeks, so on any filesystem with sparse
-- files (ext4, btrfs, xfs, APFS, tmpfs) this costs two blocks, not 4.5 GB.
import System.IO
import System.Directory
main :: IO ()
main = do
  tmp <- fmap (++ "/hfilesize-large.tmp") getTemporaryDirectory
  h <- openBinaryFile tmp WriteMode
  mapM_ (const (hSeek h RelativeSeek 1500000000)) [1 .. 3 :: Int]
  hPutChar h 'x'
  hFileSize h >>= print
  hClose h
  removeFile tmp
