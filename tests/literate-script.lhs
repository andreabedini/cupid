#! /usr/local/bin/cupid
A literate script (Haskell 2010 Report, section 10.4): the lines that
begin with > are the program, and every other line is commentary.  The
#! line above is read as blank, so it may sit next to commentary.

> module Main where
>
> import Literate.Bird (greeting)

Commentary may mention > or code, as long as the > is not the first
character of its line.  A line of spaces only counts as blank:
   
> main :: IO ()
> main = do
>     putStrLn greeting
>     print (total [1, 2, 3])
>   where
>	total = sum . map double
>	double x = 2 * x

The program ends here; this commentary has no program line after it.
