# Measure time and memory use

Find out how much work an evaluation does and how much heap it holds
on to.

## Prerequisites

- an expression to measure, loaded in an interactive session

## Steps

### 1. Count reductions and allocations

```
Cuddles> :set +s
Cuddles> length [1..10000]
10000
(210054 reductions, 278105 cells)
```

The reduction count is a crude measure of the work done; the cell count
is the total allocated, not the amount live at any one time.  Even the
most trivial evaluation costs a few dozen of each, because Cuddles wraps
the expression in code to print the value and catch exceptions.

### 2. Watch the garbage collector

```
Cuddles> :set +g
Cuddles> length [1..60000]
{{Gc:986865}}60000
```

Each `{{Gc:n}}` is a collection that recovered `n` cells.  With the
default heap of a million cells most evaluations trigger few
collections, so for this step start Cuddles with a small heap to see the
pattern:

```sh
cuddles -h250k
```

```
Cuddles> :set +g
Cuddles> length [1..60000]
{{Gc:236848}}{{Gc:236850}}{{Gc:236845}}{{Gc:236850}}{{Gc:236853}}{{Gc:236853}}{{Gc:236853}}60000
```

The number recovered is the same at every collection, so the
computation runs in constant space.  Compare:

```
Cuddles> let xs = [1..60000] in sum xs `div` length xs
{{Gc:220934}}{{Gc:204361}}{{Gc:189033}}{{Gc:174857}}{{Gc:161749}}{{Gc:149611}}
{{Gc:138397}}{{Gc:128015}}{{Gc:118413}}{{Gc:109527}}{{Gc:101314}}{{Gc:93713}}
{{Gc:86686}}{{Gc:80184}}{{Gc:74178}}{{Gc:68605}}{{Gc:63463}}{{Gc:59279}}
{{Gc:77814}}{{Gc:103752}}{{Gc:138336}}{{Gc:184452}}30000
```

The amount recovered falls steadily while `sum` traverses the list,
because `xs` is kept alive for `length`, and climbs again once
`length` starts consuming it.  The messages are interleaved with the
program's own output, so turn `+g` off again once you have seen what
you need:

```
Cuddles> :set -g -s
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
cuddles -h4M Main.hs
```

The default is one million cells.  Setting `-h` with `:set` does not
take effect in the running interpreter.

### 4. Profile the heap (profiling builds only)

A Cuddles built with `-Dprofiling=true` writes a heap profile to
`profile.hp` when told to sample every `n` reductions with `-dn`:

```sh
cuddles -d100 Main.hs
```

View it with hp2ps:

```sh
hp2ps profile.hp
```

## See also

- [Options](../reference/options.md) for `+s`, `+g`, `-h` and `-d`
- [Build options and install layout](../reference/build-options.md)
  for `profiling`
- [The language Cuddles accepts](../reference/language.md#other-known-bugs)
  for space leaks from top-level bindings
