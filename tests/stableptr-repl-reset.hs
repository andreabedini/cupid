-- storage(RESET), called at the head of every REPL command, must not wipe
-- live stable pointers, or one created in one command would stop working as
-- soon as the next command is typed.  `sp` is forced (and its underlying
-- stable pointer allocated) by the first prompt line and dereferenced again
-- by the second; both must see the same value.



import Foreign.StablePtr
import System.IO.Unsafe (unsafePerformIO)

sp :: StablePtr Int
sp = unsafePerformIO (newStablePtr 123)
{-# NOINLINE sp #-}

main :: IO ()
main = return ()
