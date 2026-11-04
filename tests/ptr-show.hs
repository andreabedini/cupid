-- show on a Ptr must print all of a 64-bit address: %x consumes only 32 bits
-- of a vararg, so passing the address to it through a `long` would truncate
-- it to its low 32 bits.  The exact address
-- of a live allocation isn't deterministic (ASLR), so check the shape of
-- the output (16 hex digits after "0x") rather than its value; ask for a
-- page-sized block, since glibc mmaps those well above the 4GB mark,
-- to make sure the truncation would be visible.
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
