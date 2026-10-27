# Build Cupid on Windows

Build Cupid on x86-64 Windows with MinGW-w64 against the UCRT, using
native Windows programs only: no MSYS2 shell, Cygwin or WSL.

## Prerequisites

- Windows 10 or later on x86-64
- MinGW-w64 gcc for the UCRT, Python, meson 1.1 or later and ninja.
  MSYS2's UCRT64 packages are the simplest source.  They are Windows
  programs, installed in `C:\msys64\ucrt64\bin`; the MSYS2 shell is only
  needed to install them:

  ```sh
  pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-meson \
            mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-python
  ```

- bison: WinFlexBison's `win_bison`, from
  `winget install WinFlexBison.win_flex_bison` or the zip on its
  [releases page](https://github.com/lexxmark/winflexbison/releases)
- git, with CRLF conversion off for this repository: the tests compare
  output byte for byte

## Steps

### 1. Put the tools on PATH

In PowerShell:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;C:\path\to\win_flex_bison;$env:PATH"
```

Use WinFlexBison's own directory, where `win_bison.exe` sits next to its
`data` directory.  The `win_bison` link that winget makes does not work:
the program looks for `data` next to the path it was started from.
winget puts the directory under
`%LOCALAPPDATA%\Microsoft\WinGet\Packages\`.

### 2. Get the sources

```powershell
git clone -c core.autocrlf=false https://github.com/andreabedini/cupid.git
cd cupid
```

### 3. Configure, build and test

```powershell
meson setup build --prefix=C:/cupid
ninja -C build
meson test -C build
```

Choose the prefix: meson's default on Windows is `c:/`.  The tests
compile small helper programs with the gcc on PATH.

### 4. Install

```powershell
ninja -C build install
```

The programs go in `C:\cupid\bin` and the libraries in
`C:\cupid\lib\cupid`.  The programs find the libraries relative to
themselves, so the whole directory can be moved later.  Add its `bin`
to your PATH.

## Verify it worked

```powershell
cupid
```

```
Cupid VERSION, a Haskell interpreter written in C
```

## Run it from the build tree

You do not have to install to use what you built.  Point `CUPIDDIR` at
the built library directory and run the programs from `build\src`:

```powershell
$env:CUPIDDIR = "$PWD\build\libraries\cupiddir"
$env:PATH = "$PWD\build\src;$env:PATH"
cupid
```

The two settings last for the current PowerShell session.  If the
libraries cannot be found, the interpreter stops before the prompt with
a message that the `Prelude` cannot be located.
[Environment variables](../reference/environment-variables.md)
describes `CUPIDDIR`.

## What is different on Windows

- `System.Posix.Internals`, `System.Posix.Signals` and
  `System.Process.Internals` are not there, and `System.Posix.Types`
  has only the types Windows has; see
  [the libraries](../reference/libraries.md#windows).
- Line editing at the prompt comes from WinEditLine, linked into the
  programs.
- `fficupid` compiles with the gcc on PATH, so it needs MinGW-w64 gcc
  for the UCRT.
- The two tests that limit the C stack are skipped: Windows fixes the
  stack size when a program is linked.

## See also

- [Install a release](install-a-release.md), for a build that is ready
  to unpack
- [Build Cupid on Linux and macOS](build-on-linux-and-macos.md)
- [Build options and install layout](../reference/build-options.md)
