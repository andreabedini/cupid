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
tree, which enables exactly five checks:

- `performance-no-int-to-ptr`
- `bugprone-suspicious-realloc-usage`
- `bugprone-implicit-widening-of-multiplication-result`
- `bugprone-narrowing-conversions`
- `bugprone-switch-missing-default-case`

### 2. Review the findings by hand

Findings are never auto-fixed: the two checks that looked mechanically
safe both broke the build when applied automatically.  Read each
finding against the code it points at.
