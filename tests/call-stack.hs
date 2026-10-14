-- HasCallStack holds everywhere, and a call stack is always empty
module Main where

import GHC.Stack
import qualified Data.Map as M

headOr :: HasCallStack => a -> [a] -> a
headOr d []    = d
headOr _ (x:_) = x

lookupOrFail :: (HasCallStack, Ord k, Show k) => k -> M.Map k v -> v
lookupOrFail k m = case M.lookup k m of
    Just v  -> v
    Nothing -> error ("missing key " ++ show k)

class Container f where
    first :: HasCallStack => f a -> a

newtype Box a = Box a
instance Container Box where
    first (Box x) = x

data Shown = forall a. (HasCallStack, Show a) => Shown a

whereAmI :: HasCallStack => String
whereAmI = prettyCallStack callStack

frozen :: HasCallStack => Int -> Int
frozen n = withFrozenCallStack (headOr n [])

main :: IO ()
main = do
    print (headOr 0 [1, 2, 3 :: Int])
    print (lookupOrFail "b" (M.fromList [("a", 1), ("b", 2 :: Int)]))
    print (first (Box 'x'))
    print [show x | Shown x <- [Shown (1 :: Int), Shown True]]
    print (whereAmI, frozen 7)
    print (null (getCallStack callStack))
    let loc = SrcLoc "main" "Main" "call-stack.hs" 3 1 3 9
        s   = pushCallStack ("f", loc) (fromCallSiteList [("g", loc)])
    putStrLn (prettyCallStack s)
    print (length (getCallStack (popCallStack s)))
    print (getCallStack (pushCallStack ("h", loc) (freezeCallStack s)) == getCallStack s)
    putStrLn (prettySrcLoc loc)
