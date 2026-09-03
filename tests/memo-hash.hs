-- Hugs.Memo.hash (the "IOHash" primitive) returned a Bool (via the
-- IOBoolResult helper, which turns any non-zero result into True) instead
-- of the Int the type signature promises, which broke Hugs.Memo.memo:
-- every hash truncated to True/False so every key collided in bucket 1.
import Hugs.Memo

slow :: Int -> Int
slow n = n * 2

main :: IO ()
main = do
  let fastSlow = memo slow
  print (map fastSlow [1, 2, 3, 40, 1, 2, 3, 40])
