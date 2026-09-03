# Build Hugs on Windows

Build Hugs with MinGW-w64 or MSVC.  The meson build is written to
support Windows but has not yet been verified there, so expect to fix a
few things.

## Prerequisites

- MinGW-w64 or Visual Studio with the C compiler
- meson 1.1 or later, ninja and python3

## Steps

### 1. Configure and build

The procedure is the same as on Unix:

```sh
meson setup build
ninja -C build
ninja -C build install
```

### 2. Expect to adjust these files

The platform-specific choices live in a few places:

- `src/sysdeps.h` — the facts about each supported target
- `meson.build` — shared-library flags and install paths; the Windows
  search path is `.;{Hugs}\packages\*` and the suffix list `.hs;.lhs`
- `libraries/tools/mksysconfig.c` — the C type widths and the errno
  and signal constants written into `Hugs.SysConfig`

The library sources under `libraries/base` are the POSIX expansion of
the 2006 sources.  Their Windows code paths were dropped when the
library became plain Haskell and would have to be brought
back by hand.

### 3. What is not built

The WinHugs graphical interface and the .NET bridge are not part of
the supported build; their sources, under
`src/winhugs` and `src/dotnet` in the 2006 release, have been deleted
from this tree.

## See also

- [Build and install Hugs](build-and-install.md)
- [Build options and install layout](../reference/build-options.md)
