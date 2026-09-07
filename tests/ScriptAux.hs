-- Imported by script-stack.hs, and a test of its own so that every .hs
-- under tests/ is one.
module ScriptAux where

auxValue :: Int
auxValue = 42

main :: IO ()
main = print auxValue
