module Queens where

queens :: Int -> [[Int]]
queens n = place n
  where
    place 0 = [[]]
    place k = [ q : qs | qs <- place (k - 1)
                       , q  <- [1 .. n]
                       , safe q qs ]
    safe q qs = and [ q /= c && abs (q - c) /= d
                    | (d, c) <- zip [1 ..] qs ]
