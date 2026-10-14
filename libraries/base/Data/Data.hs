-----------------------------------------------------------------------------
-- |
-- Module      :  Data.Data
-- License     :  BSD-style (see the file libraries/base/LICENSE)
--
-- Stability   :  experimental
-- Portability :  non-portable (rank-2 types)
--
-- "Scrap your boilerplate": the 'Data' class of types whose values can
-- be taken apart and built generically, constructor by constructor, and
-- the representations of their constructors and data types.  @deriving
-- Data@ defines an instance for a data type.
--
-- 'dataCast1' and 'dataCast2' take 'Typeable1' and 'Typeable2' evidence
-- for the type constructor, as Cupid's 'Typeable' is for types of kind
-- 'Data.Kind.Type' only.

module Data.Data (
	module Data.Typeable,
	-- * The Data class
	Data(..),
	-- * Datatype representations
	DataType, mkDataType, mkIntType, mkFloatType, mkCharType,
	mkNoRepType, dataTypeName, DataRep(..), dataTypeRep,
	repConstr, isAlgType, dataTypeConstrs, indexConstr, maxConstrIndex,
	isNorepType,
	-- * Data constructor representations
	Constr, ConIndex, Fixity(..), mkConstr, mkConstrTag, mkIntegralConstr,
	mkRealConstr, mkCharConstr, constrType, ConstrRep(..), constrRep,
	constrFields, constrFixity, constrIndex, showConstr, readConstr,
	-- * Unqualified names
	tyconUQname, tyconModule,
	-- * Building values from constructors
	fromConstr, fromConstrB, fromConstrM,
	-- * For derived instances
	mkDerivedDataType, gunfoldIndex
	) where

import Data.Typeable
import Data.Maybe (fromJust)
import Data.Ratio
import Data.Int
import Data.Word
import Cupid.Prelude (NonEmpty(..))
import Control.Monad (MonadPlus(..))

-- ---------------------------------------------------------------------------
-- The Data class

-- | Types whose values can be folded over and built constructor by
-- constructor.  Minimal complete definition: 'gunfold', 'toConstr' and
-- 'dataTypeOf', and 'gfoldl' for a type with fields.
class Typeable a => Data a where
	-- | Fold over the immediate subterms of a value: @gfoldl k z (C x y)
	-- = (z C `k` x) `k` y@.
	gfoldl :: (forall d b . Data d => c (d -> b) -> d -> c b)
	       -> (forall g . g -> c g) -> a -> c a
	gfoldl _ z = z

	-- | Build a value of the given constructor, its fields from @k@.
	gunfold :: (forall b r . Data b => c (b -> r) -> c r)
	        -> (forall r . r -> c r) -> Constr -> c a

	-- | The constructor of a value.
	toConstr :: a -> Constr

	-- | The data type of a value's type; the argument is not looked at.
	dataTypeOf :: a -> DataType

	-- | Cast a value of a type constructor applied to a type.
	dataCast1 :: Typeable1 t => (forall d . Data d => c (t d)) -> Maybe (c a)
	dataCast1 _ = Nothing

	-- | Cast a value of a type constructor applied to two types.
	dataCast2 :: Typeable2 t => (forall d e . (Data d, Data e) => c (t d e))
	          -> Maybe (c a)
	dataCast2 _ = Nothing

	-- | Transform the immediate subterms.
	gmapT :: (forall b . Data b => b -> b) -> a -> a
	gmapT f x0 = unID (gfoldl (\(ID c) x -> ID (c (f x))) ID x0)

	-- | Fold the immediate subterms from the left.
	gmapQl :: (r -> r' -> r) -> r -> (forall d . Data d => d -> r') -> a -> r
	gmapQl o r f = unCONST . gfoldl (\c x -> CONST (unCONST c `o` f x)) (\_ -> CONST r)

	-- | Fold the immediate subterms from the right.
	gmapQr :: (r' -> r -> r) -> r -> (forall d . Data d => d -> r') -> a -> r
	gmapQr o r0 f x0 = unQr (gfoldl (\(Qr c) x -> Qr (\r -> c (f x `o` r))) (\_ -> Qr id) x0) r0

	-- | The results of a query on the immediate subterms.
	gmapQ :: (forall d . Data d => d -> u) -> a -> [u]
	gmapQ f = gmapQr (:) [] f

	-- | A query on the immediate subterm at an index.
	gmapQi :: Int -> (forall d . Data d => d -> u) -> a -> u
	gmapQi i f x = case gfoldl (\(Qi n q) a -> Qi (n + 1) (if i == n then Just (f a) else q))
	                           (\_ -> Qi 0 Nothing) x of
		Qi _ q -> fromJust q

	-- | A monadic transformation of the immediate subterms.
	gmapM :: Monad m => (forall d . Data d => d -> m d) -> a -> m a
	gmapM f = gfoldl (\c x -> do { c' <- c; x' <- f x; return (c' x') }) return

	-- | A transformation of at least one immediate subterm.
	gmapMp :: MonadPlus m => (forall d . Data d => d -> m d) -> a -> m a
	gmapMp f x = unMp (gfoldl k z x) >>= \(x', b) ->
	                 if b then return x' else mzero
	  where
		z g = Mp (return (g, False))
		k (Mp c) y = Mp (c >>= \(h, b) ->
			(f y >>= \y' -> return (h y', True)) `mplus` return (h y, b))

	-- | A transformation of one immediate subterm.
	gmapMo :: MonadPlus m => (forall d . Data d => d -> m d) -> a -> m a
	gmapMo f x = unMp (gfoldl k z x) >>= \(x', b) ->
	                 if b then return x' else mzero
	  where
		z g = Mp (return (g, False))
		k (Mp c) y = Mp (c >>= \(h, b) -> if b then return (h y, b)
			else (f y >>= \y' -> return (h y', True)) `mplus` return (h y, b))

-- The type constructors the default methods fold into.
newtype ID x = ID { unID :: x }
newtype CONST c a = CONST { unCONST :: c }
newtype Qr r a = Qr { unQr :: r -> r }
data Qi q a = Qi Int (Maybe q)
newtype Mp m x = Mp { unMp :: m (x, Bool) }

-- ---------------------------------------------------------------------------
-- Building values from constructors

-- | The value of a constructor without fields.
fromConstr :: Data a => Constr -> a
fromConstr = fromConstrB (error "Data.Data.fromConstr")

-- | The value of a constructor, each field built by the given function.
fromConstrB :: Data a => (forall d . Data d => d) -> Constr -> a
fromConstrB f = unID . gunfold (\(ID c) -> ID (c f)) ID

-- | The value of a constructor, each field built monadically.
fromConstrM :: (Monad m, Data a) => (forall d . Data d => m d) -> Constr -> m a
fromConstrM f = gunfold (\c -> do { c' <- c; x <- f; return (c' x) }) return

-- ---------------------------------------------------------------------------
-- Datatype representations

-- | The representation of a data type.
data DataType = DataType { tycon :: String, datarep :: DataRep }

instance Show DataType where
	showsPrec d (DataType t r) = showParen (d > 10) $
		showString "DataType {tycon = " . shows t .
		showString ", datarep = " . shows r . showChar '}'

-- | The constructors of an algebraic data type, or the primitive type.
data DataRep = AlgRep [Constr] | IntRep | FloatRep | CharRep | NoRep
	deriving (Eq, Show)

-- | The representation of a constructor.
data Constr = Constr
	{ conrep    :: ConstrRep
	, constring :: String
	, confields :: [String]
	, confixity :: Fixity
	, datatype  :: DataType
	}

instance Show Constr where
	showsPrec _ = showString . constring

-- | Constructors are equal when their representations are.
instance Eq Constr where
	c == c' = constrRep c == constrRep c'

-- | A constructor of an algebraic data type, by index, or a value of a
-- primitive type.
data ConstrRep = AlgConstr ConIndex | IntConstr Integer | FloatConstr Rational
               | CharConstr Char
	deriving (Eq, Show)

-- | The index of a constructor in its data type, counting from 1.
type ConIndex = Int

-- | Whether a constructor is written infix.
data Fixity = Prefix | Infix
	deriving (Eq, Show)

-- | The data type with the given name and constructors.
mkDataType :: String -> [Constr] -> DataType
mkDataType str cs = DataType { tycon = str, datarep = AlgRep cs }

-- | A constructor of a data type, with its name, field labels and fixity;
-- its index is its place in the data type's list.
mkConstr :: DataType -> String -> [String] -> Fixity -> Constr
mkConstr dt str fields fix = Constr
	{ conrep = AlgConstr idx, constring = str, confields = fields
	, confixity = fix, datatype = dt }
  where
	idx = case [ i | (c, i) <- zip (dataTypeConstrs dt) [1..], showConstr c == str ] of
		(i:_) -> i
		[]    -> error ("Data.Data.mkConstr: no constructor " ++ str)

-- | A constructor of a data type with the given index.
mkConstrTag :: DataType -> String -> Int -> [String] -> Fixity -> Constr
mkConstrTag dt str idx fields fix = Constr
	{ conrep = AlgConstr idx, constring = str, confields = fields
	, confixity = fix, datatype = dt }

-- | The data type of a derived instance: its qualified name, and its
-- constructors' names, field labels and whether each is infix.
mkDerivedDataType :: String -> [(String, [String], Bool)] -> DataType
mkDerivedDataType name cons = dt
  where
	dt = mkDataType name [ mkConstrTag dt n i fs (if inf then Infix else Prefix)
	                     | ((n, fs, inf), i) <- zip cons [1..] ]

-- | 'gunfold' of a derived instance: the alternative for the
-- constructor's index.
gunfoldIndex :: Constr -> [c a] -> c a
gunfoldIndex c alts = alts !! (constrIndex c - 1)

mkPrimType :: DataRep -> String -> DataType
mkPrimType dr str = DataType { tycon = str, datarep = dr }

mkIntType, mkFloatType, mkCharType, mkNoRepType :: String -> DataType
mkIntType = mkPrimType IntRep
mkFloatType = mkPrimType FloatRep
mkCharType = mkPrimType CharRep
mkNoRepType = mkPrimType NoRep

mkPrimCon :: DataType -> String -> ConstrRep -> Constr
mkPrimCon dt str cr = Constr
	{ conrep = cr, constring = str, confields = [], confixity = Prefix
	, datatype = dt }

mkIntegralConstr :: (Integral a, Show a) => DataType -> a -> Constr
mkIntegralConstr dt i = mkPrimCon dt (show i) (IntConstr (toInteger i))

mkRealConstr :: (Real a, Show a) => DataType -> a -> Constr
mkRealConstr dt f = mkPrimCon dt (show f) (FloatConstr (toRational f))

mkCharConstr :: DataType -> Char -> Constr
mkCharConstr dt c = mkPrimCon dt (show c) (CharConstr c)

dataTypeName :: DataType -> String
dataTypeName = tycon

dataTypeRep :: DataType -> DataRep
dataTypeRep = datarep

constrType :: Constr -> DataType
constrType = datatype

constrRep :: Constr -> ConstrRep
constrRep = conrep

-- | The primitive constructor of a value of a primitive type.
repConstr :: DataType -> ConstrRep -> Constr
repConstr dt cr = case (dataTypeRep dt, cr) of
	(AlgRep cs, AlgConstr i)     -> cs !! (i - 1)
	(IntRep,    IntConstr i)     -> mkPrimCon dt (show i) cr
	(FloatRep,  FloatConstr f)   -> mkPrimCon dt (show (fromRational f :: Double)) cr
	(CharRep,   CharConstr c)    -> mkPrimCon dt (show c) cr
	_ -> error "Data.Data.repConstr: the representation does not fit the data type"

isAlgType :: DataType -> Bool
isAlgType dt = case datarep dt of
	AlgRep _ -> True
	_        -> False

isNorepType :: DataType -> Bool
isNorepType dt = case datarep dt of
	NoRep -> True
	_     -> False

dataTypeConstrs :: DataType -> [Constr]
dataTypeConstrs dt = case datarep dt of
	AlgRep cons -> cons
	_ -> error ("Data.Data.dataTypeConstrs is not supported for "
		++ dataTypeName dt ++ ", as it is not an algebraic data type.")

-- | The constructor of a data type with an index, counting from 1.
indexConstr :: DataType -> ConIndex -> Constr
indexConstr dt idx = case datarep dt of
	AlgRep cs -> cs !! (idx - 1)
	_ -> error ("Data.Data.indexConstr is not supported for "
		++ dataTypeName dt ++ ", as it is not an algebraic data type.")

maxConstrIndex :: DataType -> ConIndex
maxConstrIndex dt = case dataTypeRep dt of
	AlgRep cs -> length cs
	_ -> error ("Data.Data.maxConstrIndex is not supported for "
		++ dataTypeName dt ++ ", as it is not an algebraic data type.")

constrFields :: Constr -> [String]
constrFields = confields

constrFixity :: Constr -> Fixity
constrFixity = confixity

-- | The index of a constructor of an algebraic data type.
constrIndex :: Constr -> ConIndex
constrIndex con = case constrRep con of
	AlgConstr idx -> idx
	_ -> error ("Data.Data.constrIndex is not supported for "
		++ dataTypeName (constrType con) ++ ", as it is not an algebraic data type.")

showConstr :: Constr -> String
showConstr = constring

-- | The constructor of a data type with a name, or for a primitive type
-- the value it shows as.
readConstr :: DataType -> String -> Maybe Constr
readConstr dt str = case dataTypeRep dt of
	AlgRep cons -> case filter ((== str) . showConstr) cons of
		(c:_) -> Just c
		[]    -> Nothing
	IntRep -> case [ i | (i, "") <- reads str ] of
		(i:_) -> Just (mkPrimCon dt str (IntConstr i))
		[]    -> Nothing
	FloatRep -> case [ d | (d, "") <- reads str ] of
		(d:_) -> Just (mkPrimCon dt str (FloatConstr (toRational (d :: Double))))
		[]    -> Nothing
	CharRep -> case [ c | (c, "") <- reads str ] of
		(c:_) -> Just (mkPrimCon dt str (CharConstr c))
		[]    -> Nothing
	NoRep -> Nothing

-- | The name of a type constructor without its module.
tyconUQname :: String -> String
tyconUQname x = case dropWhile (/= '.') x of
	"" -> x
	(_:tl) -> tyconUQname tl

-- | The module of a qualified type constructor name.
tyconModule :: String -> String
tyconModule x = case break (== '.') x of
	(_, "") -> ""
	(a, _:tl) -> case tyconModule tl of
		"" -> a
		m  -> a ++ '.' : m

-- ---------------------------------------------------------------------------
-- Instances for the Prelude's types

instance Data Int where
	toConstr x = mkIntegralConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		IntConstr x -> z (fromIntegral x)
		_ -> error "Data.Data.gunfold: Int"
	dataTypeOf _ = mkIntType "Prelude.Int"

instance Data Integer where
	toConstr x = mkIntegralConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		IntConstr x -> z x
		_ -> error "Data.Data.gunfold: Integer"
	dataTypeOf _ = mkIntType "Prelude.Integer"

instance Data Word where
	toConstr x = mkIntegralConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		IntConstr x -> z (fromIntegral x)
		_ -> error "Data.Data.gunfold: Word"
	dataTypeOf _ = mkIntType "Prelude.Word"

instance Data Int8 where
	toConstr x = mkIntegralConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		IntConstr x -> z (fromIntegral x)
		_ -> error "Data.Data.gunfold: Int8"
	dataTypeOf _ = mkIntType "Data.Int.Int8"

instance Data Int16 where
	toConstr x = mkIntegralConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		IntConstr x -> z (fromIntegral x)
		_ -> error "Data.Data.gunfold: Int16"
	dataTypeOf _ = mkIntType "Data.Int.Int16"

instance Data Int32 where
	toConstr x = mkIntegralConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		IntConstr x -> z (fromIntegral x)
		_ -> error "Data.Data.gunfold: Int32"
	dataTypeOf _ = mkIntType "Data.Int.Int32"

instance Data Int64 where
	toConstr x = mkIntegralConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		IntConstr x -> z (fromIntegral x)
		_ -> error "Data.Data.gunfold: Int64"
	dataTypeOf _ = mkIntType "Data.Int.Int64"

instance Data Word8 where
	toConstr x = mkIntegralConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		IntConstr x -> z (fromIntegral x)
		_ -> error "Data.Data.gunfold: Word8"
	dataTypeOf _ = mkIntType "Data.Word.Word8"

instance Data Word16 where
	toConstr x = mkIntegralConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		IntConstr x -> z (fromIntegral x)
		_ -> error "Data.Data.gunfold: Word16"
	dataTypeOf _ = mkIntType "Data.Word.Word16"

instance Data Word32 where
	toConstr x = mkIntegralConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		IntConstr x -> z (fromIntegral x)
		_ -> error "Data.Data.gunfold: Word32"
	dataTypeOf _ = mkIntType "Data.Word.Word32"

instance Data Word64 where
	toConstr x = mkIntegralConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		IntConstr x -> z (fromIntegral x)
		_ -> error "Data.Data.gunfold: Word64"
	dataTypeOf _ = mkIntType "Data.Word.Word64"

instance Data Float where
	toConstr x = mkRealConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		FloatConstr x -> z (realToFrac x)
		_ -> error "Data.Data.gunfold: Float"
	dataTypeOf _ = mkFloatType "Prelude.Float"

instance Data Double where
	toConstr x = mkRealConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		FloatConstr x -> z (realToFrac x)
		_ -> error "Data.Data.gunfold: Double"
	dataTypeOf _ = mkFloatType "Prelude.Double"

instance Data Char where
	toConstr x = mkCharConstr (dataTypeOf x) x
	gunfold _ z c = case constrRep c of
		CharConstr x -> z x
		_ -> error "Data.Data.gunfold: Char"
	dataTypeOf _ = mkCharType "Prelude.Char"

instance (Data a, Integral a) => Data (Ratio a) where
	gfoldl k z r = z (%) `k` numerator r `k` denominator r
	toConstr _ = ratioConstr
	gunfold k z c | constrIndex c == 1 = k (k (z (%)))
	gunfold _ _ _ = error "Data.Data.gunfold: Ratio"
	dataTypeOf _ = ratioDataType

ratioConstr :: Constr
ratioConstr = mkConstr ratioDataType ":%" [] Infix

ratioDataType :: DataType
ratioDataType = mkDataType "GHC.Real.Ratio" [ratioConstr]

instance Data Bool where
	toConstr False = indexConstr boolDataType 1
	toConstr True  = indexConstr boolDataType 2
	gunfold _ z c = case constrIndex c of
		1 -> z False
		_ -> z True
	dataTypeOf _ = boolDataType

boolDataType :: DataType
boolDataType = mkDerivedDataType "Prelude.Bool" [("False", [], False), ("True", [], False)]

instance Data () where
	toConstr _ = indexConstr unitDataType 1
	gunfold _ z _ = z ()
	dataTypeOf _ = unitDataType

unitDataType :: DataType
unitDataType = mkDerivedDataType "Prelude.()" [("()", [], False)]

instance Data Ordering where
	toConstr LT = indexConstr orderingDataType 1
	toConstr EQ = indexConstr orderingDataType 2
	toConstr GT = indexConstr orderingDataType 3
	gunfold _ z c = case constrIndex c of
		1 -> z LT
		2 -> z EQ
		_ -> z GT
	dataTypeOf _ = orderingDataType

orderingDataType :: DataType
orderingDataType = mkDerivedDataType "Prelude.Ordering"
	[("LT", [], False), ("EQ", [], False), ("GT", [], False)]

instance Data a => Data [a] where
	gfoldl _ z []     = z []
	gfoldl f z (x:xs) = z (:) `f` x `f` xs
	toConstr []    = indexConstr listDataType 1
	toConstr (_:_) = indexConstr listDataType 2
	gunfold k z c = case constrIndex c of
		1 -> z []
		_ -> k (k (z (:)))
	dataTypeOf _ = listDataType
	dataCast1 f = gcast1 f

listDataType :: DataType
listDataType = mkDerivedDataType "Prelude.[]" [("[]", [], False), ("(:)", [], True)]

instance Data a => Data (Maybe a) where
	gfoldl _ z Nothing  = z Nothing
	gfoldl f z (Just x) = z Just `f` x
	toConstr Nothing  = indexConstr maybeDataType 1
	toConstr (Just _) = indexConstr maybeDataType 2
	gunfold k z c = case constrIndex c of
		1 -> z Nothing
		_ -> k (z Just)
	dataTypeOf _ = maybeDataType
	dataCast1 f = gcast1 f

maybeDataType :: DataType
maybeDataType = mkDerivedDataType "Prelude.Maybe" [("Nothing", [], False), ("Just", [], False)]

instance (Data a, Data b) => Data (Either a b) where
	gfoldl f z (Left a)  = z Left `f` a
	gfoldl f z (Right a) = z Right `f` a
	toConstr (Left _)  = indexConstr eitherDataType 1
	toConstr (Right _) = indexConstr eitherDataType 2
	gunfold k z c = case constrIndex c of
		1 -> k (z Left)
		_ -> k (z Right)
	dataTypeOf _ = eitherDataType
	dataCast2 f = gcast2 f

eitherDataType :: DataType
eitherDataType = mkDerivedDataType "Prelude.Either" [("Left", [], False), ("Right", [], False)]

instance Data a => Data (NonEmpty a) where
	gfoldl f z (a :| as) = z (:|) `f` a `f` as
	toConstr _ = indexConstr nonEmptyDataType 1
	gunfold k z _ = k (k (z (:|)))
	dataTypeOf _ = nonEmptyDataType

nonEmptyDataType :: DataType
nonEmptyDataType = mkDerivedDataType "GHC.Base.NonEmpty" [(":|", [], True)]

instance (Data a, Data b) => Data (a, b) where
	gfoldl f z (a, b) = z (,) `f` a `f` b
	toConstr _ = indexConstr (tupleDataType 2) 1
	gunfold k z _ = k (k (z (,)))
	dataTypeOf _ = tupleDataType 2
	dataCast2 f = gcast2 f

instance (Data a, Data b, Data c) => Data (a, b, c) where
	gfoldl f z (a, b, c) = z (,,) `f` a `f` b `f` c
	toConstr _ = indexConstr (tupleDataType 3) 1
	gunfold k z _ = k (k (k (z (,,))))
	dataTypeOf _ = tupleDataType 3

instance (Data a, Data b, Data c, Data d) => Data (a, b, c, d) where
	gfoldl f z (a, b, c, d) = z (,,,) `f` a `f` b `f` c `f` d
	toConstr _ = indexConstr (tupleDataType 4) 1
	gunfold k z _ = k (k (k (k (z (,,,)))))
	dataTypeOf _ = tupleDataType 4

instance (Data a, Data b, Data c, Data d, Data e) => Data (a, b, c, d, e) where
	gfoldl f z (a, b, c, d, e) = z (,,,,) `f` a `f` b `f` c `f` d `f` e
	toConstr _ = indexConstr (tupleDataType 5) 1
	gunfold k z _ = k (k (k (k (k (z (,,,,))))))
	dataTypeOf _ = tupleDataType 5

tupleDataType :: Int -> DataType
tupleDataType n = mkDerivedDataType ("Prelude." ++ name) [(name, [], False)]
  where name = "(" ++ replicate (n - 1) ',' ++ ")"
