-- deriving Generic gives a type the representation of GHC.Generics:
-- from and to convert to and from it, and the metadata give the names of
-- the type, its constructors and their fields, and constructor fixities.
-- A function written once over the representation, here a generic show,
-- then works for every type that derives Generic.
module Main where

import GHC.Generics

infixr 5 :%

data T a = A Int a | B | C { f1 :: Bool, f2 :: [a] }
    deriving (Show, Generic)
data P = Int :% Char deriving (Show, Generic)
newtype N = N Int deriving (Show, Generic)
data Five = F1 | F2 | F3 | F4 | F5 deriving (Show, Enum, Bounded, Generic)
data R = R { r1 :: Int, r2 :: Char } deriving Generic
data Void deriving Generic

class GShow f where
    gshowsPrec :: Int -> f p -> ShowS

instance GShow U1 where
    gshowsPrec _ U1 = id

instance Show c => GShow (K1 i c) where
    gshowsPrec d (K1 x) = showsPrec d x

instance (GShow f, GShow g) => GShow (f :+: g) where
    gshowsPrec d (L1 x) = gshowsPrec d x
    gshowsPrec d (R1 x) = gshowsPrec d x

instance (GShow f, GShow g) => GShow (f :*: g) where
    gshowsPrec d (a :*: b) = gshowsPrec d a . showChar ' ' . gshowsPrec d b

instance (Constructor c, GShow f) => GShow (M1 C c f) where
    gshowsPrec d m@(M1 x) =
        showParen (d > 10) (showString (conName m) . showChar ' ' . gshowsPrec 11 x)

instance (Selector s, GShow f) => GShow (M1 S s f) where
    gshowsPrec d m@(M1 x) = showString (selName m) . showChar '=' . gshowsPrec d x

instance (Datatype t, GShow f) => GShow (M1 D t f) where
    gshowsPrec d m@(M1 x) =
        showString (moduleName m ++ "." ++ datatypeName m ++ ": ") . gshowsPrec d x

gshow :: (Generic a r, GShow r) => a -> String
gshow x = gshowsPrec 0 (from x) ""

roundTrip :: Generic a r => a -> a
roundTrip = to . from

isNewtypeOf :: (Generic a (M1 D d f), Datatype d) => a -> Bool
isNewtypeOf x = isNewtype (from x)

main :: IO ()
main = do
    putStrLn (gshow (A 1 "x"))
    putStrLn (gshow (B :: T Int))
    putStrLn (gshow (C True [3 :: Int]))
    putStrLn (gshow (1 :% 'c'))
    putStrLn (gshow (N 7))
    mapM_ (putStrLn . gshow) [minBound .. maxBound :: Five]
    print (roundTrip (C False "ab"), roundTrip (2 :% 'z'), map roundTrip [F1 ..])
    print (conFixity (unM1 (from (1 :% 'c'))), conFixity (unM1 (from (N 1))))
    print (conIsRecord (unM1 (from (R 1 'r'))), conIsRecord (unM1 (from (N 1))))
    putStrLn (gshow (R 1 'r'))
    print (isNewtypeOf (N 1), isNewtypeOf F1)
    print (datatypeName (from (undefined :: Void)) == "Void")
