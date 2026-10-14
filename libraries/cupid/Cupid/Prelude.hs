{----------------------------------------------------------------------------
__   __ __  __  ____   ___    _______________________________________________
||   || ||  || ||  || ||__    Hugs 98: The Nottingham and Yale Haskell system
||___|| ||__|| ||__||  __||   Copyright (c) 1994-1999
||---||         ___||         World Wide Web: http://haskell.org/hugs
||   ||                       Report bugs to: hugs-bugs@haskell.org
||   || Version: February 1999_______________________________________________

 This is the Hugs 98 Standard Prelude, based very closely on the Standard
 Prelude for Haskell 98.

 WARNING: This file is an integral part of the Cupid source code.  Changes to
 the definitions in this file without corresponding modifications in other
 parts of the program may cause the interpreter to fail unexpectedly.  Under
 normal circumstances, you should not attempt to modify this file in any way!

-----------------------------------------------------------------------------
 The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 1994-2003, All rights reserved.  It is distributed as free software under
 the license in the file "License", which is included in the distribution.
----------------------------------------------------------------------------}

module Cupid.Prelude (
--  module PreludeList,
    map, (++), concat, filter,
    head, last, tail, init, (!!),
    scanl, scanl1, scanr, scanr1,
    iterate, repeat, replicate, cycle,
    take, drop, splitAt, takeWhile, dropWhile, span, break,
    lines, words, unlines, unwords, reverse, and, or,
    any, all, notElem, lookup,
    concatMap,
    zip, zip3, zipWith, zipWith3, unzip, unzip3,
    Foldable(fold, foldMap, foldMap', foldr, foldr', foldl, foldl',
             foldr1, foldl1, toList, null, length, elem, maximum,
             minimum, sum, product),
    Traversable(traverse, sequenceA, mapM, sequence),
    find, maximumBy, minimumBy, mapAccumL, mapAccumR, forM, forM_, msum,
    Endo(..),
--  module PreludeText, 
    ReadS, ShowS,
    Read(readsPrec, readList),
    Show(show, showsPrec, showList),
    reads, shows, read, lex,
    showChar, showString, readParen, showParen,
--  module PreludeIO,
    FilePath, IOError, ioError, userError, catch,
    putChar, putStr, putStrLn, print,
    getChar, getLine, getContents, interact,
    readFile, writeFile, appendFile, readIO, readLn,
--  module Ix,
    Ix(range, index, unsafeIndex, inRange, rangeSize),
--  module Char,
    isSpace, isUpper, isLower,
    isAlpha, isDigit, isOctDigit, isHexDigit, isAlphaNum,
    readLitChar, showLitChar, lexLitChar,
--  module Numeric
    readSigned, readInt,
    readDec, readOct, readHex, readSigned,
    readFloat, lexDigits, 
--  module Ratio,
    Ratio((:%)), (%), numerator, denominator,
--  Non-standard exports
    IO(..), IOResult(..),
    IOException(..), IOErrorType(..),
    Exception(..),
    ArithException(..), ArrayException(..), AsyncException(..),
    ExitCode(..),
    FunPtr, Ptr, Addr,
    Word, StablePtr, ForeignObj, ForeignPtr,
    Int8, Int16, Int32, Int64,
    Word8, Word16, Word32, Word64,
    Handle, Object,
    basicIORun, blockIO, IOFinished(..),
    threadToIOResult,
    catchException, throw,
    Dynamic(..), TypeRep(..), Key(..), TyCon(..), Obj,

    IOMode(..),
    stdin, stdout, stderr,
    openFile,
    hClose,
    hGetContents, hGetChar, hGetLine,
    hPutChar, hPutStr,

    Bool(False, True),
    Maybe(Nothing, Just),
    Either(Left, Right),
    Ordering(LT, EQ, GT),
    Char, String, Int, Integer, Float, Double, Rational, IO,
--  List type: []((:), [])
    (:),
--  Tuple types: (,), (,,), etc.
--  Trivial type: ()
--  Functions: (->)
    Eq((==), (/=)),
    Ord(compare, (<), (<=), (>=), (>), max, min),
    Enum(succ, pred, toEnum, fromEnum, enumFrom, enumFromThen,
         enumFromTo, enumFromThenTo),
    Bounded(minBound, maxBound),
--  Num((+), (-), (*), negate, abs, signum, fromInteger),
    Num((+), (-), (*), negate, abs, signum, fromInteger, fromInt),
    Real(toRational),
--  Integral(quot, rem, div, mod, quotRem, divMod, toInteger),
    Integral(quot, rem, div, mod, quotRem, divMod, toInteger, toInt),
--  Fractional((/), recip, fromRational),
    Fractional((/), recip, fromRational, fromDouble),
    Floating(pi, exp, log, sqrt, (**), logBase, sin, cos, tan,
             asin, acos, atan, sinh, cosh, tanh, asinh, acosh, atanh),
    RealFrac(properFraction, truncate, round, ceiling, floor),
    RealFloat(floatRadix, floatDigits, floatRange, decodeFloat,
              encodeFloat, exponent, significand, scaleFloat, isNaN,
              isInfinite, isDenormalized, isIEEE, isNegativeZero, atan2),
    Functor(fmap, (<$)), (<$>),
    Applicative(pure, (<*>), liftA2, (*>), (<*)),
    Monad((>>=), (>>), return),
    MonadFail(fail),
    Alternative(empty, (<|>), some, many),
    Semigroup((<>), sconcat, stimes),
    Monoid(mempty, mappend, mconcat),
    NonEmpty((:|)),
    MonadPlus(mzero, mplus),
    mapM_, sequence_, (=<<),
    maybe, either,
    (&&), (||), not, otherwise,
    subtract, even, odd, gcd, lcm, (^), (^^), 
    fromIntegral, realToFrac,
    fst, snd, curry, uncurry, id, const, (.), flip, ($), until,
    asTypeOf, error, undefined,
    seq, ($!),

    boundedSucc,
    boundedPred,
    boundedEnumFrom,
    boundedEnumFromTo,
    boundedEnumFromThen,
    boundedEnumFromThenTo
  ) where

-- Standard value bindings {Prelude} ----------------------------------------

infixr 9  .
infixl 9  !!
infixr 8  ^, ^^, **
infixl 7  *, /, `quot`, `rem`, `div`, `mod`, :%, %
infixl 6  +, -
--infixr 5  :    -- this fixity declaration is hard-wired into Cupid
infixr 6  <>
infixr 5  ++, :|
infix  4  ==, /=, <, <=, >=, >, `elem`, `notElem`
infixr 3  &&
infixr 2  ||
infixl 4  <$>, <$, <*>, *>, <*
infixl 3  <|>
infixl 1  >>, >>=
infixr 1  =<<
infixr 0  $, $!, `seq`

-- Equality and Ordered classes ---------------------------------------------

class Eq a where
    (==), (/=) :: a -> a -> Bool

    -- Minimal complete definition: (==) or (/=)
    x == y      = not (x/=y)
    x /= y      = not (x==y)

class (Eq a) => Ord a where
    compare                :: a -> a -> Ordering
    (<), (<=), (>=), (>)   :: a -> a -> Bool
    max, min               :: a -> a -> a

    -- Minimal complete definition: (<=) or compare
    -- using compare can be more efficient for complex types
    compare x y | x==y      = EQ
		| x<=y      = LT
		| otherwise = GT

    x <= y                  = compare x y /= GT
    x <  y                  = compare x y == LT
    x >= y                  = compare x y /= LT
    x >  y                  = compare x y == GT

    max x y   | x <= y      = y
	      | otherwise   = x
    min x y   | x <= y      = x
	      | otherwise   = y

class Bounded a where
    minBound, maxBound :: a
    -- Minimal complete definition: All

-- Numeric classes ----------------------------------------------------------

class (Eq a, Show a) => Num a where
    (+), (-), (*)  :: a -> a -> a
    negate         :: a -> a
    abs, signum    :: a -> a
    fromInteger    :: Integer -> a
    fromInt        :: Int -> a

    -- Minimal complete definition: All, except negate or (-)
    x - y           = x + negate y
    fromInt         = fromIntegral
    negate x        = 0 - x

class (Num a, Ord a) => Real a where
    toRational     :: a -> Rational

class (Real a, Enum a) => Integral a where
    quot, rem, div, mod :: a -> a -> a
    quotRem, divMod     :: a -> a -> (a,a)
    toInteger           :: a -> Integer
    toInt               :: a -> Int

    -- Minimal complete definition: quotRem and toInteger
    n `quot` d           = q where (q,r) = quotRem n d
    n `rem` d            = r where (q,r) = quotRem n d
    n `div` d            = q where (q,r) = divMod n d
    n `mod` d            = r where (q,r) = divMod n d
    divMod n d           = if signum r == - signum d then (q-1, r+d) else qr
			   where qr@(q,r) = quotRem n d
    toInt                = toInt . toInteger

class (Num a) => Fractional a where
    (/)          :: a -> a -> a
    recip        :: a -> a
    fromRational :: Rational -> a
    fromDouble   :: Double -> a

    -- Minimal complete definition: fromRational and ((/) or recip)
    recip x       = 1 / x
    fromDouble    = fromRational . fromDouble
    x / y         = x * recip y


class (Fractional a) => Floating a where
    pi                  :: a
    exp, log, sqrt      :: a -> a
    (**), logBase       :: a -> a -> a
    sin, cos, tan       :: a -> a
    asin, acos, atan    :: a -> a
    sinh, cosh, tanh    :: a -> a
    asinh, acosh, atanh :: a -> a

    -- Minimal complete definition: pi, exp, log, sin, cos, sinh, cosh,
    --				    asinh, acosh, atanh
    pi                   = 4 * atan 1
    x ** y               = exp (log x * y)
    logBase x y          = log y / log x
    sqrt x               = x ** 0.5
    tan x                = sin x / cos x
    sinh x               = (exp x - exp (-x)) / 2
    cosh x               = (exp x + exp (-x)) / 2
    tanh x               = sinh x / cosh x
    asinh x              = log (x + sqrt (x*x + 1))
    acosh x              = log (x + sqrt (x*x - 1))
    atanh x              = (log (1 + x) - log (1 - x)) / 2

class (Real a, Fractional a) => RealFrac a where
    properFraction   :: (Integral b) => a -> (b,a)
    truncate, round  :: (Integral b) => a -> b
    ceiling, floor   :: (Integral b) => a -> b

    -- Minimal complete definition: properFraction
    truncate x        = m where (m,_) = properFraction x

    round x           = let (n,r) = properFraction x
			    m     = if r < 0 then n - 1 else n + 1
			in case signum (abs r - 0.5) of
			    -1 -> n
			    0  -> if even n then n else m
			    1  -> m

    ceiling x         = if r > 0 then n + 1 else n
			where (n,r) = properFraction x

    floor x           = if r < 0 then n - 1 else n
			where (n,r) = properFraction x

class (RealFrac a, Floating a) => RealFloat a where
    floatRadix       :: a -> Integer
    floatDigits      :: a -> Int
    floatRange       :: a -> (Int,Int)
    decodeFloat      :: a -> (Integer,Int)
    encodeFloat      :: Integer -> Int -> a
    exponent         :: a -> Int
    significand      :: a -> a
    scaleFloat       :: Int -> a -> a
    isNaN, isInfinite, isDenormalized, isNegativeZero, isIEEE
		     :: a -> Bool
    atan2	     :: a -> a -> a

    -- Minimal complete definition: All, except exponent, signficand,
    --				    scaleFloat, atan2
    exponent x        = if m==0 then 0 else n + floatDigits x
			where (m,n) = decodeFloat x
    significand x     = encodeFloat m (- floatDigits x)
			where (m,_) = decodeFloat x
    scaleFloat k x    = encodeFloat m (n+k)
			where (m,n) = decodeFloat x
    atan2 y x
      | x>0           = atan (y/x)
      | x==0 && y>0   = pi/2
      | x<0 && y>0    = pi + atan (y/x)
      | (x<=0 && y<0) ||
        (x<0 && isNegativeZero y) ||
        (isNegativeZero x && isNegativeZero y)
		      = - atan2 (-y) x
      | y==0 && (x<0 || isNegativeZero x)
		      = pi    -- must be after the previous test on zero y
      | x==0 && y==0  = y     -- must be after the other double zero tests
      | otherwise     = x + y -- x or y is a NaN, return a NaN (via +)

-- Numeric functions --------------------------------------------------------

subtract       :: Num a => a -> a -> a
subtract        = flip (-)

even, odd        :: (Integral a) => a -> Bool
even n           =  n `rem` 2 == 0
odd              =  not . even

gcd            :: Integral a => a -> a -> a
gcd 0 0         = error "Prelude.gcd: gcd 0 0 is undefined"
gcd x y         = gcd' (abs x) (abs y)
		  where gcd' x 0 = x
			gcd' x y = gcd' y (x `rem` y)

lcm            :: (Integral a) => a -> a -> a
lcm _ 0         = 0
lcm 0 _         = 0
lcm x y         = abs ((x `quot` gcd x y) * y)

(^)            :: (Num a, Integral b) => a -> b -> a
x ^ 0           = 1
x ^ n  | n > 0  = f x (n-1) x
		  where f _ 0 y = y
			f x n y = g x n where
				  g x n | even n    = g (x*x) (n`quot`2)
					| otherwise = f x (n-1) (x*y)
_ ^ _           = error "Prelude.^: negative exponent"

(^^)           :: (Fractional a, Integral b) => a -> b -> a
x ^^ n          = if n >= 0 then x ^ n else recip (x^(-n))

fromIntegral   :: (Integral a, Num b) => a -> b
fromIntegral    = fromInteger . toInteger

realToFrac     :: (Real a, Fractional b) => a -> b
realToFrac      = fromRational . toRational

-- Index and Enumeration classes --------------------------------------------

class (Ord a) => Ix a where
    range                :: (a,a) -> [a]
	-- The unchecked variant unsafeIndex is non-standard, but useful
    index, unsafeIndex   :: (a,a) -> a -> Int
    inRange              :: (a,a) -> a -> Bool
    rangeSize            :: (a,a) -> Int

	-- Must specify one of index, unsafeIndex
    index b i | inRange b i = unsafeIndex b i
              | otherwise   = error "Ix.index: index out of range"
    unsafeIndex b i = index b i

    rangeSize b@(_l,h) | inRange b h = unsafeIndex b h + 1
                       | otherwise   = 0
	-- NB: replacing "inRange b h" by  "l <= u"
	-- fails if the bounds are tuples.  For example,
	-- 	(1,2) <= (2,1)
	-- but the range is nevertheless empty
	--	range ((1,2),(2,1)) = []

class Enum a where
    succ, pred           :: a -> a
    toEnum               :: Int -> a
    fromEnum             :: a -> Int
    enumFrom             :: a -> [a]              -- [n..]
    enumFromThen         :: a -> a -> [a]         -- [n,m..]
    enumFromTo           :: a -> a -> [a]         -- [n..m]
    enumFromThenTo       :: a -> a -> a -> [a]    -- [n,n'..m]

    -- Minimal complete definition: toEnum, fromEnum
    succ                  = toEnum . (1+)       . fromEnum
    pred                  = toEnum . subtract 1 . fromEnum
    enumFrom x            = map toEnum [ fromEnum x ..]
    enumFromTo x y        = map toEnum [ fromEnum x .. fromEnum y ]
    enumFromThen x y      = map toEnum [ fromEnum x, fromEnum y ..]
    enumFromThenTo x y z  = map toEnum [ fromEnum x, fromEnum y .. fromEnum z ]

-- Read and Show classes ------------------------------------------------------

type ReadS a = String -> [(a,String)]
type ShowS   = String -> String

class Read a where
    readsPrec :: Int -> ReadS a
    readList  :: ReadS [a]

    -- Minimal complete definition: readsPrec
    readList   = readParen False (\r -> [pr | ("[",s) <- lex r,
					      pr      <- readl s ])
		 where readl  s = [([],t)   | ("]",t) <- lex s] ++
				  [(x:xs,u) | (x,t)   <- reads s,
					      (xs,u)  <- readl' t]
		       readl' s = [([],t)   | ("]",t) <- lex s] ++
				  [(x:xs,v) | (",",t) <- lex s,
					      (x,u)   <- reads t,
					      (xs,v)  <- readl' u]

class Show a where
    show      :: a -> String
    showsPrec :: Int -> a -> ShowS
    showList  :: [a] -> ShowS

    -- Minimal complete definition: show or showsPrec
    show x          = showsPrec 0 x ""
    showsPrec _ x s = show x ++ s
    showList []     = showString "[]"
    showList (x:xs) = showChar '[' . shows x . showl xs
		      where showl []     = showChar ']'
			    showl (x:xs) = showChar ',' . shows x . showl xs

-- Monad classes ------------------------------------------------------------

-- The hierarchy of GHC's base rather than of Haskell 2010: Applicative
-- is a superclass of Monad, and fail is in a class of its own, which
-- do-notation uses only for a pattern that can fail to match.

class Functor f where
    fmap :: (a -> b) -> (f a -> f b)
    (<$) :: a -> f b -> f a

    -- Minimal complete definition: fmap
    (<$) = fmap . const

class Functor f => Applicative f where
    pure   :: a -> f a
    (<*>)  :: f (a -> b) -> f a -> f b
    liftA2 :: (a -> b -> c) -> f a -> f b -> f c
    (*>)   :: f a -> f b -> f b
    (<*)   :: f a -> f b -> f a

    -- Minimal complete definition: pure, and (<*>) or liftA2
    (<*>)        = liftA2 id
    liftA2 f x y = fmap f x <*> y
    a *> b       = (id <$ a) <*> b
    a <* b       = liftA2 const a b

class Applicative m => Monad m where
    (>>=)  :: m a -> (a -> m b) -> m b
    (>>)   :: m a -> m b -> m b
    return :: a -> m a

    -- Minimal complete definition: (>>=)
    p >> q  = p >>= \ _ -> q
    return  = pure

class Monad m => MonadFail m where
    fail :: String -> m a

class Applicative f => Alternative f where
    empty :: f a
    (<|>) :: f a -> f a -> f a
    some  :: f a -> f [a]
    many  :: f a -> f [a]

    -- Minimal complete definition: empty, (<|>)
    some v = some_v
      where many_v = some_v <|> pure []
            some_v = liftA2 (:) v many_v
    many v = many_v
      where many_v = some_v <|> pure []
            some_v = liftA2 (:) v many_v

class (Alternative m, Monad m) => MonadPlus m where
    mzero :: m a
    mplus :: m a -> m a -> m a

    mzero = empty
    mplus = (<|>)

(<$>)            :: Functor f => (a -> b) -> f a -> f b
f <$> x           = fmap f x

(=<<)            :: Monad m => (a -> m b) -> m a -> m b
f =<< x           = x >>= f

-- Semigroups and monoids --------------------------------------------------

-- As in GHC's base, Semigroup is a superclass of Monoid, and mappend
-- defaults to (<>).

class Semigroup a where
    (<>)    :: a -> a -> a
    sconcat :: NonEmpty a -> a
    stimes  :: Integral b => b -> a -> a

    -- Minimal complete definition: (<>)
    sconcat (a :| as) = go a as
      where go b (c:cs) = b <> go c cs
            go b []     = b

    stimes y0 x0
      | y0 <= 0   = error "stimes: positive multiplier expected"
      | otherwise = f x0 y0
      where
        f x y
          | even y    = f (x <> x) (y `quot` 2)
          | y == 1    = x
          | otherwise = g (x <> x) (y `quot` 2) x
        g x y z
          | even y    = g (x <> x) (y `quot` 2) z
          | y == 1    = x <> z
          | otherwise = g (x <> x) (y `quot` 2) (x <> z)

class Semigroup a => Monoid a where
    mempty  :: a
    mappend :: a -> a -> a
    mconcat :: [a] -> a

    -- Minimal complete definition: mempty
    mappend = (<>)
    mconcat = foldr mappend mempty

-- | Non-empty lists, which sconcat takes; Data.List.NonEmpty has the
-- functions on them.
data NonEmpty a = a :| [a]
                  deriving (Eq, Ord, Read, Show)

instance Semigroup (NonEmpty a) where
    (a :| as) <> (b :| bs) = a :| (as ++ b : bs)

instance Functor NonEmpty where
    fmap f (a :| as) = f a :| map f as

instance Semigroup [a] where
    (<>) = (++)

instance Monoid [a] where
    mempty  = []
    mconcat = concat

instance Semigroup b => Semigroup (a -> b) where
    (f <> g) x = f x <> g x

instance Monoid b => Monoid (a -> b) where
    mempty _ = mempty

instance Semigroup () where
    _ <> _    = ()
    sconcat _ = ()
    stimes _ _ = ()

instance Monoid () where
    mempty    = ()
    mconcat _ = ()

instance (Semigroup a, Semigroup b) => Semigroup (a,b) where
    (a,b) <> (a',b') = (a<>a', b<>b')

instance (Monoid a, Monoid b) => Monoid (a,b) where
    mempty = (mempty, mempty)

instance (Semigroup a, Semigroup b, Semigroup c) => Semigroup (a,b,c) where
    (a,b,c) <> (a',b',c') = (a<>a', b<>b', c<>c')

instance (Monoid a, Monoid b, Monoid c) => Monoid (a,b,c) where
    mempty = (mempty, mempty, mempty)

instance (Semigroup a, Semigroup b, Semigroup c, Semigroup d) =>
         Semigroup (a,b,c,d) where
    (a,b,c,d) <> (a',b',c',d') = (a<>a', b<>b', c<>c', d<>d')

instance (Monoid a, Monoid b, Monoid c, Monoid d) => Monoid (a,b,c,d) where
    mempty = (mempty, mempty, mempty, mempty)

instance (Semigroup a, Semigroup b, Semigroup c, Semigroup d, Semigroup e) =>
         Semigroup (a,b,c,d,e) where
    (a,b,c,d,e) <> (a',b',c',d',e') = (a<>a', b<>b', c<>c', d<>d', e<>e')

instance (Monoid a, Monoid b, Monoid c, Monoid d, Monoid e) =>
         Monoid (a,b,c,d,e) where
    mempty = (mempty, mempty, mempty, mempty, mempty)

-- lexicographical ordering
instance Semigroup Ordering where
    LT <> _ = LT
    EQ <> y = y
    GT <> _ = GT

instance Monoid Ordering where
    mempty = EQ

-- Nothing is the identity: the monoid that adjoins one to a semigroup.
instance Semigroup a => Semigroup (Maybe a) where
    Nothing <> b       = b
    a       <> Nothing = a
    Just a  <> Just b  = Just (a <> b)

instance Semigroup a => Monoid (Maybe a) where
    mempty = Nothing

instance Semigroup a => Semigroup (IO a) where
    (<>) = liftA2 (<>)

instance Monoid a => Monoid (IO a) where
    mempty = pure mempty

instance Monoid a => Applicative ((,) a) where
    pure x            = (mempty, x)
    (u, f) <*> (v, x) = (u <> v, f x)
    liftA2 f (u, x) (v, y) = (u <> v, f x y)

instance Monoid a => Monad ((,) a) where
    (u, a) >>= k = case k a of (v, b) -> (u <> v, b)

-- | The monoid of endomorphisms under composition, which Foldable's
-- default foldr uses; Data.Monoid exports it.
newtype Endo a = Endo { appEndo :: a -> a }

instance Semigroup (Endo a) where
    Endo f <> Endo g = Endo (f . g)

instance Monoid (Endo a) where
    mempty = Endo id

-- Foldable and traversable structures ---------------------------------------

-- As in GHC's base, the list functions of the Prelude that fold a list
-- are those of Foldable, and those that map an action over one are those
-- of Traversable.  The list instances use the list functions further
-- down, listFoldr and so on.

class Foldable t where
    fold     :: Monoid m => t m -> m
    foldMap  :: Monoid m => (a -> m) -> t a -> m
    foldMap' :: Monoid m => (a -> m) -> t a -> m
    foldr    :: (a -> b -> b) -> b -> t a -> b
    foldr'   :: (a -> b -> b) -> b -> t a -> b
    foldl    :: (b -> a -> b) -> b -> t a -> b
    foldl'   :: (b -> a -> b) -> b -> t a -> b
    foldr1   :: (a -> a -> a) -> t a -> a
    foldl1   :: (a -> a -> a) -> t a -> a
    toList   :: t a -> [a]
    null     :: t a -> Bool
    length   :: t a -> Int
    elem     :: Eq a => a -> t a -> Bool
    maximum  :: Ord a => t a -> a
    minimum  :: Ord a => t a -> a
    sum      :: Num a => t a -> a
    product  :: Num a => t a -> a

    -- Minimal complete definition: foldMap or foldr
    fold          = foldMap id
    foldMap f     = foldr (mappend . f) mempty
    foldMap' f    = foldl' (\acc a -> acc <> f a) mempty
    foldr f z t   = appEndo (foldMap (Endo . f) t) z
    foldr' f z0 t = foldl (\k x z -> k $! f x z) id t z0
    foldl f z t   = foldr (\x k acc -> k (f acc x)) id t z
    foldl' f z0 t = foldr (\x k z -> k $! f z x) id t z0
    foldr1 f t    = case foldr mf Nothing t of
                        Nothing -> error "foldr1: empty structure"
                        Just x  -> x
      where mf x m = Just (case m of
                               Nothing -> x
                               Just y  -> f x y)
    foldl1 f t    = case foldl mf Nothing t of
                        Nothing -> error "foldl1: empty structure"
                        Just x  -> x
      where mf m y = Just (case m of
                               Nothing -> y
                               Just x  -> f x y)
    toList t      = foldr (:) [] t
    null          = foldr (\_ _ -> False) True
    length        = foldl' (\c _ -> c + 1) 0
    elem x        = any (== x)
    maximum t     = case foldl' mx Nothing t of
                        Nothing -> error "maximum: empty structure"
                        Just x  -> x
      where mx Nothing  y = Just y
            mx (Just x) y = Just $! max x y
    minimum t     = case foldl' mn Nothing t of
                        Nothing -> error "minimum: empty structure"
                        Just x  -> x
      where mn Nothing  y = Just y
            mn (Just x) y = Just $! min x y
    sum           = foldl' (+) 0
    product       = foldl' (*) 1

class (Functor t, Foldable t) => Traversable t where
    traverse  :: Applicative f => (a -> f b) -> t a -> f (t b)
    sequenceA :: Applicative f => t (f a) -> f (t a)
    mapM      :: Monad m => (a -> m b) -> t a -> m (t b)
    sequence  :: Monad m => t (m a) -> m (t a)

    -- Minimal complete definition: traverse or sequenceA
    traverse f = sequenceA . fmap f
    sequenceA  = traverse id
    mapM       = traverse
    sequence   = sequenceA

concat           :: Foldable t => t [a] -> [a]
concat xs         = foldr (++) [] xs

concatMap        :: Foldable t => (a -> [b]) -> t a -> [b]
concatMap f xs    = foldr (\x b -> f x ++ b) [] xs

and, or          :: Foldable t => t Bool -> Bool
and               = foldr (&&) True
or                = foldr (||) False

any, all         :: Foldable t => (a -> Bool) -> t a -> Bool
any p             = foldr (\x b -> p x || b) False
all p             = foldr (\x b -> p x && b) True

notElem          :: (Foldable t, Eq a) => a -> t a -> Bool
notElem x         = not . elem x

mapM_            :: (Foldable t, Monad m) => (a -> m b) -> t a -> m ()
mapM_ f           = foldr ((>>) . f) (return ())

sequence_        :: (Foldable t, Monad m) => t (m a) -> m ()
sequence_         = foldr (>>) (return ())

-- The Foldable and Traversable functions that Control.Monad, Data.List,
-- Data.Foldable and Data.Traversable share, defined once here so that
-- importing two of those modules does not make them ambiguous.

forM             :: (Traversable t, Monad m) => t a -> (a -> m b) -> m (t b)
forM              = flip mapM

forM_            :: (Foldable t, Monad m) => t a -> (a -> m b) -> m ()
forM_             = flip mapM_

msum             :: (Foldable t, MonadPlus m) => t (m a) -> m a
msum              = foldr mplus mzero

find             :: Foldable t => (a -> Bool) -> t a -> Maybe a
find p            = foldr (\x r -> if p x then Just x else r) Nothing

maximumBy        :: Foldable t => (a -> a -> Ordering) -> t a -> a
maximumBy cmp     = foldl1 (\x y -> case cmp x y of GT -> x; _ -> y)

minimumBy        :: Foldable t => (a -> a -> Ordering) -> t a -> a
minimumBy cmp     = foldl1 (\x y -> case cmp x y of GT -> y; _ -> x)

-- The Traversable functions that Data.List and Data.Traversable both
-- export, with the state applicatives they use.

newtype StateL s a = StateL { runStateL :: s -> (s, a) }

instance Functor (StateL s) where
    fmap f (StateL k) = StateL $ \s -> let (s', v) = k s in (s', f v)

instance Applicative (StateL s) where
    pure x = StateL (\s -> (s, x))
    StateL kf <*> StateL kv = StateL $ \s ->
        let (s', f)  = kf s
            (s'', v) = kv s'
        in (s'', f v)

newtype StateR s a = StateR { runStateR :: s -> (s, a) }

instance Functor (StateR s) where
    fmap f (StateR k) = StateR $ \s -> let (s', v) = k s in (s', f v)

instance Applicative (StateR s) where
    pure x = StateR (\s -> (s, x))
    StateR kf <*> StateR kv = StateR $ \s ->
        let (s', v)  = kv s
            (s'', f) = kf s'
        in (s'', f v)

mapAccumL        :: Traversable t => (s -> a -> (s, b)) -> s -> t a -> (s, t b)
mapAccumL f s t   = runStateL (traverse (StateL . flip f) t) s

mapAccumR        :: Traversable t => (s -> a -> (s, b)) -> s -> t a -> (s, t b)
mapAccumR f s t   = runStateR (traverse (StateR . flip f) t) s

instance Foldable [] where
    foldr    = listFoldr
    foldl    = listFoldl
    foldl'   = listFoldl'
    foldr1   = listFoldr1
    foldl1   = listFoldl1
    toList   = id
    null     = listNull
    length   = listLength
    elem     = listElem
    maximum  = listMaximum
    minimum  = listMinimum
    sum      = listSum
    product  = listProduct

instance Traversable [] where
    traverse f = listFoldr (\x ys -> liftA2 (:) (f x) ys) (pure [])

instance Foldable Maybe where
    foldMap _ Nothing  = mempty
    foldMap f (Just x) = f x
    foldr _ z Nothing  = z
    foldr f z (Just x) = f x z
    foldl _ z Nothing  = z
    foldl f z (Just x) = f z x
    toList Nothing     = []
    toList (Just x)    = [x]
    null Nothing       = True
    null (Just _)      = False
    length Nothing     = 0
    length (Just _)    = 1

instance Traversable Maybe where
    traverse _ Nothing  = pure Nothing
    traverse f (Just x) = fmap Just (f x)

instance Foldable (Either a) where
    foldMap _ (Left _)  = mempty
    foldMap f (Right y) = f y
    foldr _ z (Left _)  = z
    foldr f z (Right y) = f y z
    null (Left _)       = True
    null (Right _)      = False
    length (Left _)     = 0
    length (Right _)    = 1

instance Traversable (Either a) where
    traverse _ (Left x)  = pure (Left x)
    traverse f (Right y) = fmap Right (f y)

instance Foldable ((,) a) where
    foldMap f (_, y) = f y
    foldr f z (_, y) = f y z
    null _           = False
    length _         = 1

instance Traversable ((,) a) where
    traverse f (x, y) = fmap ((,) x) (f y)

instance Foldable NonEmpty where
    foldr f z (a :| as) = f a (listFoldr f z as)
    foldl f z (a :| as) = listFoldl f (f z a) as
    foldl1 f (a :| as)  = listFoldl f a as
    toList (a :| as)    = a : as
    null _              = False
    length (_ :| as)    = 1 + listLength as

instance Traversable NonEmpty where
    traverse f (a :| as) = liftA2 (:|) (f a) (traverse f as)

-- Evaluation and strictness ------------------------------------------------

primitive seq           :: a -> b -> b

primitive ($!) "strict" :: (a -> b) -> a -> b
-- f $! x                = x `seq` f x

-- Trivial type -------------------------------------------------------------

-- data () = () deriving (Eq, Ord, Ix, Enum, Read, Show, Bounded)

instance Eq () where
    () == ()  =  True

instance Ord () where
    compare () () = EQ

instance Ix () where
    range ((),())      = [()]
    index ((),()) ()   = 0
    inRange ((),()) () = True

instance Enum () where
    toEnum 0           = ()
    fromEnum ()        = 0
    enumFrom ()        = [()]

instance Read () where
    readsPrec p = readParen False (\r -> [((),t) | ("(",s) <- lex r,
						   (")",t) <- lex s ])

instance Show () where
    showsPrec p () = showString "()"

instance Bounded () where
    minBound = ()
    maxBound = ()

-- Boolean type -------------------------------------------------------------

data Bool    = False | True
	       deriving (Eq, Ord, Ix, Enum, Read, Show, Bounded)

(&&), (||)  :: Bool -> Bool -> Bool
False && x   = False
True  && x   = x
False || x   = x
True  || x   = True

not         :: Bool -> Bool
not True     = False
not False    = True

otherwise   :: Bool
otherwise    = True

-- Character type -----------------------------------------------------------

data Char               -- builtin datatype of ISO Latin characters
type String = [Char]    -- strings are lists of characters

primitive primEqChar    :: Char -> Char -> Bool
primitive primCmpChar   :: Char -> Char -> Ordering

instance Eq Char  where (==)    = primEqChar
instance Ord Char where compare = primCmpChar

primitive primCharToInt :: Char -> Int
primitive primIntToChar :: Int -> Char

instance Enum Char where
    toEnum           = primIntToChar
    fromEnum         = primCharToInt
    enumFrom c       = map toEnum [fromEnum c .. fromEnum (maxBound::Char)]
    enumFromThen     = boundedEnumFromThen

instance Ix Char where
    range (c,c')      = [c..c']
    unsafeIndex (c,_) i = fromEnum i - fromEnum c
    inRange (c,c') i  = c <= i && i <= c'

instance Read Char where
    readsPrec p      = readParen False
			    (\r -> [(c,t) | ('\'':s,t) <- lex r,
					    (c,"\'")   <- readLitChar s ])
    readList = readParen False (\r -> [(l,t) | ('"':s, t) <- lex r,
					       (l,_)      <- readl s ])
	       where readl ('"':s)      = [("",s)]
		     readl ('\\':'&':s) = readl s
		     readl s            = [(c:cs,u) | (c ,t) <- readLitChar s,
						      (cs,u) <- readl t ]
instance Show Char where
    showsPrec p '\'' = showString "'\\''"
    showsPrec p c    = showChar '\'' . showLitChar c . showChar '\''

    showList cs   = showChar '"' . showl cs
		    where showl ""       = showChar '"'
			  showl ('"':cs) = showString "\\\"" . showl cs
			  showl (c:cs)   = showLitChar c . showl cs

instance Bounded Char where
    minBound = '\0'
    maxBound = primMaxChar

primitive primMaxChar :: Char

isSpace, isDigit :: Char -> Bool

isSpace c              =  c == ' '  ||
			  c == '\t' ||
			  c == '\n' ||
			  c == '\r' ||
			  c == '\f' ||
			  c == '\v' ||
			  c == '\xa0'

isDigit c              =  c >= '0'   &&  c <= '9'

primitive isUpper      :: Char -> Bool
primitive isLower      :: Char -> Bool
primitive isAlpha      :: Char -> Bool
primitive isAlphaNum   :: Char -> Bool

-- Maybe type ---------------------------------------------------------------

data Maybe a = Nothing | Just a
	       deriving (Eq, Ord, Read, Show)

maybe             :: b -> (a -> b) -> Maybe a -> b
maybe n f Nothing  = n
maybe n f (Just x) = f x

instance Functor Maybe where
    fmap f Nothing  = Nothing
    fmap f (Just x) = Just (f x)

instance Applicative Maybe where
    pure               = Just
    Just f  <*> m      = fmap f m
    Nothing <*> _      = Nothing
    liftA2 f (Just x) (Just y) = Just (f x y)
    liftA2 _ _        _        = Nothing
    Just _  *> m       = m
    Nothing *> _       = Nothing

instance Monad Maybe where
    Just x  >>= k = k x
    Nothing >>= k = Nothing
    (>>)          = (*>)

instance MonadFail Maybe where
    fail s        = Nothing

instance Alternative Maybe where
    empty         = Nothing
    Nothing <|> r = r
    l       <|> _ = l

instance MonadPlus Maybe

-- Either type --------------------------------------------------------------

data Either a b = Left a | Right b
		  deriving (Eq, Ord, Read, Show)

either              :: (a -> c) -> (b -> c) -> Either a b -> c
either l r (Left x)  = l x
either l r (Right y) = r y

instance Functor (Either e) where
    fmap _ (Left x)  = Left x
    fmap f (Right y) = Right (f y)

instance Applicative (Either e) where
    pure          = Right
    Left e  <*> _ = Left e
    Right f <*> r = fmap f r

instance Monad (Either e) where
    Left e  >>= _ = Left e
    Right x >>= k = k x

-- Functions and pairs ------------------------------------------------------

instance Functor ((->) r) where
    fmap = (.)

instance Applicative ((->) r) where
    pure           = const
    (<*>) f g x    = f x (g x)
    liftA2 q f g x = q (f x) (g x)

instance Monad ((->) r) where
    f >>= k = \ r -> k (f r) r

instance Functor ((,) a) where
    fmap f (x,y) = (x, f y)

-- Ordering type ------------------------------------------------------------

data Ordering = LT | EQ | GT
		deriving (Eq, Ord, Ix, Enum, Read, Show, Bounded)

-- Lists --------------------------------------------------------------------

-- data [a] = [] | a : [a] deriving (Eq, Ord)

instance Eq a => Eq [a] where
    []     == []     =  True
    (x:xs) == (y:ys) =  x==y && xs==ys
    _      == _      =  False

instance Ord a => Ord [a] where
    compare []     (_:_)  = LT
    compare []     []     = EQ
    compare (_:_)  []     = GT
    compare (x:xs) (y:ys) = primCompAux x y (compare xs ys)

instance Functor [] where
    fmap = map

instance Applicative [] where
    pure x        = [x]
    fs <*> xs     = [ f x | f <- fs, x <- xs ]
    liftA2 f xs ys = [ f x y | x <- xs, y <- ys ]
    xs *> ys      = [ y | _ <- xs, y <- ys ]

instance Monad [ ] where
    (x:xs) >>= f = f x ++ (xs >>= f)
    []     >>= f = []

instance MonadFail [] where
    fail s       = []

instance Alternative [] where
    empty = []
    (<|>) = (++)

instance MonadPlus []

instance Read a => Read [a]  where
    readsPrec p = readList

instance Show a => Show [a]  where
    showsPrec p = showList

-- Tuples -------------------------------------------------------------------

-- data (a,b) = (a,b) deriving (Eq, Ord, Ix, Read, Show)
-- etc..

-- Standard Integral types --------------------------------------------------

data Int      -- builtin datatype of fixed size integers
data Integer  -- builtin datatype of arbitrary size integers

primitive primEqInt      :: Int -> Int -> Bool
primitive primCmpInt     :: Int -> Int -> Ordering
primitive primEqInteger  :: Integer -> Integer -> Bool
primitive primCmpInteger :: Integer -> Integer -> Ordering

instance Eq  Int     where (==)    = primEqInt
instance Eq  Integer where (==)    = primEqInteger
instance Ord Int     where compare = primCmpInt
instance Ord Integer where compare = primCmpInteger

primitive primPlusInt,
	  primMinusInt,
	  primMulInt	   :: Int -> Int -> Int
primitive primNegInt	   :: Int -> Int
primitive primIntegerToInt :: Integer -> Int

instance Num Int where
    (+)           = primPlusInt
    (-)           = primMinusInt
    negate        = primNegInt
    (*)           = primMulInt
    abs           = absReal
    signum        = signumReal
    fromInteger   = primIntegerToInt
    fromInt x     = x

primitive primMinInt, primMaxInt :: Int

instance Bounded Int where
    minBound = primMinInt
    maxBound = primMaxInt

primitive primPlusInteger,
	  primMinusInteger,
	  primMulInteger   :: Integer -> Integer -> Integer
primitive primNegInteger   :: Integer -> Integer
primitive primIntToInteger :: Int -> Integer

instance Num Integer where
    (+)           = primPlusInteger
    (-)           = primMinusInteger
    negate        = primNegInteger
    (*)           = primMulInteger
    abs           = absReal
    signum        = signumReal
    fromInteger x = x
    fromInt       = primIntToInteger

absReal x    | x >= 0    = x
	     | otherwise = -x

signumReal x | x == 0    =  0
	     | x > 0     =  1
	     | otherwise = -1

instance Real Int where
    toRational x = toInteger x % 1

instance Real Integer where
    toRational x = x % 1

primitive primDivInt,
	  primQuotInt,
	  primRemInt,
	  primModInt  :: Int -> Int -> Int
primitive primQrmInt  :: Int -> Int -> (Int,Int)

instance Integral Int where
    div       = primDivInt
    quot      = primQuotInt
    rem       = primRemInt
    mod       = primModInt
    quotRem   = primQrmInt
    toInteger = primIntToInteger
    toInt x   = x

primitive primQrmInteger  :: Integer -> Integer -> (Integer,Integer)

instance Integral Integer where
    quotRem     = primQrmInteger
    toInteger x = x
    toInt       = primIntegerToInt

instance Ix Int where
    range (m,n)          = [m..n]
    unsafeIndex (m,_) i  = i - m
    inRange (m,n) i      = m <= i && i <= n

instance Ix Integer where
    range (m,n)          = [m..n]
    unsafeIndex (m,_) i  = toInt (i - m)
    inRange (m,n) i      = m <= i && i <= n

instance Enum Int where
    succ           = boundedSucc
    pred           = boundedPred
    toEnum         = id
    fromEnum       = id
    enumFrom       = boundedEnumFrom
    enumFromTo     = boundedEnumFromTo
    enumFromThen   = boundedEnumFromThen
    enumFromThenTo = boundedEnumFromThenTo

boundedSucc, boundedPred :: (Num a, Bounded a, Enum a) => a -> a
boundedSucc x
  | x == maxBound = error "succ: applied to maxBound"
  | otherwise     = x+1
boundedPred x
  | x == minBound = error "pred: applied to minBound"
  | otherwise     = x-1

boundedEnumFrom       :: (Ord a, Bounded a, Enum a) => a -> [a]
boundedEnumFromTo     :: (Ord a, Bounded a, Enum a) => a -> a -> [a]
boundedEnumFromThenTo :: (Ord a, Num a, Bounded a, Enum a) => a -> a -> a -> [a]
boundedEnumFromThen   :: (Ord a, Bounded a, Enum a) => a -> a -> [a]

boundedEnumFrom n     = takeWhile1 (/= maxBound) (iterate succ n)
boundedEnumFromTo n m = takeWhile (<= m) (boundedEnumFrom n)
boundedEnumFromThen n m =
    enumFromThenTo n m (if n <= m then maxBound else minBound)
boundedEnumFromThenTo n n' m
  | n' >= n   = if n <= m then takeWhile1 (<= m - delta) ns else []
  | otherwise = if n >= m then takeWhile1 (>= m - delta) ns else []
 where
  delta = n'-n
  ns = iterate (+delta) n

-- takeWhile and one more
takeWhile1 :: (a -> Bool) -> [a] -> [a]
takeWhile1 p (x:xs) = x : if p x then takeWhile1 p xs else []

instance Enum Integer where
    succ x         = x + 1
    pred x         = x - 1

    toEnum         = primIntToInteger
    fromEnum       = primIntegerToInt
    enumFrom       = numericEnumFrom
    enumFromThen   = numericEnumFromThen
    enumFromTo n m = takeWhile (<= m) (numericEnumFrom n)
    enumFromThenTo n n' m = takeWhile p (numericEnumFromThen n n')
                                where p | n' >= n   = (<= m)
                                        | otherwise = (>= m)

numericEnumFrom        :: Num a => a -> [a]
numericEnumFromThen    :: Num a => a -> a -> [a]
numericEnumFromTo      :: (Ord a, Fractional a) => a -> a -> [a]
numericEnumFromThenTo  :: (Ord a, Fractional a) => a -> a -> a -> [a]
numericEnumFrom n            = iterate' (+1) n
numericEnumFromThen n m      = iterate' (+(m-n)) n
numericEnumFromTo n m        = takeWhile (<= m+1/2) (numericEnumFrom n)
numericEnumFromThenTo n n' m = takeWhile p (numericEnumFromThen n n')
                               where p | n' >= n   = (<= m + (n'-n)/2)
                                       | otherwise = (>= m + (n'-n)/2)

iterate' :: (a -> a) -> a -> [a]	-- strict version of iterate
iterate' f x = x : (iterate' f $! f x)

primitive primShowsInt :: Int -> Int -> ShowS

instance Read Int where
    readsPrec p = readSigned readDec

instance Show Int where
    showsPrec   = primShowsInt

primitive primShowsInteger :: Int -> Integer -> ShowS

instance Read Integer where
    readsPrec p = readSigned readDec

instance Show Integer where
    showsPrec   = primShowsInteger

-- Standard Floating types --------------------------------------------------

data Float     -- builtin datatype of single precision floating point numbers
data Double    -- builtin datatype of double precision floating point numbers

primitive primEqFloat   :: Float -> Float -> Bool
primitive primCmpFloat  :: Float -> Float -> Ordering
primitive primEqDouble  :: Double -> Double -> Bool
primitive primCmpDouble :: Double -> Double -> Ordering

instance Eq  Float  where (==) = primEqFloat
instance Eq  Double where (==) = primEqDouble

instance Ord Float  where compare = primCmpFloat
instance Ord Double where compare = primCmpDouble

primitive primPlusFloat,
	  primMinusFloat,
	  primMulFloat       :: Float -> Float -> Float
primitive primNegFloat       :: Float -> Float
primitive primIntToFloat     :: Int -> Float
primitive primIntegerToFloat :: Integer -> Float

instance Num Float where
    (+)           = primPlusFloat
    (-)           = primMinusFloat
    negate        = primNegFloat
    (*)           = primMulFloat
    abs           = absReal
    signum        = signumReal
    fromInteger   = primIntegerToFloat
    fromInt       = primIntToFloat

primitive primPlusDouble,
	  primMinusDouble,
	  primMulDouble       :: Double -> Double -> Double
primitive primNegDouble       :: Double -> Double
primitive primIntToDouble     :: Int -> Double
primitive primIntegerToDouble :: Integer -> Double

instance Num Double where
    (+)         = primPlusDouble
    (-)         = primMinusDouble
    negate      = primNegDouble
    (*)         = primMulDouble
    abs         = absReal
    signum      = signumReal
    fromInteger = primIntegerToDouble
    fromInt     = primIntToDouble

instance Real Float where
    toRational = floatToRational

instance Real Double where
    toRational = doubleToRational

-- Calls to these functions are optimised when passed as arguments to
-- fromRational.
floatToRational  :: Float  -> Rational
doubleToRational :: Double -> Rational
floatToRational  x = realFloatToRational x 
doubleToRational x = realFloatToRational x

realFloatToRational x = (m%1)*(b%1)^^n
			where (m,n) = decodeFloat x
			      b     = floatRadix x

primitive primDivFloat      :: Float -> Float -> Float
primitive primDoubleToFloat :: Double -> Float
primitive primFloatToDouble :: Float -> Double	-- used by runtime optimizer

instance Fractional Float where
    (/)          = primDivFloat
    fromRational = primRationalToFloat
    fromDouble   = primDoubleToFloat

primitive primDivDouble :: Double -> Double -> Double

instance Fractional Double where
    (/)          = primDivDouble
    fromRational = primRationalToDouble
    fromDouble x = x

-- These primitives are equivalent to (and are defined using) 
-- rationalTo{Float,Double}.  The difference is that they test to see
-- if their argument is of the form (fromDouble x) - which allows a much
-- more efficient implementation.
primitive primRationalToFloat  :: Rational -> Float
primitive primRationalToDouble :: Rational -> Double

-- These functions are used by Cupid - don't change their types.
rationalToFloat  :: Rational -> Float
rationalToDouble :: Rational -> Double
rationalToFloat  = rationalToRealFloat
rationalToDouble = rationalToRealFloat

rationalToRealFloat x = x'
 where x'    = f e
       f e   = if e' == e then y else f e'
	       where y      = encodeFloat (round (x * (1%b)^^e)) e
		     (_,e') = decodeFloat y
       (_,e) = decodeFloat (fromInteger (numerator x) `asTypeOf` x'
			     / fromInteger (denominator x))
       b     = floatRadix x'

primitive primSinFloat,  primAsinFloat, primCosFloat,
	  primAcosFloat, primTanFloat,  primAtanFloat,
	  primLogFloat,  primExpFloat,  primSqrtFloat :: Float -> Float

instance Floating Float where
    exp   = primExpFloat
    log   = primLogFloat
    sqrt  = primSqrtFloat
    sin   = primSinFloat
    cos   = primCosFloat
    tan   = primTanFloat
    asin  = primAsinFloat
    acos  = primAcosFloat
    atan  = primAtanFloat

primitive primSinDouble,  primAsinDouble, primCosDouble,
	  primAcosDouble, primTanDouble,  primAtanDouble,
	  primLogDouble,  primExpDouble,  primSqrtDouble :: Double -> Double

instance Floating Double where
    exp   = primExpDouble
    log   = primLogDouble
    sqrt  = primSqrtDouble
    sin   = primSinDouble
    cos   = primCosDouble
    tan   = primTanDouble
    asin  = primAsinDouble
    acos  = primAcosDouble
    atan  = primAtanDouble

instance RealFrac Float where
    properFraction = floatProperFraction

instance RealFrac Double where
    properFraction = floatProperFraction

floatProperFraction x
   | n >= 0      = (fromInteger m * fromInteger b ^ n, 0)
   | otherwise   = (fromInteger w, encodeFloat r n)
		   where (m,n) = decodeFloat x
			 b     = floatRadix x
			 (w,r) = quotRem m (b^(-n))

primitive primFloatRadix  :: Integer
primitive primFloatDigits :: Int
primitive primFloatMinExp,
          primFloatMaxExp :: Int
primitive primFloatEncode :: Integer -> Int -> Float
primitive primFloatDecode :: Float -> (Integer, Int)

instance RealFloat Float where
    floatRadix  _ = primFloatRadix
    floatDigits _ = primFloatDigits
    floatRange  _ = (primFloatMinExp, primFloatMaxExp)
    encodeFloat = primFloatEncode
    decodeFloat = primFloatDecode
    isNaN       _ = False
    isInfinite  _ = False
    isDenormalized _ = False
    isNegativeZero _ = False
    isIEEE      _ = False

primitive primDoubleRadix  :: Integer
primitive primDoubleDigits :: Int
primitive primDoubleMinExp,
          primDoubleMaxExp :: Int
primitive primDoubleEncode :: Integer -> Int -> Double
primitive primDoubleDecode :: Double -> (Integer, Int)

instance RealFloat Double where
    floatRadix  _ = primDoubleRadix
    floatDigits _ = primDoubleDigits
    floatRange  _ = (primDoubleMinExp, primDoubleMaxExp)
    encodeFloat   = primDoubleEncode
    decodeFloat   = primDoubleDecode
    isNaN       _ = False
    isInfinite  _ = False
    isDenormalized _ = False
    isNegativeZero _ = False
    isIEEE      _ = False

instance Enum Float where
    succ x                = x+1
    pred x                = x-1
    toEnum		  = primIntToFloat
    fromEnum		  = fromInteger . truncate   -- may overflow
    enumFrom		  = numericEnumFrom
    enumFromThen	  = numericEnumFromThen
    enumFromTo		  = numericEnumFromTo
    enumFromThenTo	  = numericEnumFromThenTo

instance Enum Double where
    succ x                = x+1
    pred x                = x-1
    toEnum		  = primIntToDouble
    fromEnum		  = fromInteger . truncate   -- may overflow
    enumFrom		  = numericEnumFrom
    enumFromThen	  = numericEnumFromThen
    enumFromTo		  = numericEnumFromTo
    enumFromThenTo	  = numericEnumFromThenTo

primitive primShowsFloat :: Int -> Float -> ShowS

instance Read Float where
    readsPrec p = readSigned readFloat

-- Note that showFloat in Numeric isn't used here
instance Show Float where
    showsPrec   = primShowsFloat

primitive primShowsDouble :: Int -> Double -> ShowS

instance Read Double where
    readsPrec p = readSigned readFloat

-- Note that showFloat in Numeric isn't used here
instance Show Double where
    showsPrec   = primShowsDouble

-- Some standard functions --------------------------------------------------

fst            :: (a,b) -> a
fst (x,_)       = x

snd            :: (a,b) -> b
snd (_,y)       = y

curry          :: ((a,b) -> c) -> (a -> b -> c)
curry f x y     = f (x,y)

uncurry        :: (a -> b -> c) -> ((a,b) -> c)
uncurry f p     = f (fst p) (snd p)

id             :: a -> a
id    x         = x

const          :: a -> b -> a
const k _       = k

(.)            :: (b -> c) -> (a -> b) -> (a -> c)
(f . g) x       = f (g x)

flip           :: (a -> b -> c) -> b -> a -> c
flip f x y      = f y x

($)            :: (a -> b) -> a -> b
f $ x           = f x

until          :: (a -> Bool) -> (a -> a) -> a -> a
until p f x     = if p x then x else until p f (f x)

asTypeOf       :: a -> a -> a
asTypeOf        = const

error          :: String -> a
error s         = throw (ErrorCall s)

undefined      :: a
undefined       = error "Prelude.undefined"

-- Standard functions on rational numbers {PreludeRatio} --------------------

data Integral a => Ratio a = !a :% !a deriving (Eq)
type Rational              = Ratio Integer

(%)                       :: Integral a => a -> a -> Ratio a
x % y                      = reduce (x * signum y) (abs y)

reduce                    :: Integral a => a -> a -> Ratio a
reduce x y | y == 0        = error "Ratio.%: zero denominator"
	   | otherwise     = (x `quot` d) :% (y `quot` d)
			     where d = gcd x y

numerator, denominator    :: Integral a => Ratio a -> a
numerator (x :% y)         = x
denominator (x :% y)       = y

instance Integral a => Ord (Ratio a) where
    compare (x:%y) (x':%y') = compare (x*y') (x'*y)

instance Integral a => Num (Ratio a) where
    (x:%y) + (x':%y') = reduce (x*y' + x'*y) (y*y')
    (x:%y) * (x':%y') = reduce (x*x') (y*y')
    negate (x :% y)   = negate x :% y
    abs (x :% y)      = abs x :% y
    signum (x :% y)   = signum x :% 1
    fromInteger x     = fromInteger x :% 1
    fromInt           = intToRatio

-- Cupid optimises code of the form fromRational (intToRatio x)
intToRatio :: Integral a => Int -> Ratio a
intToRatio x = fromInt x :% 1

instance Integral a => Real (Ratio a) where
    toRational (x:%y) = toInteger x :% toInteger y

instance Integral a => Fractional (Ratio a) where
    (x:%y) / (x':%y')   = (x*y') % (y*x')
    recip (x:%y)        = y % x
    fromRational (x:%y) = fromInteger x :% fromInteger y
    fromDouble 		= doubleToRatio

-- Cupid optimises code of the form fromRational (doubleToRatio x)
-- Since this function is private, and only used to convert floating point
-- literals, it yields a decimal fraction, hopefully the one the user
-- specified in the first place (but some precision may be lost).  A real
-- Haskell implementation would use Rational to represent these literals.
doubleToRatio :: Integral a => Double -> Ratio a
doubleToRatio x
	    | n>=0      = (round (x / fromInteger pow) * fromInteger pow) % 1
	    | otherwise = fromRational (round (x * fromInteger denom) % denom)
			  where (m,n) = decodeFloat x
				n_dec = ceiling (logBase 10 (encodeFloat 1 n))
				denom = 10 ^ (-n_dec)
				pow   = 10 ^ n_dec

instance Integral a => RealFrac (Ratio a) where
    properFraction (x:%y) = (fromIntegral q, r:%y)
			    where (q,r) = quotRem x y

instance Integral a => Enum (Ratio a) where
    succ x         = x+1
    pred x         = x-1
    toEnum         = fromInt
    fromEnum       = fromInteger . truncate   -- may overflow
    enumFrom       = numericEnumFrom
    enumFromTo     = numericEnumFromTo
    enumFromThen   = numericEnumFromThen
    enumFromThenTo = numericEnumFromThenTo

instance (Read a, Integral a) => Read (Ratio a) where
    readsPrec p = readParen (p > 7)
			    (\r -> [(x%y,u) | (x,s)   <- readsPrec 8 r,
					      ("%",t) <- lex s,
					      (y,u)   <- readsPrec 8 t ])

instance Integral a => Show (Ratio a) where
    showsPrec p (x:%y) = showParen (p > 7)
			     (showsPrec 8 x . showString " % " . showsPrec 8 y)

-- Standard list functions {PreludeList} ------------------------------------

head             :: [a] -> a
head (x:_)        = x

last             :: [a] -> a
last [x]          = x
last (_:xs)       = last xs

tail             :: [a] -> [a]
tail (_:xs)       = xs

init             :: [a] -> [a]
init [x]          = []
init (x:xs)       = x : init xs

listNull         :: [a] -> Bool
listNull []       = True
listNull (_:_)    = False

(++)             :: [a] -> [a] -> [a]
[]     ++ ys      = ys
(x:xs) ++ ys      = x : (xs ++ ys)

map              :: (a -> b) -> [a] -> [b]
map f xs          = [ f x | x <- xs ]

filter           :: (a -> Bool) -> [a] -> [a]
filter p xs       = [ x | x <- xs, p x ]

listLength       :: [a] -> Int
listLength        = listFoldl' (\n _ -> n + 1) 0

(!!)             :: [a] -> Int -> a
xs     !! n | n<0 = error "Prelude.!!: negative index"
[]     !! _       = error "Prelude.!!: index too large"
(x:_)  !! 0       = x
(_:xs) !! n       = xs !! (n-1)

listFoldl        :: (a -> b -> a) -> a -> [b] -> a
listFoldl f z []      = z
listFoldl f z (x:xs)  = listFoldl f (f z x) xs

listFoldl'       :: (a -> b -> a) -> a -> [b] -> a
listFoldl' f a []     = a
listFoldl' f a (x:xs) = (listFoldl' f $! f a x) xs

listFoldl1       :: (a -> a -> a) -> [a] -> a
listFoldl1 f (x:xs)   = listFoldl f x xs
listFoldl1 _ []       = error "Prelude.foldl1: empty list"

scanl            :: (a -> b -> a) -> a -> [b] -> [a]
scanl f q xs      = q : (case xs of
			 []   -> []
			 x:xs -> scanl f (f q x) xs)

scanl1           :: (a -> a -> a) -> [a] -> [a]
scanl1 _ []       = []
scanl1 f (x:xs)   = scanl f x xs

listFoldr        :: (a -> b -> b) -> b -> [a] -> b
listFoldr f z []      = z
listFoldr f z (x:xs)  = f x (listFoldr f z xs)

listFoldr1       :: (a -> a -> a) -> [a] -> a
listFoldr1 f [x]      = x
listFoldr1 f (x:xs)   = f x (listFoldr1 f xs)
listFoldr1 _ []       = error "Prelude.foldr1: empty list"

scanr            :: (a -> b -> b) -> b -> [a] -> [b]
scanr f q0 []     = [q0]
scanr f q0 (x:xs) = f x q : qs
		    where qs@(q:_) = scanr f q0 xs

scanr1           :: (a -> a -> a) -> [a] -> [a]
scanr1 f []       = []
scanr1 f [x]      = [x]
scanr1 f (x:xs)   = f x q : qs
		    where qs@(q:_) = scanr1 f xs

iterate          :: (a -> a) -> a -> [a]
iterate f x       = x : iterate f (f x)

repeat           :: a -> [a]
repeat x          = xs where xs = x:xs

replicate        :: Int -> a -> [a]
replicate n x     = take n (repeat x)

cycle            :: [a] -> [a]
cycle []          = error "Prelude.cycle: empty list"
cycle xs          = xs' where xs'=xs++xs'

take                :: Int -> [a] -> [a]
take n _  | n <= 0  = []
take _ []           = []
take n (x:xs)       = x : take (n-1) xs

drop                :: Int -> [a] -> [a]
drop n xs | n <= 0  = xs
drop _ []           = []
drop n (_:xs)       = drop (n-1) xs

splitAt               :: Int -> [a] -> ([a], [a])
splitAt n xs | n <= 0 = ([],xs)
splitAt _ []          = ([],[])
splitAt n (x:xs)      = (x:xs',xs'') where (xs',xs'') = splitAt (n-1) xs

takeWhile           :: (a -> Bool) -> [a] -> [a]
takeWhile p []       = []
takeWhile p (x:xs)
	 | p x       = x : takeWhile p xs
	 | otherwise = []

dropWhile           :: (a -> Bool) -> [a] -> [a]
dropWhile p []       = []
dropWhile p xs@(x:xs')
	 | p x       = dropWhile p xs'
	 | otherwise = xs

span, break         :: (a -> Bool) -> [a] -> ([a],[a])
span p []            = ([],[])
span p xs@(x:xs')
	 | p x       = (x:ys, zs)
	 | otherwise = ([],xs)
                       where (ys,zs) = span p xs'
break p              = span (not . p)

lines     :: String -> [String]
lines ""   = []
lines s    = let (l,s') = break ('\n'==) s
             in l : case s' of []      -> []
                               (_:s'') -> lines s''

words     :: String -> [String]
words s    = case dropWhile isSpace s of
		  "" -> []
		  s' -> w : words s''
			where (w,s'') = break isSpace s'

unlines   :: [String] -> String
unlines []      = []
unlines (l:ls)  = l ++ '\n' : unlines ls

unwords   :: [String] -> String
unwords []	=  ""
unwords [w]	= w
unwords (w:ws)	= w ++ ' ' : unwords ws

reverse   :: [a] -> [a]
reverse    = foldl (flip (:)) []

listElem         :: Eq a => a -> [a] -> Bool
listElem x []     = False
listElem x (y:ys) = x == y || listElem x ys

lookup           :: Eq a => a -> [(a,b)] -> Maybe b
lookup k []       = Nothing
lookup k ((x,y):xys)
      | k==x      = Just y
      | otherwise = lookup k xys

listSum, listProduct :: Num a => [a] -> a
listSum           = listFoldl' (+) 0
listProduct       = listFoldl' (*) 1

listMaximum, listMinimum :: Ord a => [a] -> a
listMaximum []    = error "Prelude.maximum: empty list"
listMaximum xs    = listFoldl1 max xs
listMinimum []    = error "Prelude.minimum: empty list"
listMinimum xs    = listFoldl1 min xs

zip              :: [a] -> [b] -> [(a,b)]
zip               = zipWith  (\a b -> (a,b))

zip3             :: [a] -> [b] -> [c] -> [(a,b,c)]
zip3              = zipWith3 (\a b c -> (a,b,c))

zipWith                  :: (a->b->c) -> [a]->[b]->[c]
zipWith z (a:as) (b:bs)   = z a b : zipWith z as bs
zipWith _ _      _        = []

zipWith3                 :: (a->b->c->d) -> [a]->[b]->[c]->[d]
zipWith3 z (a:as) (b:bs) (c:cs)
			  = z a b c : zipWith3 z as bs cs
zipWith3 _ _ _ _          = []

unzip                    :: [(a,b)] -> ([a],[b])
unzip                     = foldr (\(a,b) ~(as,bs) -> (a:as, b:bs)) ([], [])

unzip3                   :: [(a,b,c)] -> ([a],[b],[c])
unzip3                    = foldr (\(a,b,c) ~(as,bs,cs) -> (a:as,b:bs,c:cs))
				  ([],[],[])

-- PreludeText ----------------------------------------------------------------

reads        :: Read a => ReadS a
reads         = readsPrec 0

shows        :: Show a => a -> ShowS
shows         = showsPrec 0

read         :: Read a => String -> a
read s        =  case [x | (x,t) <- reads s, ("","") <- lex t] of
		      [x] -> x
		      []  -> error "Prelude.read: no parse"
		      _   -> error "Prelude.read: ambiguous parse"

showChar     :: Char -> ShowS
showChar      = (:)

showString   :: String -> ShowS
showString    = (++)

showParen    :: Bool -> ShowS -> ShowS
showParen b p = if b then showChar '(' . p . showChar ')' else p

showField    :: Show a => String -> a -> ShowS
showField m@(c:_) v
  | isAlpha c || c == '_' = showString m . showString " = " . shows v
  | otherwise = showChar '(' . showString m . showString ") = " . shows v

readParen    :: Bool -> ReadS a -> ReadS a
readParen b g = if b then mandatory else optional
		where optional r  = g r ++ mandatory r
		      mandatory r = [(x,u) | ("(",s) <- lex r,
					     (x,t)   <- optional s,
					     (")",u) <- lex t    ]

readField    :: Read a => String -> ReadS a
readField m s0 = [ r | (t,  s1) <- readFieldName m s0,
                       ("=",s2) <- lex s1,
                       r        <- reads s2 ]

readFieldName :: String -> ReadS String
readFieldName m@(c:_) s0
  | isAlpha c || c == '_' = [ (f,s1) | (f,s1) <- lex s0, f == m ]
  | otherwise = [ (f,s3) | ("(",s1) <- lex s0,
			   (f,s2)   <- lex s1, f == m,
			   (")",s3) <- lex s2 ]

lex                    :: ReadS String
lex ""                  = [("","")]
lex (c:s) | isSpace c   = lex (dropWhile isSpace s)
lex ('\'':s)            = [('\'':ch++"'", t) | (ch,'\'':t)  <- lexLitChar s,
					       ch /= "'"                ]
lex ('"':s)             = [('"':str, t)      | (str,t) <- lexString s]
			  where
			  lexString ('"':s) = [("\"",s)]
			  lexString s = [(ch++str, u)
						| (ch,t)  <- lexStrItem s,
						  (str,u) <- lexString t  ]

			  lexStrItem ('\\':'&':s) = [("\\&",s)]
			  lexStrItem ('\\':c:s) | isSpace c
			      = [("",t) | '\\':t <- [dropWhile isSpace s]]
			  lexStrItem s            = lexLitChar s

lex (c:s) | isSym c     = [(c:sym,t)         | (sym,t) <- [span isSym s]]
          | isAlpha c   = [(c:nam,t)         | (nam,t) <- [span isIdChar s]]
             -- '_' can be the start of a single char or a name/id.
          | c == '_'    = case span isIdChar s of 
                            ([],_) -> [([c],s)]
                            (nm,t) -> [((c:nm),t)]
          | isSingle c  = [([c],s)]
          | isDigit c   = [(c:ds++fe,t)      | (ds,s)  <- [span isDigit s],
                                               (fe,t)  <- lexFracExp s     ]
          | otherwise   = []    -- bad character
		where
		isSingle c  =  c `elem` ",;()[]{}_`"
		isSym c     =  c `elem` "!@#$%&*+./<=>?\\^|:-~"
		isIdChar c  =  isAlphaNum c || c `elem` "_'"

		lexFracExp ('.':c:cs) | isDigit c 
                            = [('.':ds++e,u) | (ds,t) <- lexDigits (c:cs),
					       (e,u)  <- lexExp t    ]
		lexFracExp s       = lexExp s

		lexExp (e:s) | e `elem` "eE"
			 = [(e:c:ds,u) | (c:t)  <- [s], c `elem` "+-",
						   (ds,u) <- lexDigits t] ++
			   [(e:ds,t)   | (ds,t) <- lexDigits s]
		lexExp s = [("",s)]

lexDigits               :: ReadS String
lexDigits               =  nonnull isDigit

nonnull                 :: (Char -> Bool) -> ReadS String
nonnull p s             =  [(cs,t) | (cs@(_:_),t) <- [span p s]]

lexLitChar          :: ReadS String
lexLitChar ""       =  []
lexLitChar (c:s)
 | c /= '\\'        =  [([c],s)]
 | otherwise        =  map (prefix '\\') (lexEsc s)
 where
   lexEsc (c:s)     | c `elem` "abfnrtv\\\"'" = [([c],s)]
   lexEsc ('^':c:s) | c >= '@' && c <= '_'    = [(['^',c],s)]
    -- Numeric escapes
   lexEsc ('o':s)  = [prefix 'o' (span isOctDigit s)]
   lexEsc ('x':s)  = [prefix 'x' (span isHexDigit s)]
   lexEsc s@(c:_) 
     | isDigit c   = [span isDigit s]  
     | isUpper c   = case [(mne,s') | (c, mne) <- table,
	 	        ([],s') <- [lexmatch mne s]] of
                       (pr:_) -> [pr]
	               []     -> []
   lexEsc _        = []

   table = ('\DEL',"DEL") : asciiTab
   prefix c (t,s) = (c:t, s)

isOctDigit c  =  c >= '0' && c <= '7'
isHexDigit c  =  isDigit c || c >= 'A' && c <= 'F'
			   || c >= 'a' && c <= 'f'

lexmatch                   :: (Eq a) => [a] -> [a] -> ([a],[a])
lexmatch (x:xs) (y:ys) | x == y  =  lexmatch xs ys
lexmatch xs     ys               =  (xs,ys)

asciiTab = zip ['\NUL'..' ']
	   ["NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
	    "BS",  "HT",  "LF",  "VT",  "FF",  "CR",  "SO",  "SI",
	    "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
	    "CAN", "EM",  "SUB", "ESC", "FS",  "GS",  "RS",  "US",
	    "SP"]

readLitChar            :: ReadS Char
readLitChar ('\\':s)    = readEsc s
 where
       readEsc ('a':s)  = [('\a',s)]
       readEsc ('b':s)  = [('\b',s)]
       readEsc ('f':s)  = [('\f',s)]
       readEsc ('n':s)  = [('\n',s)]
       readEsc ('r':s)  = [('\r',s)]
       readEsc ('t':s)  = [('\t',s)]
       readEsc ('v':s)  = [('\v',s)]
       readEsc ('\\':s) = [('\\',s)]
       readEsc ('"':s)  = [('"',s)]
       readEsc ('\'':s) = [('\'',s)]
       readEsc ('^':c:s) | c >= '@' && c <= '_'
			= [(toEnum (fromEnum c - fromEnum '@'), s)]
       readEsc s@(d:_) | isDigit d
			= [(toEnum n, t) | (n,t) <- readDec s]
       readEsc ('o':s)  = [(toEnum n, t) | (n,t) <- readOct s]
       readEsc ('x':s)  = [(toEnum n, t) | (n,t) <- readHex s]
       readEsc s@(c:_) | isUpper c
			= let table = ('\DEL',"DEL") : asciiTab
			  in case [(c,s') | (c, mne) <- table,
					    ([],s') <- [lexmatch mne s]]
			     of (pr:_) -> [pr]
				[]     -> []
       readEsc _        = []
readLitChar (c:s)       = [(c,s)]

showLitChar               :: Char -> ShowS
showLitChar c | c > '\DEL' = showChar '\\' .
			     protectEsc isDigit (shows (fromEnum c))
showLitChar '\DEL'         = showString "\\DEL"
showLitChar '\\'           = showString "\\\\"
showLitChar c | c >= ' '   = showChar c
showLitChar '\a'           = showString "\\a"
showLitChar '\b'           = showString "\\b"
showLitChar '\f'           = showString "\\f"
showLitChar '\n'           = showString "\\n"
showLitChar '\r'           = showString "\\r"
showLitChar '\t'           = showString "\\t"
showLitChar '\v'           = showString "\\v"
showLitChar '\SO'          = protectEsc ('H'==) (showString "\\SO")
showLitChar c              = showString ('\\' : snd (asciiTab!!fromEnum c))

protectEsc p f             = f . cont
 where cont s@(c:_) | p c  = "\\&" ++ s
       cont s              = s

-- Unsigned readers for various bases
readDec, readOct, readHex :: Integral a => ReadS a
readDec = readInt 10 isDigit    (\ d -> fromEnum d - fromEnum_0)
readOct = readInt  8 isOctDigit (\ d -> fromEnum d - fromEnum_0)
readHex = readInt 16 isHexDigit hex
	    where hex d = fromEnum d - (if isDigit d then fromEnum_0
				       else fromEnum (if isUpper d then 'A' else 'a') - 10)

fromEnum_0 :: Int
fromEnum_0 = fromEnum '0'

-- readInt reads a string of digits using an arbitrary base.  
-- Leading minus signs must be handled elsewhere.

readInt :: Integral a => a -> (Char -> Bool) -> (Char -> Int) -> ReadS a
readInt radix isDig digToInt s =
    [(foldl1 (\n d -> n * radix + d) (map (fromIntegral . digToInt) ds), r)
	| (ds,r) <- nonnull isDig s ]

readSigned:: Real a => ReadS a -> ReadS a
readSigned readPos = readParen False read'
		     where read' r  = read'' r ++
				      [(-x,t) | ("-",s) <- lex r,
						(x,t)   <- read'' s]
			   read'' r = [(n,s)  | (str,s) <- lex r,
						(n,"")  <- readPos str]


-- This floating point reader uses a less restrictive syntax for floating
-- point than the Haskell lexer.  The `.' is optional.
readFloat     :: RealFrac a => ReadS a
readFloat r    = [(fromRational ((n%1)*10^^(k-d)),t) | (n,d,s) <- readFix r,
						       (k,t)   <- readExp s] ++
                 [ (0/0, t) | ("NaN",t)      <- lex r] ++
                 [ (1/0, t) | ("Infinity",t) <- lex r]
		 where readFix r = [(read (ds++ds'), length ds', t)
					| (ds, d) <- lexDigits r
                                        , (ds',t) <- lexFrac d   ]

                       lexFrac ('.':s) = lexDigits s
		       lexFrac s       = [("",s)]

		       readExp (e:s) | e `elem` "eE" = readExp' s
		       readExp s                     = [(0,s)]

		       readExp' ('-':s) = [(-k,t) | (k,t) <- readDec s]
		       readExp' ('+':s) = readDec s
		       readExp' s       = readDec s

----------------------------------------------------------------
-- Exception datatype and operations
----------------------------------------------------------------

data Exception
  = ArithException      ArithException
  | ArrayException      ArrayException
  | AssertionFailed     String
  | AsyncException      AsyncException
  | BlockedOnDeadMVar
  | Deadlock
  | DynException        Dynamic
  | ErrorCall           String
  | ExitException       ExitCode
  | IOException 	IOException	-- IO exceptions (from 'ioError')
  | NoMethodError       String
  | NonTermination
  | PatternMatchFail    String
  | RecConError         String
  | RecSelError         String
  | RecUpdError         String

instance Show Exception where
  showsPrec _ (ArithException e)  = shows e
  showsPrec _ (ArrayException e)  = shows e
  showsPrec _ (AssertionFailed s) = showException "assertion failed" s
  showsPrec _ (AsyncException e)  = shows e
  showsPrec _ BlockedOnDeadMVar   = showString "thread blocked indefinitely"
  showsPrec _ Deadlock            = showString "<<deadlock>>"
  showsPrec _ (DynException _)    = showString "unknown exception"
  showsPrec _ (ErrorCall s)       = showString s
  showsPrec _ (ExitException err) = showString "exit: " . shows err
  showsPrec _ (IOException err)	  = shows err
  showsPrec _ (NoMethodError s)   = showException "undefined member" s
  showsPrec _ NonTermination	  = showString "<<loop>>"
  showsPrec _ (PatternMatchFail s) = showException "pattern match failure" s
  showsPrec _ (RecConError s)     = showException "undefined field" s
  showsPrec _ (RecSelError s)     = showException "select of missing field" s
  showsPrec _ (RecUpdError s)     = showException "update of missing field" s

data ArithException
  = Overflow
  | Underflow
  | LossOfPrecision
  | DivideByZero
  | Denormal
  deriving (Eq, Ord)

instance Show ArithException where
  showsPrec _ Overflow        = showString "arithmetic overflow"
  showsPrec _ Underflow       = showString "arithmetic underflow"
  showsPrec _ LossOfPrecision = showString "loss of precision"
  showsPrec _ DivideByZero    = showString "divide by zero"
  showsPrec _ Denormal        = showString "denormal"

data ArrayException
  = IndexOutOfBounds    String
  | UndefinedElement    String
  deriving (Eq, Ord)

instance Show ArrayException where
  showsPrec _ (IndexOutOfBounds s) =
    showException "array index out of range" s
  showsPrec _ (UndefinedElement s) =
    showException "undefined array element" s

data AsyncException
  = StackOverflow
  | HeapOverflow
  | ThreadKilled
  deriving (Eq, Ord)

instance Show AsyncException where
  showsPrec _ StackOverflow   = showString "stack overflow"
  showsPrec _ HeapOverflow    = showString "heap overflow"
  showsPrec _ ThreadKilled    = showString "thread killed"

showException :: String -> String -> ShowS
showException tag msg =
  showString tag . (if null msg then id else showString ": " . showString msg)

data ExitCode = ExitSuccess | ExitFailure Int
                deriving (Eq, Ord, Read, Show)

-- data type describing IOErrors / exceptions.
type IOError = IOException

data IOException
  = IOError
      { ioe_handle      :: Maybe Handle   -- the handle used by the action
					  -- flagging the error
      , ioe_type        :: IOErrorType    -- what kind of (std) error
      , ioe_location    :: String         -- location of the error
      , ioe_description :: String         -- error-specific string
      , ioe_filename    :: Maybe FilePath -- the resource involved.
      } 
      deriving (Eq)

data IOErrorType
  = AlreadyExists
  | NoSuchThing
  | ResourceBusy
  | ResourceExhausted
  | EOF
  | IllegalOperation
  | PermissionDenied
  | UserError
     -- GHC compatibility
  | ProtocolError
  | UnsupportedOperation
  | OtherError
    deriving (Eq)

instance Show IOErrorType where
  show x = 
    case x of
      AlreadyExists     -> "already exists"
      NoSuchThing       -> "does not exist"
      ResourceBusy      -> "resource busy"
      ResourceExhausted -> "resource exhausted"
      EOF               -> "end of file"
      IllegalOperation  -> "illegal operation"
      PermissionDenied  -> "permission denied"
      UserError         -> "user error"
      ProtocolError     -> "protocol error"
      UnsupportedOperation -> "unsupported operation"
      OtherError        -> "failed"

instance Show IOException where
  showsPrec p (IOError hdl iot loc s fn) =
    (case fn of
       Nothing -> case hdl of
		      Nothing -> id
		      Just h  -> showsPrec p h . showString ": "
       Just name -> showString name . showString ": ") .
    (case loc of
       "" -> id
       _  -> showString loc . showString ": ") .
    showsPrec p iot .
    (case s of
       "" -> id
       _  -> showString " (" . showString s . showString ")")

-- Monadic I/O: --------------------------------------------------------------

--data IO a             -- builtin datatype of IO actions

type FilePath = String  -- file pathnames are represented by strings

primitive primbindIO		 :: IO a -> (a -> IO b) -> IO b
primitive primretIO		 :: a -> IO a

ioError :: IOError -> IO a
ioError e = IO (\ s -> throw (IOException e))

userError :: String -> IOError
userError str = IOError Nothing UserError "" str Nothing

catch :: IO a -> (IOError -> IO a) -> IO a
catch m h = catchException m $ \e -> case e of
		IOException err -> h err
		_ -> throw e

putChar   :: Char -> IO ()
putChar    = hPutChar stdout

putStr    :: String -> IO ()
putStr     = hPutStr stdout

print     :: Show a => a -> IO ()
print      = putStrLn . show

putStrLn  :: String -> IO ()
putStrLn s = do putStr s
		putChar '\n'

getChar   :: IO Char
getChar    = hGetChar stdin

getContents :: IO String
getContents  = hGetContents stdin

getLine   :: IO String
getLine    = hGetLine stdin

hGetLine :: Handle -> IO String
hGetLine h = do 
  c <- hGetChar h
  hGetLine' c
  where
   hGetLine' '\n' = return ""
   hGetLine' c = do
     cs <- getRest
     return (c:cs)
   getRest = do
     c <- catch (hGetChar h) $ \ ex ->
	if isEOFError ex then return '\n' else ioError ex
     hGetLine' c
   isEOFError ex = ioe_type ex == EOF	-- defined in System.IO.Error

-- raises an exception instead of an error
readIO          :: Read a => String -> IO a
readIO s         = case [x | (x,t) <- reads s, ("","") <- lex t] of
                        [x] -> return x
                        []  -> ioError (userError "PreludeIO.readIO: no parse")
                        _   -> ioError (userError 
                                       "PreludeIO.readIO: ambiguous parse")

readLn          :: Read a => IO a
readLn           = do l <- getLine
                      r <- readIO l
                      return r

data IOMode      =  ReadMode | WriteMode | AppendMode | ReadWriteMode
                    deriving (Eq, Ord, Ix, Bounded, Enum, Read, Show)

writeFile	:: FilePath -> String -> IO ()
writeFile        = writeFile' WriteMode

appendFile	:: FilePath -> String -> IO ()
appendFile       = writeFile' AppendMode

writeFile'      :: IOMode -> FilePath -> String -> IO ()
writeFile' mode name s = do
  h <- openFile name mode
  catchException (hPutStr h s) (\e -> hClose h >> throw e)
  hClose h

readFile	:: FilePath -> IO String
readFile name    = openFile name ReadMode >>= hGetContents

interact  :: (String -> String) -> IO ()
interact f = getContents >>= (putStr . f)

primitive stdin       :: Handle
primitive stdout      :: Handle
primitive stderr      :: Handle
primitive openFile    :: FilePath -> IOMode -> IO Handle
primitive hClose      :: Handle -> IO ()
primitive hGetContents :: Handle -> IO String
primitive hGetChar    :: Handle -> IO Char
primitive hPutChar    :: Handle -> Char -> IO ()
primitive hPutStr     :: Handle -> String -> IO ()

instance Functor IO where
    fmap f x = x `primbindIO` (primretIO . f)

instance Applicative IO where
    pure     = primretIO
    f <*> x  = f `primbindIO` \g -> x `primbindIO` (primretIO . g)
    liftA2 f x y = x `primbindIO` \a -> y `primbindIO` (primretIO . f a)
    x *> y   = x `primbindIO` \_ -> y

instance Monad IO where
    (>>=)  = primbindIO
    return = primretIO

instance MonadFail IO where
    fail s = ioError (userError s)

-- Hooks for primitives: -----------------------------------------------------
-- Do not mess with these!

data FunPtr a -- builtin datatype of C function pointers
data Ptr a    -- builtin datatype of C pointers
data Addr     -- builtin datatype of C pointers (deprecated)
data Word     -- builtin datatype of unsigned ints (deprecated)
data Int8
data Int16
data Int32
data Int64
data Word8
data Word16
data Word32
data Word64
data ForeignObj  -- builtin datatype of C pointers with finalizers (deprecated)
data ForeignPtr a -- builtin datatype of C pointers with finalizers
data StablePtr a
data Handle

data Object a -- builtin datatype of external object references.
              -- (needed as primitive since they're supported in FFI decls.)

instance Eq Handle where (==) = primEqHandle
primitive primEqHandle :: Handle -> Handle -> Bool

instance Show Handle where
    showsPrec _ h = case primGetHandleNumber h of
	0 -> showString "<stdin>"
	1 -> showString "<stdout>"
	2 -> showString "<stderr>"
	_ -> showString "<handle>"

primitive primGetHandleNumber :: Handle -> Int

primitive unsafeCoerce "primUnsafeCoerce" :: a -> b

data Dynamic = Dynamic TypeRep Obj

data TypeRep = TypeRep !Key TyCon [TypeRep]

instance Eq TypeRep where
  (TypeRep k1 _ _) == (TypeRep k2 _ _) = k1 == k2

data TyCon = TyCon !Key String

instance Eq TyCon where
  (TyCon t1 _) == (TyCon t2 _) = t1 == t2

newtype Key = Key Int deriving( Eq )

data Obj = Obj

toObj :: a -> Obj
toObj   = unsafeCoerce

fromObj :: Obj -> a
fromObj = unsafeCoerce

newtype IO a = IO ((a -> IOResult) -> IOResult)

data IOResult 
  = Cupid_ExitWith    Int
  | Cupid_Catch       IOResult (Exception -> IOResult) (Obj -> IOResult)
  | Cupid_ForkThread  IOResult IOResult
  | Cupid_DeadThread
  | Cupid_YieldThread IOResult
  | Cupid_Return      Obj
  | Cupid_BlockThread (Obj -> IOResult) ((Obj -> IOResult) -> IOResult) 

data IOFinished a
  = Finished_ExitWith Int
  | Finished_Return   a

primitive throw "primThrowException" :: Exception -> a
primitive primCatchException :: a -> Either Exception a

catchException :: IO a -> (Exception -> IO a) -> IO a
catchException (IO m) k = IO $ \ s ->
  Cupid_Catch (m cupidReturn)
             (\ e -> case (k e) of { IO k' -> k' s })
             (s . fromObj)

cupidReturn :: a -> IOResult
cupidReturn x = Cupid_Return (toObj x)

-- reify current thread, execute 'm <thread>' and switch to next thread
blockIO :: ((a -> IOResult) -> IO ()) -> IO a
blockIO m = IO (\ s -> Cupid_BlockThread (s . fromObj) m')
 where
  m' k = threadToIOResult (m (k . toObj))

cupidIORun  :: IO a -> Either Int a
cupidIORun m = 
  case basicIORun (runAndShowError m) of
    Finished_ExitWith i -> Left i
    Finished_Return   a -> Right a
 where
  runAndShowError :: IO a -> IO a
  runAndShowError m = m `catchException` exceptionHandler
  exceptionHandler :: Exception -> IO a
  exceptionHandler (ExitException ExitSuccess) = primExitWith 0
  exceptionHandler (ExitException (ExitFailure n)) = primExitWith n
  exceptionHandler err = runAndShowError $ do
	putChar '\n'
	putStr "Program error: "
	putStrLn (show err)
	primExitWith 1

basicIORun :: IO a -> IOFinished a
basicIORun (IO m) = loop [m cupidReturn]

threadToIOResult :: IO a -> IOResult
threadToIOResult (IO m) = m (const Cupid_DeadThread)

-- This is the queue of *runnable* threads.
-- There may be blocked threads attached to MVars
-- An important invariant is that at most one thread will result in
-- Cupid_Return - and its Obj value has type \alpha
loop :: [IOResult] -> IOFinished a
loop []                      = error "no more threads (deadlock?)"
loop [Cupid_Return   a]       = Finished_Return (fromObj a)
loop (Cupid_Return   a:r)     = loop (r ++ [Cupid_Return a])
loop (Cupid_Catch m f s:r)    = loop (cupid_catch m f s : r)
loop (Cupid_ExitWith i:_)     = Finished_ExitWith i
loop (Cupid_DeadThread:r)     = loop r
loop (Cupid_ForkThread a b:r) = loop (a:b:r)
loop (Cupid_YieldThread a:r)  = loop (r ++ [a])
loop (Cupid_BlockThread a b:r)= loop (b a : r)
loop _                       = error "Fatal error in Cupid scheduler"

cupid_catch :: IOResult -> (Exception -> IOResult) -> (Obj -> IOResult) -> IOResult
cupid_catch m f s = case primCatchException (catch' m) of
  Left  exn                   -> f exn
  Right (Cupid_Return a)       -> s a
  Right (Cupid_ForkThread a b) -> Cupid_ForkThread (Cupid_Catch a f s) b
  Right (Cupid_YieldThread a)  -> Cupid_YieldThread (Cupid_Catch a f s)
  Right (Cupid_BlockThread a b)-> Cupid_BlockThread (\x -> Cupid_Catch (a x) f s) b
  Right r                     -> r
 where
  catch' :: IOResult -> IOResult
  catch' (Cupid_Catch m' f' s') = catch' (cupid_catch m' f' s')
  catch' x                     = x

primExitWith     :: Int -> IO a
primExitWith c    = IO (\ s -> Cupid_ExitWith c)

primCompAux      :: Ord a => a -> a -> Ordering -> Ordering
primCompAux x y o = case compare x y of EQ -> o; LT -> LT; GT -> GT

primPmInt        :: Num a => Int -> a -> Bool
primPmInt n x     = fromInt n == x

primPmInteger    :: Num a => Integer -> a -> Bool
primPmInteger n x = fromInteger n == x

primPmFlt        :: Fractional a => Double -> a -> Bool
primPmFlt n x     = fromDouble n == x

-- The following primitives are only needed if (n+k) patterns are enabled:
primPmNpk        :: Integral a => Int -> a -> Maybe a
primPmNpk n x     = if n'<=x then Just (x-n') else Nothing
		    where n' = fromInt n

primPmSub        :: Integral a => Int -> a -> a
primPmSub n x     = x - fromInt n

-- End of Cupid standard prelude ----------------------------------------------
