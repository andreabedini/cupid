-- storage(RESET), called at the head of every REPL command, used to call
-- resetStablePtrs() unconditionally (the only thing guarding it was
-- "#if !HSCRIPT", and HSCRIPT is never defined), wiping every live stable
-- pointer between one prompt line and the next. A stable pointer created
-- in one command therefore stopped working as soon as the next command
-- was typed. `sp` is forced (and its underlying stable pointer allocated)
-- by the first prompt line and dereferenced again by the second; both
-- must see the same value.
import Foreign.StablePtr
import System.IO.Unsafe (unsafePerformIO)

sp :: StablePtr Int
sp = unsafePerformIO (newStablePtr 123)
{-# NOINLINE sp #-}

main :: IO ()
main = return ()
