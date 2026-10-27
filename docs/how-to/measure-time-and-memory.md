# Measure time and memory use

Find out how much work an evaluation does and how much heap it holds
on to.

## Prerequisites

- an expression to measure, loaded in an interactive session

## Steps

### 1. Count reductions and allocations

```
Cupid> :set +s
Cupid> length [1..10000]
10000
(210056 reductions, 278134 cells)
```

The reduction count is a crude measure of the work done; the cell count
is the total allocated, not the amount live at any one time.  Even the
most trivial evaluation costs a few dozen of each, because Cupid wraps
the expression in code to print the value and catch exceptions.

### 2. Watch the garbage collector

```
Cupid> :set -s +g
Cupid> length [1..60000]
{{Gc:982638}}60000
```

Each `{{Gc:n}}` is a collection that recovered `n` cells.  With the
default heap of a million cells most evaluations trigger few
collections, so for this step start Cupid with a small heap to see the
pattern:

```sh
cupid -h250k
```

```
Cupid> :set +g
Cupid> length [1..60000]
{{Gc:232614}}{{Gc:232619}}{{Gc:232616}}{{Gc:232612}}{{Gc:232618}}{{Gc:232617}}{{Gc:232612}}60000
```

The number recovered is the same at every collection, so the
computation runs in constant space.  Compare:

```
Cupid> let xs = [1..60000] in sum xs `div` length xs
{{Gc:217013}}{{Gc:200730}}{{Gc:185683}}{{Gc:171755}}{{Gc:158875}}{{Gc:146954}}
{{Gc:135933}}{{Gc:125743}}{{Gc:116306}}{{Gc:107594}}{{Gc:99518}}{{Gc:92055}}
{{Gc:85150}}{{Gc:78765}}{{Gc:72861}}{{Gc:67389}}{{Gc:62335}}{{Gc:57659}}
{{Gc:58828}}{{Gc:78437}}{{Gc:104584}}{{Gc:139443}}{{Gc:185925}}30000
```

The amount recovered falls steadily while `sum` traverses the list,
because `xs` is kept alive for `length`, and climbs again once
`length` starts consuming it.  The messages are interleaved with the
program's own output, and one more appears just before the next prompt,
so turn `+g` off again once you have seen what
you need:

```
Cupid> :set -g -s
```

### 3. Give the program more heap

The heap size is fixed for the life of the interpreter.  If a
computation ends with

```
ERROR - Garbage collection fails to reclaim sufficient space
```

restart with a bigger heap.  The unit is cells, and `k`, `M` and `G`
suffixes are accepted:

```sh
cupid -h4M -i Main.hs
```

The default is one million cells.  Setting `-h` with `:set` does not
take effect in the running interpreter.

### 4. Profile the heap (profiling builds only)

A Cupid built with `-Dprofiling=true` writes a heap profile to
`profile.hp` when told to sample every `n` reductions with `-dn`:

```sh
cupid -d100 -i Main.hs
```

View it with hp2ps:

```sh
hp2ps profile.hp
```

## See also

- [Options](../reference/options.md) for `+s`, `+g`, `-h` and `-d`
- [Build options and install layout](../reference/build-options.md)
  for `profiling`
- [The language Cupid accepts](../reference/language.md#other-known-bugs)
  for space leaks from top-level bindings
