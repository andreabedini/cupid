-- The modules of base added since 2006 that Hackage code imports most,
-- and the functions added to the older modules.
module Main where

import Control.Monad.Zip
import Data.Bifunctor
import Data.Bool (bool)
import Data.Either
import Data.Function
import Data.Functor
import Data.Functor.Compose
import Data.Functor.Const
import Data.Functor.Identity
import qualified Data.Functor.Product as P
import qualified Data.Functor.Sum as S
import Data.IORef
import Data.List
import Data.List.NonEmpty (NonEmpty(..))
import qualified Data.List.NonEmpty as NE
import Data.Ord
import Data.Proxy
import Data.String
import Data.Tuple (swap)
import Data.Void
import Numeric.Natural
import Text.Read (readMaybe, readEither)

newtype Name = Name String deriving Show

instance IsString Name where
    fromString = Name

main :: IO ()
main = do
    -- Data.Function, Data.Functor
    print ([3, 1, 2] & sortBy (compare `on` negate), fix (\f n -> if n == 0 then 1 else n * f (n - 1)) (5 :: Int))
    print (Just 1 $> 'x', [1, 2] <&> (* 10), void (Just 3))
    -- Data.Functor.*
    print (runIdentity (traverse (Identity . succ) "abc"), Identity 'q')
    print (getConst (traverse (\x -> Const [x]) [1, 2, 3 :: Int]))
    print (fmap (+ 1) (Compose (Just [1, 2])), sum (Compose [Just 3, Nothing, Just 4]))
    print (fmap show (P.Pair (Just 1) [2, 3]), length (S.InR "abc" :: S.Sum Maybe [] Char))
    -- Data.Bifunctor, Data.Tuple, Data.Bool
    print (bimap length show ("ab", 3), first not (True, 'c'), second (* 2) (Left 'e' :: Either Char Int))
    print (swap (1, "one"), bool "no" "yes" (3 > 2))
    -- Data.Either
    let es = [Left 'a', Right 1, Left 'b', Right (2 :: Int)]
    print (lefts es, rights es, partitionEithers es)
    print (isLeft (head es), fromRight 0 (head es), fromLeft 'z' (head es))
    -- Data.Ord
    print (sortOn Down [3, 1, 2], comparing snd (1, 'b') (2, 'a'), clamp (0, 10) (15 :: Int))
    -- Data.List
    print (uncons "abc", unsnoc "abc", singleton 'x', intercalate ", " ["a", "b"])
    print (subsequences [1, 2, 3], length (permutations [1 .. 4]), sort (permutations "abc"))
    print (scanl' (+) 0 [1, 2, 3], take 3 (iterate' (* 2) 1), dropWhileEnd (== ' ') "trim  ")
    print (stripPrefix "foo" "foobar", isSubsequenceOf "ace" "abcde", [1, 2, 3] !? 1, [1] !? 5)
    -- Data.List.NonEmpty
    let ne = 5 :| [3, 8, 1]
    print (NE.head ne, NE.length ne, NE.toList (NE.sort ne), maximum ne, NE.nonEmpty ([] :: [Int]))
    print (NE.groupBy (==) "aabbbc", ne <> NE.fromList [0])
    -- Control.Monad.Zip
    print (mzip [1, 2, 3] "ab", munzip (Just (1, 'x')))
    -- Data.Proxy, Data.Void, Data.String
    print (Proxy :: Proxy Int, (vacuous Nothing :: Maybe Int), fromString "Ada" :: Name)
    -- Numeric.Natural
    print ((7 :: Natural) * 6, toInteger (2 :: Natural) ^ 70, minusNaturalMaybe 3 5)
    -- Text.Read
    print (readMaybe "42" :: Maybe Int, readMaybe "4x" :: Maybe Int, readEither "[1,2]" :: Either String [Int])
    -- Data.IORef
    r <- newIORef (0 :: Int)
    modifyIORef' r (+ 5)
    atomicWriteIORef r 7
    n <- atomicModifyIORef' r (\x -> (x * 2, x))
    readIORef r >>= \v -> print (n, v)
