-----------------------------------------------------------------------------
-- |
-- Module      :  Data.List.NonEmpty
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  provisional
-- Portability :  portable
--
-- Lists with at least one element.  The type is defined in the Prelude,
-- for 'sconcat'.  Many of the names here are those of list functions,
-- so the module is meant to be imported qualified.

module Data.List.NonEmpty (
	NonEmpty(..),
	-- * Non-empty stream transformations
	map, intersperse, scanl, scanr, scanl1, scanr1,
	transpose, sortBy, sortWith, sortOn,
	-- * Basic functions
	length, head, tail, last, init, singleton, (<|), cons,
	uncons, unfoldr, sort, reverse, inits, tails, append, appendList,
	prependList,
	-- * Building streams
	iterate, repeat, cycle, insert, some1,
	-- * Extracting sublists
	take, drop, splitAt, takeWhile, dropWhile, span, break,
	filter, partition, group, groupBy, groupWith, groupAllWith,
	group1, groupBy1, groupWith1, groupAllWith1,
	-- * Sublist predicates
	isPrefixOf,
	-- * Set operations
	nub, nubBy,
	-- * Indexing streams
	(!!),
	-- * Zipping and unzipping streams
	zip, zipWith, unzip,
	-- * Converting to and from a list
	fromList, toList, nonEmpty, xor
	) where

import Prelude hiding (map, scanl, scanr, scanl1, scanr1, length, head,
	tail, last, init, reverse, iterate, repeat, cycle, take, drop,
	splitAt, takeWhile, dropWhile, span, break, filter, (!!), zip,
	zipWith, unzip)
import qualified Prelude
import qualified Data.List as List
import Data.Ord (comparing)
import Cupid.Prelude (NonEmpty(..), Alternative(..))
import qualified Cupid.Prelude as P (Foldable(toList))

infixr 5 <|

-- | The number of elements.
length :: NonEmpty a -> Int
length (_ :| xs) = 1 + Prelude.length xs

-- | Convert a list to a non-empty one, or 'Nothing' if it is empty.
nonEmpty :: [a] -> Maybe (NonEmpty a)
nonEmpty []     = Nothing
nonEmpty (a:as) = Just (a :| as)

-- | Convert a list to a non-empty one; an error if it is empty.
fromList :: [a] -> NonEmpty a
fromList (a:as) = a :| as
fromList []     = error "NonEmpty.fromList: empty list"

-- | Convert to a list.
toList :: NonEmpty a -> [a]
toList (a :| as) = a : as

-- | A one-element list.
singleton :: a -> NonEmpty a
singleton a = a :| []

head :: NonEmpty a -> a
head (a :| _) = a

tail :: NonEmpty a -> [a]
tail (_ :| as) = as

last :: NonEmpty a -> a
last (a :| as) = Prelude.last (a : as)

init :: NonEmpty a -> [a]
init (a :| as) = Prelude.init (a : as)

-- | Prepend an element.
(<|) :: a -> NonEmpty a -> NonEmpty a
a <| (b :| bs) = a :| b : bs

cons :: a -> NonEmpty a -> NonEmpty a
cons = (<|)

uncons :: NonEmpty a -> (a, Maybe (NonEmpty a))
uncons (a :| as) = (a, nonEmpty as)

unfoldr :: (a -> (b, Maybe a)) -> a -> NonEmpty b
unfoldr f a = case f a of
	(b, mc) -> b :| maybe [] go mc
  where
	go c = case f c of
		(d, me) -> d : maybe [] go me

sort :: Ord a => NonEmpty a -> NonEmpty a
sort = lift List.sort

sortBy :: (a -> a -> Ordering) -> NonEmpty a -> NonEmpty a
sortBy f = lift (List.sortBy f)

sortWith :: Ord o => (a -> o) -> NonEmpty a -> NonEmpty a
sortWith = sortBy . comparing

sortOn :: Ord b => (a -> b) -> NonEmpty a -> NonEmpty a
sortOn f = lift (Prelude.map snd . List.sortBy (comparing fst) . Prelude.map (\x -> let y = f x in y `seq` (y, x)))

reverse :: NonEmpty a -> NonEmpty a
reverse = lift Prelude.reverse

inits :: Foldable f => f a -> NonEmpty [a]
inits = fromList . List.inits . P.toList

tails :: Foldable f => f a -> NonEmpty [a]
tails = fromList . List.tails . P.toList

append :: NonEmpty a -> NonEmpty a -> NonEmpty a
append = (<>)

appendList :: NonEmpty a -> [a] -> NonEmpty a
appendList (x :| xs) ys = x :| xs ++ ys

prependList :: [a] -> NonEmpty a -> NonEmpty a
prependList ls ne = case ls of
	[]     -> ne
	(x:xs) -> x :| xs ++ toList ne

-- | Apply a list function to a non-empty list, whose result must not be
-- empty.
lift :: ([a] -> [b]) -> NonEmpty a -> NonEmpty b
lift f = fromList . f . toList

map :: (a -> b) -> NonEmpty a -> NonEmpty b
map f (a :| as) = f a :| Prelude.map f as

intersperse :: a -> NonEmpty a -> NonEmpty a
intersperse a = lift (List.intersperse a)

scanl :: Foldable f => (b -> a -> b) -> b -> f a -> NonEmpty b
scanl f z = fromList . Prelude.scanl f z . P.toList

scanr :: Foldable f => (a -> b -> b) -> b -> f a -> NonEmpty b
scanr f z = fromList . Prelude.scanr f z . P.toList

scanl1 :: (a -> a -> a) -> NonEmpty a -> NonEmpty a
scanl1 f (a :| as) = fromList (Prelude.scanl f a as)

scanr1 :: (a -> a -> a) -> NonEmpty a -> NonEmpty a
scanr1 f (a :| as) = fromList (Prelude.scanr1 f (a : as))

transpose :: NonEmpty (NonEmpty a) -> NonEmpty (NonEmpty a)
transpose = fmap fromList . fromList . List.transpose . toList . fmap toList

iterate :: (a -> a) -> a -> NonEmpty a
iterate f a = a :| Prelude.iterate f (f a)

repeat :: a -> NonEmpty a
repeat a = a :| Prelude.repeat a

cycle :: NonEmpty a -> NonEmpty a
cycle = lift Prelude.cycle

insert :: (Foldable f, Ord a) => a -> f a -> NonEmpty a
insert a = fromList . List.insert a . P.toList

-- | One or more.
some1 :: Alternative f => f a -> f (NonEmpty a)
some1 x = liftA2 (:|) x (many x)

take :: Int -> NonEmpty a -> [a]
take n = Prelude.take n . toList

drop :: Int -> NonEmpty a -> [a]
drop n = Prelude.drop n . toList

splitAt :: Int -> NonEmpty a -> ([a], [a])
splitAt n = Prelude.splitAt n . toList

takeWhile :: (a -> Bool) -> NonEmpty a -> [a]
takeWhile p = Prelude.takeWhile p . toList

dropWhile :: (a -> Bool) -> NonEmpty a -> [a]
dropWhile p = Prelude.dropWhile p . toList

span :: (a -> Bool) -> NonEmpty a -> ([a], [a])
span p = Prelude.span p . toList

break :: (a -> Bool) -> NonEmpty a -> ([a], [a])
break p = span (not . p)

filter :: (a -> Bool) -> NonEmpty a -> [a]
filter p = Prelude.filter p . toList

partition :: (a -> Bool) -> NonEmpty a -> ([a], [a])
partition p = List.partition p . toList

group :: (Foldable f, Eq a) => f a -> [NonEmpty a]
group = groupBy (==)

groupBy :: Foldable f => (a -> a -> Bool) -> f a -> [NonEmpty a]
groupBy eq0 = go eq0 . P.toList
  where
	go _  [] = []
	go eq (x : xs) = (x :| ys) : groupBy eq zs
	  where (ys, zs) = Prelude.span (eq x) xs

groupWith :: (Foldable f, Eq b) => (a -> b) -> f a -> [NonEmpty a]
groupWith f = groupBy (\x y -> f x == f y)

groupAllWith :: Ord b => (a -> b) -> [a] -> [NonEmpty a]
groupAllWith f = groupWith f . List.sortBy (comparing f)

group1 :: Eq a => NonEmpty a -> NonEmpty (NonEmpty a)
group1 = groupBy1 (==)

groupBy1 :: (a -> a -> Bool) -> NonEmpty a -> NonEmpty (NonEmpty a)
groupBy1 eq (x :| xs) = (x :| ys) :| groupBy eq zs
  where (ys, zs) = Prelude.span (eq x) xs

groupWith1 :: Eq b => (a -> b) -> NonEmpty a -> NonEmpty (NonEmpty a)
groupWith1 f = groupBy1 (\x y -> f x == f y)

groupAllWith1 :: Ord b => (a -> b) -> NonEmpty a -> NonEmpty (NonEmpty a)
groupAllWith1 f = groupWith1 f . sortWith f

isPrefixOf :: Eq a => [a] -> NonEmpty a -> Bool
isPrefixOf xs ys = List.isPrefixOf xs (toList ys)

nub :: Eq a => NonEmpty a -> NonEmpty a
nub = nubBy (==)

nubBy :: (a -> a -> Bool) -> NonEmpty a -> NonEmpty a
nubBy eq (a :| as) = a :| List.nubBy eq (Prelude.filter (\b -> not (eq a b)) as)

-- | The element at an index, counting from zero.
(!!) :: NonEmpty a -> Int -> a
(!!) (x :| xs) n
	| n == 0    = x
	| n > 0     = xs Prelude.!! (n - 1)
	| otherwise = error "NonEmpty.!! negative index"
infixl 9 !!

zip :: NonEmpty a -> NonEmpty b -> NonEmpty (a, b)
zip (x :| xs) (y :| ys) = (x, y) :| Prelude.zip xs ys

zipWith :: (a -> b -> c) -> NonEmpty a -> NonEmpty b -> NonEmpty c
zipWith f (x :| xs) (y :| ys) = f x y :| Prelude.zipWith f xs ys

unzip :: Functor f => f (a, b) -> (f a, f b)
unzip xs = (fst <$> xs, snd <$> xs)

-- | Exclusive or of a non-empty list of booleans.
xor :: NonEmpty Bool -> Bool
xor (x :| xs) = foldr xor' x xs
  where xor' True y  = not y
	xor' False y = y
