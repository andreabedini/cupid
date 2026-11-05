# Run the static analysis

Run the curated set of clang-tidy checks over the C sources.

## Prerequisites

- clang-tidy installed
- a configured build directory (meson finds clang-tidy at setup time)

## Steps

### 1. Run the checks

```sh
ninja -C build clang-tidy
```

meson runs clang-tidy with the `.clang-tidy` file at the top of the
tree, which enables exactly four checks:

- `bugprone-suspicious-realloc-usage`
- `bugprone-implicit-widening-of-multiplication-result`
- `bugprone-narrowing-conversions`
- `bugprone-switch-missing-default-case`

### 2. Review the findings by hand

Findings are never auto-fixed: the auto-fixes of `macro-parentheses` and
`use-internal-linkage`, neither of them enabled now, broke the build.
Read each finding against the code it points at.  Before acting on one,
check it against a build with `profiling` and `checks` on (see
[Check every build configuration](check-every-configuration.md)): a finding
in one configuration can be wrong for the tree.

## See also

- [The design of Cupid](../inside/design.md#the-c-code) for why the check list
  is short
