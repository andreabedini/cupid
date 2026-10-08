-- GADT syntax for ordinary data declarations: each constructor as a
-- signature whose result is the type applied to distinct variables.
data Shape a where
    Circle :: a -> Shape a
    Rect   :: a -> a -> Shape a
    Named  :: { label :: String, inner :: Shape a } -> Shape a
    Strict :: !Int -> a -> Shape a
    Nil, Unit :: Shape a
    (:+:)  :: Shape a -> Shape a -> Shape a
  deriving Show

-- A variable not in the result is existential; a context is packed with
-- the constructor and given back on a match.
data Some where
    Some :: Show a => a -> Some

data Pack a where
    Pack :: forall b. (Show b, Eq b) => a -> b -> b -> Pack a

-- A kind in the head binds the variables.
data Expr :: * -> * where
    Lit :: a -> Expr a
    Neg :: Expr a -> Expr a

data Id (f :: * -> *) where
    Id :: f Int -> Id f

data Poly where
    Poly :: (forall a. a -> a) -> Poly

newtype Wrap a where
    Wrap :: { unwrap :: [a] } -> Wrap a
  deriving Show

-- The signature's names are its own: here a is existential and b is the
-- head's a.
data T a where
    C :: a -> b -> T b

showSome :: Some -> String
showSome (Some x) = show x

samePack :: Pack a -> Bool
samePack (Pack _ x y) = x == y

area :: Shape Double -> Double
area (Circle r) = 3 * r * r
area (Rect w h) = w * h
area (Named _ s) = area s
area (Strict n x) = fromIntegral n * x
area Nil = 0
area Unit = 1
area (s :+: t) = area s + area t

eval :: Expr Int -> Int
eval (Lit n) = n
eval (Neg e) = negate (eval e)

usePoly :: Poly -> (Int, Bool)
usePoly (Poly f) = (f 1, f True)

main :: IO ()
main = do
    print (area (Named "box" (Rect 2 3) :+: Circle 1 :+: Strict 2 5))
    print (Named "u" Unit :+: Nil :: Shape Int)
    putStrLn (showSome (Some [1, 2, 3]))
    print (samePack (Pack 'x' 1 1), samePack (Pack 'x' 1 2))
    print (eval (Neg (Lit 4)))
    case Id (Just 1) of Id m -> print m
    print (unwrap (Wrap "abc"))
    print (usePoly (Poly id))
    case C 'x' True of C _ b -> print b
