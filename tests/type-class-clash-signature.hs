-- With a type T and a class T imported from different modules, an
-- unqualified T in a type is ambiguous too.  It was taken to be the type.
import ClashType
import ClashClass

f :: T -> Int
f _ = 2

main :: IO ()
main = print (f T1)
