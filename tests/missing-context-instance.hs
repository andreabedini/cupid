-- A member that needs Show a reports Show a as the missing constraint:
-- Show [a] is reduced by its instance before the error is raised.
data T a = T a

instance Show (T a) where
    show (T x) = show [x]

main :: IO ()
main = print (T 'x')
