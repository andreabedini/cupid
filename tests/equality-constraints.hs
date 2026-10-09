-- Equality constraints, t1 ~ t2, in contexts.

-- In a signature the equality is given: the body is checked with its two
-- sides identified, here with a as Int.
f :: (a ~ Int) => a -> Int
f x = x + 1

-- Several, in either direction, beside class constraints.
g :: (b ~ a, Show a) => a -> b -> String
g x y = show x ++ show y

-- Either side may be an application; a function type needs parentheses.
h :: (f a ~ Maybe Int, g ~ (Int -> Bool)) => f a -> g -> Bool
h m p = maybe False p m

-- A single equality needs no parentheses.
k :: a ~ [Char] => a -> Int
k = length

-- A variable that occurs only in the context is not ambiguous: the
-- equality determines it.
z :: (a ~ Int) => Int
z = 3

-- An instance whose context is an equality.  The head matches any list,
-- and the equality then fixes the element type, so describe [] is at
-- [Char] without an annotation.
class Describe a where
    describe :: a -> String

instance (a ~ Char) => Describe [a] where
    describe s = "string " ++ s

-- A class method with an equality in its own context, and the instance
-- that implements it.
class Container f where
    empty :: f a
    insert :: (b ~ a) => b -> f a -> f a

newtype Box a = Box [a]

instance Container Box where
    empty = Box []
    insert x (Box xs) = Box (x : xs)

-- A rank-2 argument whose type has an equality in its context.
withInt :: (forall a. (a ~ Int) => a -> a) -> Int
withInt fn = fn 41

-- A polymorphic field whose type has one, and its selector.
data R = R { get :: forall a. (a ~ Int) => a }

-- An expression signature.
e :: Int
e = (5 :: (a ~ Int) => a)

main :: IO ()
main = do
    print (f 41)
    putStrLn (g 1 2)
    print (h (Just 3) even)
    print (k "abc")
    print z
    putStrLn (describe "hello")
    putStrLn (describe [])
    case insert 'x' (insert 'y' empty) of Box cs -> putStrLn cs
    print (withInt (\x -> x + 1))
    print (get (R { get = 7 }))
    print e
