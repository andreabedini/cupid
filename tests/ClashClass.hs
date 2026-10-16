-- Exports a class named T, which the type-class-clash tests import together
-- with the type T of ClashType.  A test of its own so that every .hs under
-- tests/ is one.
module ClashClass where

class T a where
    tm :: a -> Int

instance T Char where
    tm _ = 7

main :: IO ()
main = print (tm 'x')
