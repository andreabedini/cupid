-- Every type built from type constructors is Typeable without an
-- instance, as in GHC: the type checker builds its TypeRep from the
-- type's structure, so cast works on types the program defines, and
-- deriving Typeable is accepted and does nothing.
module Main where

import Data.Typeable

data Color = Red | Green deriving Show
data Pair a b = Pair a b
newtype Wrap f a = Wrap (f a)
data Old = Old deriving (Show, Typeable)

-- The context Typeable (Pair a a) reduces to Typeable a.
pairRep :: Typeable a => a -> TypeRep
pairRep x = typeOf (Pair x x)

main :: IO ()
main = do
    print (typeOf Red, typeOf (Pair Red [Just 'c']), typeOf (Wrap (Just Green)))
    print (cast Red :: Maybe Color, cast Red :: Maybe Int, cast (3 :: Int) :: Maybe Int)
    print (pairRep True, typeOf Old, typeRep (Nothing :: Maybe Color))
    print (typeOf (Pair Red Red) == typeOf (Pair Green Green), typeOf Red == typeOf Old)
    print (typeRepArgs (typeOf (Pair Red 'x')), tyConName (typeRepTyCon (typeOf Red)))
