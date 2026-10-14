-- Cupid.Memo.hash (the "IOHash" primitive) returned a Bool (via the
-- IOBoolResult helper, which turns any non-zero result into True) instead
-- of the Int the type signature promises, which broke Cupid.Memo.memo:
-- every hash truncated to True/False so every key collided in bucket 1.
import Cupid.Memo

slow :: Int -> Int
slow n = n * 2

main :: IO ()
main = do
  let fastSlow = memo slow
  print (map fastSlow [1, 2, 3, 40, 1, 2, 3, 40])
