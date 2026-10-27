-- -i loads files at the prompt and --ffi compiles stubs; there is no
-- meaning for both at once, so cupid refuses before doing anything.
main :: IO ()
main = putStrLn "ran"
