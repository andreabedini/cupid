-- Regression test for reading a well-formed 4-byte UTF-8 sequence that
-- encodes a code point above U+10FFFF (F4 90 80 80 = U+110000).  The
-- decoder must reject it rather than let the out-of-range code point
-- reach consChar, which used to read past the end of consCharTable.
main :: IO ()
main = do
  s <- readFile "bad-utf8-decode.dat"
  print (map fromEnum s)
