{-# OPTIONS_GHC -fno-implicit-prelude #-}
-----------------------------------------------------------------------------
-- |
-- Module      :  Text.Read
-- Copyright   :  (c) The University of Glasgow 2001
-- License     :  BSD-style (see the file libraries/base/LICENSE)
-- 
-- Maintainer  :  libraries@haskell.org
-- Stability   :  provisional
-- Portability :  non-portable (uses Text.ParserCombinators.ReadP)
--
-- Converting strings to values.
--
-- The "Text.Read" library is the canonical library to import for
-- 'Read'-class facilities.  For GHC only, it offers an extended and much
-- improved 'Read' class, which constitutes a proposed alternative to the 
-- Haskell 98 'Read'.  In particular, writing parsers is easier, and
-- the parsers are much more efficient.
--
-----------------------------------------------------------------------------

module Text.Read (
   -- * The 'Read' class
   Read(..),		-- The Read class
   ReadS, 		-- String -> Maybe (a,String)

   -- * Haskell 98 functions
   reads,		-- :: (Read a) => ReadS a
   read,		-- :: (Read a) => String -> a
   readParen, 		-- :: Bool -> ReadS a -> ReadS a
   lex,			-- :: ReadS String


   -- * New parsing functions
   module Text.ParserCombinators.ReadPrec,
   L.Lexeme(..),	
   lexP,		-- :: ReadPrec Lexeme
   parens,		-- :: ReadPrec a -> ReadPrec a
   readMaybe,		-- :: Read a => String -> Maybe a
   readEither,		-- :: Read a => String -> Either String a






 ) where





import Text.ParserCombinators.ReadPrec
import qualified Text.Read.Lex as L



-- copied from GHC.Read

lexP :: ReadPrec L.Lexeme
lexP = lift L.lex

parens :: ReadPrec a -> ReadPrec a
parens p = optional
 where
  optional  = p +++ mandatory
  mandatory = do
    L.Punc "(" <- lexP
    x          <- reset optional
    L.Punc ")" <- lexP
    return x

-- | Parse a string with 'reads', succeeding only when the whole string,
-- but for white space, is a single value.
readEither :: Read a => String -> Either String a
readEither s =
  case [ x | (x, rest) <- reads s, ("", "") <- lex rest ] of
    [x] -> Right x
    []  -> Left "Prelude.read: no parse"
    _   -> Left "Prelude.read: ambiguous parse"

-- | Parse a string with 'reads', or 'Nothing' if it is not a single
-- value.
readMaybe :: Read a => String -> Maybe a
readMaybe s = case readEither s of
  Left _  -> Nothing
  Right a -> Just a
