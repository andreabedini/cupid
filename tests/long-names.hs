-- `:names` must print a qualified name of any length, with no fixed buffer
-- to overflow.  This module's name (36 chars) plus its exported identifier's
-- name (30 chars), joined by a dot, is 67 characters.


module ModuleWithAVeryLongNameForTestingXYZ where

veryLongIdentifierNameForTestX :: Int
veryLongIdentifierNameForTestX = 42
