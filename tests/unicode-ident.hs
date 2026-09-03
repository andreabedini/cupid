-- Regression test: non-Latin-1 letters must be classified through
-- the Unicode tables, not folded onto the Latin-1 byte with the
-- same low eight bits (isIn used to cast the code point to
-- unsigned char).  U+0131 (dotless i) is a lowercase letter and
-- must lex as an identifier, not as the integer 257; Greek and
-- Cyrillic letters must work too, and an uppercase letter must
-- start a constructor.

ı :: Int
ı = 100

αβγ :: Int
αβγ = 23

data Λόγος = Λόγος Int deriving Show

main :: IO ()
main = do
  print (ı + αβγ)
  print (Λόγος 7)
