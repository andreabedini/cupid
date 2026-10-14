-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Functor.Classes
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  experimental
-- Portability :  portable
--
-- Liftings of the Prelude classes 'Eq', 'Ord', 'Read' and 'Show' to
-- unary and binary type constructors, so that an instance for @T f a@
-- can be written once for every @f@:
--
-- > instance (Eq1 f, Eq a) => Eq (T f a) where (==) = eq1
--
-- 'Read1' and 'Read2' work with 'ReadS' only, since Cupid's 'Read' class
-- has no 'readPrec'; and the classes have no quantified superclasses,
-- which Cupid cannot express, so @Eq1 f@ does not imply @Eq (f a)@.

module Data.Functor.Classes (
	-- * Liftings of Prelude classes
	-- ** For unary constructors
	Eq1(..), eq1,
	Ord1(..), compare1,
	Read1(..), readsPrec1,
	Show1(..), showsPrec1,
	-- ** For binary constructors
	Eq2(..), eq2,
	Ord2(..), compare2,
	Read2(..), readsPrec2,
	Show2(..), showsPrec2,
	-- * Helper functions
	readsData,
	readsUnaryWith,
	readsBinaryWith,
	showsUnaryWith,
	showsBinaryWith,
	-- ** Obsolete helpers
	readsUnary,
	readsUnary1,
	readsBinary1,
	showsUnary,
	showsUnary1,
	showsBinary1
	) where

import Control.Applicative (Const(Const))
import Data.Functor.Identity (Identity(Identity))
import Data.Ord (Down(Down))
import Data.Proxy (Proxy(Proxy))
import Cupid.Prelude (NonEmpty(..))
import Text.Show (showListWith)

-- | Lifting of 'Eq' to unary type constructors.
class Eq1 f where
	-- | Lift an equality test through the type constructor.
	liftEq :: (a -> b -> Bool) -> f a -> f b -> Bool

-- | Lift the standard '(==)' function through the type constructor.
eq1 :: (Eq1 f, Eq a) => f a -> f a -> Bool
eq1 = liftEq (==)

-- | Lifting of 'Ord' to unary type constructors.
class Eq1 f => Ord1 f where
	-- | Lift a 'compare' function through the type constructor.
	liftCompare :: (a -> b -> Ordering) -> f a -> f b -> Ordering

-- | Lift the standard 'compare' function through the type constructor.
compare1 :: (Ord1 f, Ord a) => f a -> f a -> Ordering
compare1 = liftCompare compare

-- | Lifting of 'Read' to unary type constructors.
class Read1 f where
	-- | 'readsPrec' for @f a@, given 'readsPrec' and 'readList' for @a@.
	liftReadsPrec :: (Int -> ReadS a) -> ReadS [a] -> Int -> ReadS (f a)

	-- | 'readList' for @f a@, given 'readsPrec' and 'readList' for @a@.
	liftReadList :: (Int -> ReadS a) -> ReadS [a] -> ReadS [f a]
	liftReadList rp rl = readListWith (liftReadsPrec rp rl 0)

-- | Lift the standard 'readsPrec' and 'readList' functions through the
-- type constructor.
readsPrec1 :: (Read1 f, Read a) => Int -> ReadS (f a)
readsPrec1 = liftReadsPrec readsPrec readList

-- | Lifting of 'Show' to unary type constructors.
class Show1 f where
	-- | 'showsPrec' for @f a@, given 'showsPrec' and 'showList' for @a@.
	liftShowsPrec :: (Int -> a -> ShowS) -> ([a] -> ShowS) -> Int -> f a -> ShowS

	-- | 'showList' for @f a@, given 'showsPrec' and 'showList' for @a@.
	liftShowList :: (Int -> a -> ShowS) -> ([a] -> ShowS) -> [f a] -> ShowS
	liftShowList sp sl = showListWith (liftShowsPrec sp sl 0)

-- | Lift the standard 'showsPrec' and 'showList' functions through the
-- type constructor.
showsPrec1 :: (Show1 f, Show a) => Int -> f a -> ShowS
showsPrec1 = liftShowsPrec showsPrec showList

-- | Lifting of 'Eq' to binary type constructors.
class Eq2 f where
	-- | Lift equality tests through the type constructor.
	liftEq2 :: (a -> b -> Bool) -> (c -> d -> Bool) -> f a c -> f b d -> Bool

-- | Lift the standard '(==)' function through the type constructor.
eq2 :: (Eq2 f, Eq a, Eq b) => f a b -> f a b -> Bool
eq2 = liftEq2 (==) (==)

-- | Lifting of 'Ord' to binary type constructors.
class Eq2 f => Ord2 f where
	-- | Lift 'compare' functions through the type constructor.
	liftCompare2 :: (a -> b -> Ordering) -> (c -> d -> Ordering) ->
		f a c -> f b d -> Ordering

-- | Lift the standard 'compare' function through the type constructor.
compare2 :: (Ord2 f, Ord a, Ord b) => f a b -> f a b -> Ordering
compare2 = liftCompare2 compare compare

-- | Lifting of 'Read' to binary type constructors.
class Read2 f where
	-- | 'readsPrec' for @f a b@, given those for @a@ and @b@.
	liftReadsPrec2 :: (Int -> ReadS a) -> ReadS [a] ->
		(Int -> ReadS b) -> ReadS [b] -> Int -> ReadS (f a b)

	-- | 'readList' for @f a b@, given those for @a@ and @b@.
	liftReadList2 :: (Int -> ReadS a) -> ReadS [a] ->
		(Int -> ReadS b) -> ReadS [b] -> ReadS [f a b]
	liftReadList2 rp1 rl1 rp2 rl2 =
		readListWith (liftReadsPrec2 rp1 rl1 rp2 rl2 0)

-- | Lift the standard 'readsPrec' function through the type constructor.
readsPrec2 :: (Read2 f, Read a, Read b) => Int -> ReadS (f a b)
readsPrec2 = liftReadsPrec2 readsPrec readList readsPrec readList

-- | Lifting of 'Show' to binary type constructors.
class Show2 f where
	-- | 'showsPrec' for @f a b@, given those for @a@ and @b@.
	liftShowsPrec2 :: (Int -> a -> ShowS) -> ([a] -> ShowS) ->
		(Int -> b -> ShowS) -> ([b] -> ShowS) -> Int -> f a b -> ShowS

	-- | 'showList' for @f a b@, given those for @a@ and @b@.
	liftShowList2 :: (Int -> a -> ShowS) -> ([a] -> ShowS) ->
		(Int -> b -> ShowS) -> ([b] -> ShowS) -> [f a b] -> ShowS
	liftShowList2 sp1 sl1 sp2 sl2 =
		showListWith (liftShowsPrec2 sp1 sl1 sp2 sl2 0)

-- | Lift the standard 'showsPrec' function through the type constructor.
showsPrec2 :: (Show2 f, Show a, Show b) => Int -> f a b -> ShowS
showsPrec2 = liftShowsPrec2 showsPrec showList showsPrec showList

-- | Read a list in the syntax of 'showList', with the given reader for
-- its elements.
readListWith :: ReadS a -> ReadS [a]
readListWith rp = readParen False $ \r ->
	[ pr | ("[", s) <- lex r, pr <- readl s ]
  where
	readl s = [ ([], t) | ("]", t) <- lex s ] ++
		[ (x : xs, u) | (x, t) <- rp s, (xs, u) <- readl' t ]
	readl' s = [ ([], t) | ("]", t) <- lex s ] ++
		[ (x : xs, v) | (",", t) <- lex s, (x, u) <- rp t, (xs, v) <- readl' u ]

-- Instances for the Prelude's types

instance Eq1 Maybe where
	liftEq _ Nothing Nothing = True
	liftEq _ Nothing (Just _) = False
	liftEq _ (Just _) Nothing = False
	liftEq eq (Just x) (Just y) = eq x y

instance Ord1 Maybe where
	liftCompare _ Nothing Nothing = EQ
	liftCompare _ Nothing (Just _) = LT
	liftCompare _ (Just _) Nothing = GT
	liftCompare comp (Just x) (Just y) = comp x y

instance Read1 Maybe where
	liftReadsPrec rp _ d =
		readParen False (\r -> [ (Nothing, s) | ("Nothing", s) <- lex r ])
		`mappend` readsData (readsUnaryWith rp "Just" Just) d

instance Show1 Maybe where
	liftShowsPrec _ _ _ Nothing = showString "Nothing"
	liftShowsPrec sp _ d (Just x) = showsUnaryWith sp "Just" d x

instance Eq1 [] where
	liftEq _ [] [] = True
	liftEq _ [] (_:_) = False
	liftEq _ (_:_) [] = False
	liftEq eq (x:xs) (y:ys) = eq x y && liftEq eq xs ys

instance Ord1 [] where
	liftCompare _ [] [] = EQ
	liftCompare _ [] (_:_) = LT
	liftCompare _ (_:_) [] = GT
	liftCompare comp (x:xs) (y:ys) = comp x y `mappend` liftCompare comp xs ys

instance Read1 [] where
	liftReadsPrec _ rl _ = rl

instance Show1 [] where
	liftShowsPrec _ sl _ = sl

instance Eq1 NonEmpty where
	liftEq eq (a :| as) (b :| bs) = eq a b && liftEq eq as bs

instance Ord1 NonEmpty where
	liftCompare cmp (a :| as) (b :| bs) = cmp a b `mappend` liftCompare cmp as bs

instance Read1 NonEmpty where
	liftReadsPrec rp rl d = readParen (d > 5) $ \s0 ->
		[ (a :| as, s3) | (a, s1) <- rp 6 s0, (":|", s2) <- lex s1,
				  (as, s3) <- rl s2 ]

instance Show1 NonEmpty where
	liftShowsPrec sp sl d (a :| as) = showParen (d > 5) $
		sp 6 a . showString " :| " . sl as

instance Eq2 (,) where
	liftEq2 e1 e2 (x1, y1) (x2, y2) = e1 x1 x2 && e2 y1 y2

instance Ord2 (,) where
	liftCompare2 comp1 comp2 (x1, y1) (x2, y2) =
		comp1 x1 x2 `mappend` comp2 y1 y2

instance Read2 (,) where
	liftReadsPrec2 rp1 _ rp2 _ _ = readParen False $ \r ->
		[ ((x, y), w) | ("(", s) <- lex r, (x, t) <- rp1 0 s,
				(",", u) <- lex t, (y, v) <- rp2 0 u, (")", w) <- lex v ]

instance Show2 (,) where
	liftShowsPrec2 sp1 _ sp2 _ _ (x, y) =
		showChar '(' . sp1 0 x . showChar ',' . sp2 0 y . showChar ')'

instance Eq a => Eq1 ((,) a) where
	liftEq = liftEq2 (==)

instance Ord a => Ord1 ((,) a) where
	liftCompare = liftCompare2 compare

instance Read a => Read1 ((,) a) where
	liftReadsPrec = liftReadsPrec2 readsPrec readList

instance Show a => Show1 ((,) a) where
	liftShowsPrec = liftShowsPrec2 showsPrec showList

instance Eq2 Either where
	liftEq2 e1 _ (Left x) (Left y) = e1 x y
	liftEq2 _ _ (Left _) (Right _) = False
	liftEq2 _ _ (Right _) (Left _) = False
	liftEq2 _ e2 (Right x) (Right y) = e2 x y

instance Ord2 Either where
	liftCompare2 comp1 _ (Left x) (Left y) = comp1 x y
	liftCompare2 _ _ (Left _) (Right _) = LT
	liftCompare2 _ _ (Right _) (Left _) = GT
	liftCompare2 _ comp2 (Right x) (Right y) = comp2 x y

instance Read2 Either where
	liftReadsPrec2 rp1 _ rp2 _ = readsData $
		readsUnaryWith rp1 "Left" Left `mappend`
		readsUnaryWith rp2 "Right" Right

instance Show2 Either where
	liftShowsPrec2 sp1 _ _ _ d (Left x) = showsUnaryWith sp1 "Left" d x
	liftShowsPrec2 _ _ sp2 _ d (Right x) = showsUnaryWith sp2 "Right" d x

instance Eq a => Eq1 (Either a) where
	liftEq = liftEq2 (==)

instance Ord a => Ord1 (Either a) where
	liftCompare = liftCompare2 compare

instance Read a => Read1 (Either a) where
	liftReadsPrec = liftReadsPrec2 readsPrec readList

instance Show a => Show1 (Either a) where
	liftShowsPrec = liftShowsPrec2 showsPrec showList

-- Instances for other functors defined in base

instance Eq1 Identity where
	liftEq eq (Identity x) (Identity y) = eq x y

instance Ord1 Identity where
	liftCompare comp (Identity x) (Identity y) = comp x y

instance Read1 Identity where
	liftReadsPrec rp _ = readsData $ readsUnaryWith rp "Identity" Identity

instance Show1 Identity where
	liftShowsPrec sp _ d (Identity x) = showsUnaryWith sp "Identity" d x

instance Eq2 Const where
	liftEq2 eq _ (Const x) (Const y) = eq x y

instance Ord2 Const where
	liftCompare2 comp _ (Const x) (Const y) = comp x y

instance Read2 Const where
	liftReadsPrec2 rp _ _ _ = readsData $ readsUnaryWith rp "Const" Const

instance Show2 Const where
	liftShowsPrec2 sp _ _ _ d (Const x) = showsUnaryWith sp "Const" d x

instance Eq a => Eq1 (Const a) where
	liftEq = liftEq2 (==)

instance Ord a => Ord1 (Const a) where
	liftCompare = liftCompare2 compare

instance Read a => Read1 (Const a) where
	liftReadsPrec = liftReadsPrec2 readsPrec readList

instance Show a => Show1 (Const a) where
	liftShowsPrec = liftShowsPrec2 showsPrec showList

instance Eq1 Proxy where
	liftEq _ _ _ = True

instance Ord1 Proxy where
	liftCompare _ _ _ = EQ

instance Show1 Proxy where
	liftShowsPrec _ _ _ _ = showString "Proxy"

instance Read1 Proxy where
	liftReadsPrec _ _ d = readParen (d > 10) (\r -> [ (Proxy, s) | ("Proxy", s) <- lex r ])

instance Eq1 Down where
	liftEq eq (Down x) (Down y) = eq x y

instance Ord1 Down where
	liftCompare comp (Down x) (Down y) = case comp x y of
		LT -> GT
		EQ -> EQ
		GT -> LT

instance Read1 Down where
	liftReadsPrec rp _ = readsData $ readsUnaryWith rp "Down" Down

instance Show1 Down where
	liftShowsPrec sp _ d (Down x) = showsUnaryWith sp "Down" d x

-- Building blocks

-- | @'readsData' p d@ parses a value of a data type, whose alternatives
-- @p@ parses at precedence @d@: the arguments of a constructor are read
-- at precedence 11, so a value with any is in parentheses above 10.
readsData :: (String -> ReadS a) -> Int -> ReadS a
readsData reader d =
	readParen (d > 10) $ \r -> [ res | (s, t) <- lex r, res <- reader s t ]

-- | @'readsUnaryWith' rp n c n'@ matches the name of a unary data
-- constructor @n'@ against @n@ and then parses its argument with @rp@.
readsUnaryWith :: (Int -> ReadS a) -> String -> (a -> t) -> String -> ReadS t
readsUnaryWith rp name cons kw s =
	[ (cons x, t) | kw == name, (x, t) <- rp 11 s ]

-- | @'readsBinaryWith' rp1 rp2 n c n'@ matches the name of a binary data
-- constructor and then parses its arguments with @rp1@ and @rp2@.
readsBinaryWith :: (Int -> ReadS a) -> (Int -> ReadS b) ->
	String -> (a -> b -> t) -> String -> ReadS t
readsBinaryWith rp1 rp2 name cons kw s =
	[ (cons x y, u) | kw == name, (x, t) <- rp1 11 s, (y, u) <- rp2 11 t ]

-- | @'showsUnaryWith' sp n d x@ shows the application of the constructor
-- named @n@ to @x@, at precedence @d@.
showsUnaryWith :: (Int -> a -> ShowS) -> String -> Int -> a -> ShowS
showsUnaryWith sp name d x = showParen (d > 10) $
	showString name . showChar ' ' . sp 11 x

-- | @'showsBinaryWith' sp1 sp2 n d x y@ shows the application of the
-- constructor named @n@ to @x@ and @y@, at precedence @d@.
showsBinaryWith :: (Int -> a -> ShowS) -> (Int -> b -> ShowS) ->
	String -> Int -> a -> b -> ShowS
showsBinaryWith sp1 sp2 name d x y = showParen (d > 10) $
	showString name . showChar ' ' . sp1 11 x . showChar ' ' . sp2 11 y

-- | Read the argument of a unary constructor with 'readsPrec'.
readsUnary :: Read a => String -> (a -> t) -> String -> ReadS t
readsUnary name cons kw s =
	[ (cons x, t) | kw == name, (x, t) <- readsPrec 11 s ]

-- | Read the argument of a unary constructor with 'readsPrec1'.
readsUnary1 :: (Read1 f, Read a) => String -> (f a -> t) -> String -> ReadS t
readsUnary1 name cons kw s =
	[ (cons x, t) | kw == name, (x, t) <- readsPrec1 11 s ]

-- | Read the arguments of a binary constructor with 'readsPrec1'.
readsBinary1 :: (Read1 f, Read1 g, Read a) =>
	String -> (f a -> g a -> t) -> String -> ReadS t
readsBinary1 name cons kw s =
	[ (cons x y, u) | kw == name, (x, t) <- readsPrec1 11 s, (y, u) <- readsPrec1 11 t ]

-- | Show the argument of a unary constructor with 'showsPrec'.
showsUnary :: Show a => String -> Int -> a -> ShowS
showsUnary name d x = showsUnaryWith showsPrec name d x

-- | Show the argument of a unary constructor with 'showsPrec1'.
showsUnary1 :: (Show1 f, Show a) => String -> Int -> f a -> ShowS
showsUnary1 name d x = showsUnaryWith showsPrec1 name d x

-- | Show the arguments of a binary constructor with 'showsPrec1'.
showsBinary1 :: (Show1 f, Show1 g, Show a) => String -> Int -> f a -> g a -> ShowS
showsBinary1 name d x y = showsBinaryWith showsPrec1 showsPrec1 name d x y
