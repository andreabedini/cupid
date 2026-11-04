-- This N-Queens program is based on a small variation of the 8-queens
-- program from Bird and Wadler's book.

module Queens where
import Gofer

queens number_of_queens  = qu number_of_queens where
    qu 0          = [[]]
    qu m          = [ p++[n] | p<-qu (m-1), n<-[1..number_of_queens], safe p n ]

safe p n          = all not [ check (i,j) (m,n) | (i,j) <- zip [1..] p ]
                    where m = 1 + length p

check (i,j) (m,n) = j==n || (i+j==m+n) || (i-j==m-n)

-- Use q 5 to see the list of solutions for 5 queens.
-- Use q 8 to see the list of solutions for 8 queens ....
q = putStr . layn . map show . queens

