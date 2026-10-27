-- -i is a mode, read from the command line only.  In CUPIDFLAGS it would
-- send every script to the prompt, so there it is an unknown toggle, and
-- the file still runs.
main :: IO ()
main = putStrLn "ran"
