-- Hiding a name that the module does not export is not an error, as in
-- GHC: code written for a Prelude that exported catch hides it to use
-- Control.Exception's.
module Main where

import Prelude hiding (catch, noSuchName)
import Control.Exception (catch, ErrorCall, evaluate)

main :: IO ()
main = catch (evaluate (error "boom") >> return ())
             (\e -> putStrLn ("caught " ++ show (e :: ErrorCall)))
