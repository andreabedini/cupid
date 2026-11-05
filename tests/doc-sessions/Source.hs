module Source where

answer = twice (3 + 4 * 5)

twice x =
  let y = x + x
  in y

evenLength [] = True
evenLength (_:xs) = oddLength xs
oddLength [] = False
oddLength (_:xs) = evenLength xs
