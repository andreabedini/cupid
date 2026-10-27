# Check every build configuration

Build and run the regression suite in each configuration the feature
options make, rather than only the one your build directory happens to
be set up with.

## Prerequisites

- meson, ninja, bison, python3 and a C compiler, as for an ordinary build
- a few minutes: four configurations

## Steps

### 1. Run the sweep

```sh
tools/check-configurations
```

It builds and tests the default configuration, each feature option on
its own, and all of them together, and prints a line per configuration:

```
4 configurations, 4 at a time, under build-configurations

ok   default                        126 tests, 0 failed
ok   profiling                      126 tests, 0 failed
ok   checks                         126 tests, 0 failed
ok   all                            126 tests, 0 failed

4 configurations, 0 failed
```

The lines come out in the order the configurations finish, so the order
varies.

A configuration that fails prints the last forty lines of whichever
stage failed -- setup, build or test -- after the summary, and the
script exits non-zero.

### 2. Rerun after a change

The build directories under `build-configurations/` are kept, so a
second run rebuilds only what changed.  To repeat one configuration
while fixing it, name it:

```sh
tools/check-configurations all
tools/check-configurations profiling checks
```

`--list` prints the names.  `--jobs N` sets how many run at once, and
`--build-root DIR` puts the build directories somewhere else.

### 3. Choose a compiler

`CC` is passed through:

```sh
CC=clang tools/check-configurations
```

## What it does not cover

There are two feature options, so the default, each alone and both
together are every configuration they can make: the sweep is
exhaustive.  It stops being so if a third option is added, since three
alone and all together are five of the eight.  The one bug the sweep was
written after needed two options at once -- `profiling` and what was
then `tag-checks` -- and was caught only by the run with all of them on.

## See also

- [The design of Cupid](../explanation/design.md#platforms-and-the-build) for why
  the suite runs under every option
- `.github/workflows/ci.yml`, which runs this on Linux and macOS with
  both gcc and clang
