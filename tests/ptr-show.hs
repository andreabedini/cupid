-- show on a Ptr passed a 64-bit address to %x through a `long` vararg;
-- on a 64-bit system %x only consumes 32 bits from the vararg, so the
-- printed address was truncated to its low 32 bits.  The exact address
-- of a live allocation isn't deterministic (ASLR), so check the shape of
-- the output (16 hex digits after "0x") rather than its value; ask for a
-- page-sized block, since glibc mmaps those well above the 4GB mark,
-- to make sure the truncation would actually be visible if it recurred.
import Foreign
import Data.Char (isHexDigit)

main :: IO ()
main = do
  print (show nullPtr)
  p <- mallocBytes (1024*1024) :: IO (Ptr ())
  let s = show p
  print (take 2 s == "0x")
  print (length s == 18)
  print (all isHexDigit (drop 2 s))
  print (p == p)
  free p
