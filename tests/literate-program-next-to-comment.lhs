A program line must not sit next to a comment line.

> main :: IO ()
> main = putStrLn "never run"
This comment touches the program line above it.
