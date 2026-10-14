-- | Generic programming over the structure of data types, after GHC's
-- module of the same name.  @deriving Generic@ gives a type a
-- representation built from the types below, and functions written once
-- over the representation then work for every such type.
--
-- Two things differ from GHC, both because Cupid has no type families
-- and no type-level strings (see docs/reference/extensions.md):
--
-- * 'Generic' has the representation as a second parameter, determined
--   by the first, rather than as the associated type @Rep a@.  Write
--   @(Generic a r, GShow r) => a -> String@ where GHC code writes
--   @(Generic a, GShow (Rep a)) => a -> String@.
--
-- * The metadata in a representation is not a list of type-level
--   strings but a type index into a value: 'MetaData' @t@ for the type,
--   'MetaCons' @t n@ for its @n@th constructor and 'MetaSel' @t n m@ for
--   that constructor's @m@th field, counting from 'Zero'.  'Datatype',
--   'Constructor' and 'Selector' read the names from 'datatypeInfo',
--   which @deriving Generic@ also defines, so code that only uses those
--   classes is unaffected.
module GHC.Generics (
    -- * Representation types
    V1, U1(..), K1(..), M1(..), (:+:)(..), (:*:)(..),
    Par1(..), Rec1(..), (:.:)(..),
    R, D, C, S, Rec0, D1, C1, S1,
    -- * The Generic class
    Generic(..),
    -- * Metadata
    Datatype(..), Constructor(..), Selector(..),
    Fixity(..), Associativity(..), prec,
    MetaData, MetaCons, MetaSel, Zero, Succ,
    DataInfo(..), ConInfo(..)
  ) where

infixr 5 :+:
infixr 6 :*:
infixr 7 :.:

-- | The representation of a type with no constructors.
data V1 p

-- | A constructor without fields.
data U1 p = U1 deriving (Eq, Ord, Show)

-- | A field of type @c@; @i@ is always 'R'.
newtype K1 i c p = K1 { unK1 :: c }

-- | Metadata @c@ of kind @i@ ('D', 'C' or 'S') around @f@.
newtype M1 i c f p = M1 { unM1 :: f p }

-- | A choice between constructors.
data (f :+: g) p = L1 (f p) | R1 (g p)

-- | The fields of a constructor, side by side.
data (f :*: g) p = f p :*: g p

-- | The parameter, in representations of type constructors.  No
-- @deriving@ produces these yet; they are here so that code mentioning
-- them loads.
newtype Par1 p = Par1 { unPar1 :: p }
newtype Rec1 f p = Rec1 { unRec1 :: f p }
newtype (f :.: g) p = Comp1 { unComp1 :: f (g p) }

-- | The tags of 'K1' and 'M1'.
data R
data D
data C
data S

type Rec0 = K1 R
type D1 = M1 D
type C1 = M1 C
type S1 = M1 S

-- | A type @a@ with representation @rep@: 'from' and 'to' convert between
-- them, and 'datatypeInfo' describes the declaration of @a@.  Its argument
-- is never evaluated.
class Generic a rep | a -> rep where
    from         :: a -> rep x
    to           :: rep x -> a
    datatypeInfo :: a -> DataInfo

-- | What @deriving Generic@ records of a declaration: the type's name, its
-- module, whether it is a newtype, and its constructors in order.
data DataInfo = DataInfo String String Bool [ConInfo]

-- | A constructor's name, fixity, whether it is declared with field
-- labels, and the labels, one per field, empty when it has none.
data ConInfo = ConInfo String Fixity Bool [String]

data Fixity = Prefix | Infix Associativity Int
    deriving (Eq, Show, Ord, Read)

data Associativity = LeftAssociative | RightAssociative | NotAssociative
    deriving (Eq, Show, Ord, Read, Enum, Bounded)

-- | The precedence of a fixity; prefix application is 10.
prec :: Fixity -> Int
prec Prefix      = 10
prec (Infix _ n) = n

-- | The indices of the metadata.
data MetaData t
data MetaCons t n
data MetaSel t n m

-- | Type-level numbers, which index constructors and fields.
data Zero
data Succ n

class Index n where
    index :: n -> Int

instance Index Zero where
    index _ = 0

instance Index n => Index (Succ n) where
    index n = 1 + index (predOf n)

predOf :: Succ n -> n
predOf _ = undefined

-- | The type a representation describes.
class Datatype d where
    datatypeName :: M1 i d f p -> String
    moduleName   :: M1 i d f p -> String
    packageName  :: M1 i d f p -> String
    isNewtype    :: M1 i d f p -> Bool

class Constructor c where
    conName     :: M1 i c f p -> String
    conFixity   :: M1 i c f p -> Fixity
    conIsRecord :: M1 i c f p -> Bool

class Selector s where
    selName :: M1 i s f p -> String

instance Generic t r => Datatype (MetaData t) where
    datatypeName m = case dataInfo m of DataInfo n _ _ _ -> n
    moduleName m   = case dataInfo m of DataInfo _ n _ _ -> n
    packageName _  = ""
    isNewtype m    = case dataInfo m of DataInfo _ _ b _ -> b

dataInfo :: Generic t r => M1 i (MetaData t) f p -> DataInfo
dataInfo m = datatypeInfo (dataOf m)
  where dataOf :: M1 i (MetaData t) f p -> t
        dataOf _ = undefined

instance (Generic t r, Index n) => Constructor (MetaCons t n) where
    conName m     = case conInfo m of ConInfo n _ _ _ -> n
    conFixity m   = case conInfo m of ConInfo _ f _ _ -> f
    conIsRecord m = case conInfo m of ConInfo _ _ b _ -> b

conInfo :: (Generic t r, Index n) => M1 i (MetaCons t n) f p -> ConInfo
conInfo m = constructors (datatypeInfo (dataOf m)) !! index (conOf m)
  where dataOf :: M1 i (MetaCons t n) f p -> t
        dataOf _ = undefined
        conOf :: M1 i (MetaCons t n) f p -> n
        conOf _ = undefined

instance (Generic t r, Index n, Index m) => Selector (MetaSel t n m) where
    selName s = case constructors (datatypeInfo (dataOf s)) !! index (conOf s) of
                    ConInfo _ _ _ ls -> ls !! index (selOf s)
      where dataOf :: M1 i (MetaSel t n m) f p -> t
            dataOf _ = undefined
            conOf :: M1 i (MetaSel t n m) f p -> n
            conOf _ = undefined
            selOf :: M1 i (MetaSel t n m) f p -> m
            selOf _ = undefined

constructors :: DataInfo -> [ConInfo]
constructors (DataInfo _ _ _ cs) = cs

-- | What @deriving Generic@ uses for a type with no constructors.
voidFrom :: a -> V1 p
voidFrom x = x `seq` error "GHC.Generics.from: a type with no values"

voidTo :: V1 p -> a
voidTo x = x `seq` error "GHC.Generics.to: a type with no values"
