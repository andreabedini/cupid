# Check every build configuration

Build and run the regression suite in each configuration the feature
options make, rather than only the one your build directory happens to
be set up with.

## Prerequisites

- meson, ninja, bison and a C compiler, as for an ordinary build
- a few minutes: twelve configurations, around a minute and a half in
  total on sixteen cores

## Steps

### 1. Run the sweep

```sh
tools/check-configurations
```

It builds and tests the default configuration, each feature option on
its own, and all of them together, and prints a line per configuration:

```
12 configurations, 4 at a time, under build-configurations

ok   default                        41 tests, 0 failed
ok   profiling                      41 tests, 0 failed
...
12 configurations, 0 failed
```

A configuration that fails prints the last forty lines of whichever
stage failed -- setup, build or test -- after the summary, and the
script exits non-zero.

### 2. Rerun after a change

The build directories under `build-configurations/` are kept, so a
second run rebuilds only what changed.  To repeat one configuration
while fixing it, name it:

```sh
tools/check-configurations all
tools/check-configurations profiling tag-checks
```

`--list` prints the names.  `--jobs N` sets how many run at once, and
`--build-root DIR` puts the build directories somewhere else.

### 3. Choose a compiler

`CC` is passed through:

```sh
CC=clang tools/check-configurations
```

## What it does not cover

Each option is tested alone and all are tested together, which is
twelve configurations rather than the 1024 the options can make.  The
one bug this was written after needed two options at once -- `profiling`
and `tag-checks` -- and was caught only by the run with all of them on.
A fault that needs a particular pair, and that some third option masks,
would still get through.

## See also

- [The design of Cupid](../explanation/design.md#platforms-and-the-build) for why
  the suite runs under every option
- `.github/workflows/ci.yml`, which runs this on Linux and macOS with
  both gcc and clang
