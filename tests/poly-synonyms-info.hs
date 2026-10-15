-- :info shows a polymorphic synonym with its forall, and :type the type a
-- signature that uses one has once the forall is moved to its front.
module Main where

type Lens s t a b = forall f. Functor f => (a -> f b) -> s -> f t
type SimpleLens s a = Lens s s a a

lens :: (s -> a) -> (s -> b -> t) -> Lens s t a b
lens sa sbt afb s = fmap (sbt s) (afb (sa s))

_2 :: SimpleLens (c, a) a
_2 = lens snd (\(c, _) a -> (c, a))

main :: IO ()
main = return ()
