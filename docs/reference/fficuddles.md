# fficuddles

Compile a module that contains foreign declarations into a loadable
object.

## Synopsis

```
fficuddles [option ...] [-iinclude ...] file [compiler-argument ...]
```

## Description

`fficuddles` loads *file* as `cuddles` would, and for the module in it
writes a C file with the same base name containing a stub for each
`foreign import`.  It then compiles that file with the C compiler Cuddles
was built with, producing `Module.so` (`Module.dll` on Windows) next to
the source.  When `cuddles` or `runcuddles` later loads the module it loads
the object as well.

The compile command is

```
cc -shared -fPIC -D__CUDDLES__ -I$CUDDLESDIR/include -o Module.so Module.c compiler-argument ...
```

with the compiler and flags substituted for the target.  On failure the
whole command is printed.

Modules imported by *file* are loaded first, so a module with foreign
imports must have been compiled before any module that imports it is.

## Arguments

| Position | Meaning |
|---|---|
| Before *file*, beginning with `+` or `-` | Options for the interpreter, as in [Options](options.md), such as `-P` |
| `-i`*include* | Add `#include` *include* to the generated C.  *include* is what follows `#include` in C, so `'-i<math.h>'` or `'-i"defs.h"'`, quoted for the shell |
| *file* | The first argument that does not begin with `+` or `-` |
| After *file* | Passed unchanged to the C compiler: extra sources, `-I`, `-l`, `-L` and so on |

A `foreign import` string of the form `"header.h name"` has the same
effect as `-i"header.h"` for that declaration.  Since the generated
code calls the function by name, one of the two is needed under C23,
which does not allow implicit declarations.

## Files

| File | Role |
|---|---|
| `Module.c` | Generated stubs; overwritten on each run |
| `Module.so` | The compiled object that Cuddles loads with the module |
| `$CUDDLESDIR/include/HsFFI.h` | Types and the interpreter API used by the stubs |

## Examples

```sh
fficuddles Hyp.hs clib.c -lm
fficuddles '-i"clib.h"' Hyp.hs clib.c -lm
```

## See also

- [Call C functions from Haskell](../how-to/call-c-functions.md)
- [The language Cuddles accepts](language.md#foreign-function-interface)
  for the supported calling conventions and import forms
