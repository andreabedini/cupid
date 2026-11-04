-- Cupid.Memo.hash (the "IOHash" primitive) must return the Int its type
-- signature promises, not a Bool through the IOBoolResult helper, which turns
-- any non-zero result into True.  Cupid.Memo.memo needs the Int: with the
-- Bool, every key would collide in bucket 1.
import Cupid.Memo

slow :: Int -> Int
slow n = n * 2

main :: IO ()
main = do
  let fastSlow = memo slow
  print (map fastSlow [1, 2, 3, 40, 1, 2, 3, 40])
