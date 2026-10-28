-- The programs under demos/, which ship with Cupid.  demos.stdin loads each
-- one at the prompt and evaluates something from it, so a change that stops
-- a demo loading or running fails here.
main :: IO ()
main = return ()
