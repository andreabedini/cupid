-- Loading this chases an import, so the script stack holds the Prelude,
-- ScriptAux and this script.  The session reloads, unloads and reloads
-- them to exercise the script numbering.
module Main where

import ScriptAux

main :: IO ()
main = print (auxValue + 1)
