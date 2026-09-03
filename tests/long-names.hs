-- Regression test: `:names` used to format each name into a fixed
-- `static char newVar[60]` with sprintf("%s.%s", module, name), which
-- overflowed when the module name and the identifier together were 59
-- characters or more.  This module's name (41 chars) plus its exported
-- identifier's name (22 chars), joined by a dot, is 64 characters.
module ModuleWithAVeryLongNameForTestingXYZ where

veryLongIdentifierNameForTestX :: Int
veryLongIdentifierNameForTestX = 42
