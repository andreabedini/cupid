-- With a type T and a class T imported from different modules, qualified
-- names tell them apart: this module is legal and runs.
import ClashType
import ClashClass

instance ClashClass.T Bool where
    tm _ = 1

f :: ClashType.T -> Int
f _ = 2

main :: IO ()
main = print (f T1 + tm True + tm 'c')
