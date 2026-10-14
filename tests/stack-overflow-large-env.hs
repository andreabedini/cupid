-- stack-overflow-foldr with 64 KB of environment.  The environment sits
-- above main's frame and counts against the stack limit; when the stack
-- check left it out, this died of SIGSEGV instead of reporting the
-- overflow.
main = print (foldr (+) 0 [1..15000 :: Int])
