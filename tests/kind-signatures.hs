-- Kind signatures: on the variables a declaration binds, and standalone.
import Data.Kind (Type, Constraint)

newtype Fix (f :: * -> *) = In (f (Fix f))

data ListF a r = NilF | ConsF a r

type Compose :: (Type -> Type) -> (Type -> Type) -> Type -> Type
newtype Compose f g a = Compose (f (g a))

type Pair (f :: Type -> Type) = (f Int, f Bool)

class Container (f :: * -> *) where
    empty :: f a
    insert :: a -> f a -> f a
    toL :: f a -> [a]

instance Container [] where
    empty = []
    insert = (:)
    toL = id

type Shaped :: (* -> *) -> Constraint
class Shaped f where
    shape :: f a -> String

instance Shaped Maybe where
    shape Nothing = "nothing"
    shape (Just _) = "just"

-- Both forms at once, as long as they agree.
type Proxy :: (Type -> Type) -> Type
data Proxy (t :: Type -> Type) = Proxy

type Wrapped :: Type
data Wrapped = Wrapped Int deriving Show

cata :: (ListF a r -> r) -> Fix (ListF a) -> r
cata alg (In NilF) = alg NilF
cata alg (In (ConsF a r)) = alg (ConsF a (cata alg r))

fromList :: [a] -> Fix (ListF a)
fromList = foldr (\a r -> In (ConsF a r)) (In NilF)

total :: Fix (ListF Int) -> Int
total = cata alg where
    alg NilF = 0
    alg (ConsF a r) = a + r

pr :: Pair Maybe
pr = (Just 1, Nothing)

main :: IO ()
main = do
    print (total (fromList [1 .. 10]))
    print (toL (insert 'a' (insert 'b' (empty :: [Char]))))
    putStrLn (shape (Just ()))
    case Compose [Just 'x'] of Compose x -> print x
    print pr
    print (Wrapped 3)
