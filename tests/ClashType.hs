-- Exports a type named T, which the type-class-clash tests import together
-- with the class T of ClashClass.  A test of its own so that every .hs
-- under tests/ is one.
module ClashType where

data T = T1 deriving Show

main :: IO ()
main = print T1
