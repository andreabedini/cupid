-- | The names of the kinds, so that @import Data.Kind (Type, Constraint)@
-- is accepted.  In Cuddles a kind is not a type: the parser reads @Type@
-- and @Constraint@ in a kind signature as the kinds @*@ and @Constraint@
-- whatever is in scope, and the two empty data types here are never
-- used for anything else.  See docs/reference/extensions.md.
module Data.Kind (Type, Constraint) where

data Type
data Constraint
