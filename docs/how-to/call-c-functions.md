# Call C functions from Haskell

Compile a module that contains `foreign import` declarations with
`ffihugs` so that Hugs can load it.

## Prerequisites

- `ffihugs`, either installed or reachable from the build tree with
  `HUGSDIR` set (see [Run Hugs from the build tree](run-from-the-build-tree.md))
- the C compiler that Hugs was built with, since `ffihugs` runs the
  same command

## Steps

This guide wraps one C function, `half_hypot`, that lives in `clib.c`
and needs the maths library.

### 1. Give the C function a header

Put the prototype in `clib.h`:

```c
#include <math.h>
double half_hypot(double a, double b);
```

and the definition in `clib.c`:

```c
#include "clib.h"
double half_hypot(double a, double b) { return hypot(a, b) / 2; }
```

The header is not optional.  The C that `ffihugs` generates calls the
function by name, and a C23 compiler rejects a call to a function that
has not been declared.

### 2. Declare the import in Haskell

Create `Hyp.hs`.  The string after `ccall` names the header and the C
function:

```haskell
module Hyp where

foreign import ccall "clib.h half_hypot" halfHypot :: Double -> Double -> Double
```

### 3. Compile the module with ffihugs

Hugs options, if any, go before the module name.  Everything after the
module name is passed to the C compiler: extra sources, include paths,
libraries.

```sh
ffihugs Hyp.hs clib.c -lm
```

This writes `Hyp.c`, the glue between the interpreter and the C
function, and compiles it together with `clib.c` into `Hyp.so`
(`Hyp.dll` on Windows).  Any earlier `Hyp.so` is overwritten.

### 4. Use the module

Hugs loads `Hyp.so` whenever it loads `Hyp.hs`:

```sh
hugs Hyp.hs
```

```
Hyp> halfHypot 3 4
2.5
```

## Verify it worked

The directory now contains `Hyp.c` and `Hyp.so` next to `Hyp.hs`, and
the expression above prints `2.5`.

## Alternatives

### Name the header on the command line instead

If you would rather keep the import declaration short:

```haskell
foreign import ccall "half_hypot" halfHypot :: Double -> Double -> Double
```

then tell `ffihugs` what to include with `-i`.  The argument is exactly
what would follow `#include` in C, so it needs its own quotes:

```sh
ffihugs '-i"clib.h"' Hyp.hs clib.c -lm
ffihugs '-i<math.h>' Trig.hs -lm
```

### A module that needs `-98`

Options for Hugs go before the module name, and must be given again
when the module is loaded:

```sh
ffihugs -98 Hyp.hs clib.c -lm
hugs -98 Hyp.hs
```

### Several FFI modules

If `A.hs` imports `B.hs` and both contain foreign imports, compile `B`
first: `ffihugs` loads a module's imports, and `B.so` must exist for
that.

## Troubleshooting

### `implicit declaration of function` from the C compiler

**Cause**: the generated C calls the function without a prototype in
scope.

**Fix**: name a header in the import string, or pass one with `-i`, as
in step 1 and the first alternative above.

### `Error while importing DLL "./Hyp.so"`

**Cause**: `Hyp.hs` was loaded but `Hyp.so` does not exist next to it,
usually because `ffihugs` failed or was never run.

**Fix**: run `ffihugs` and read its output; the compile command it ran
is printed on failure.

### `Error while running compilation command`

**Cause**: the C compiler failed, or the compiler command Hugs was
configured with does not work as a plain command (a wrapper such as
`sccache cc`).

**Fix**: the failing command is printed in full; run it by hand to see
the compiler's message.  If the compiler itself cannot be run, rebuild
Hugs with `CC` naming the real compiler.

### A generated file clashes with your own

**Cause**: `ffihugs` writes a C file with the same base name as the
Haskell module, so `Hyp.hs` produces `Hyp.c`.

**Fix**: give your own C sources a different base name.

## See also

- [ffihugs](../reference/ffihugs.md) for the full command line
- [The language Hugs accepts](../reference/language.md#foreign-function-interface)
  for what the FFI does and does not support
- [Foreign import wrappers](../explanation/foreign-import-wrappers.md)
  on callbacks from C into Haskell
