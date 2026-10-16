-- A recursion deep enough to overflow the C stack of every platform's
-- default size, without a NAME.stack limit: Windows cannot lower the
-- stack of a process it starts, so this is the overflow test that runs
-- there.  It must report the overflow, not crash.
main = print (foldr (+) 0 [1..1000000 :: Int])
