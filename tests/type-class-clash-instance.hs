-- Types and classes share one namespace, so with a type T and a class T
-- imported from different modules an unqualified T is ambiguous.  An
-- instance declaration naming it is an error, and the report of the ambiguity
-- must not take the type for a class and crash the interpreter.
import ClashType
import ClashClass

instance T Bool where
    tm _ = 1

main :: IO ()
main = print (tm True)
