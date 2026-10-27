# Install a release

Install Cupid from a binary archive on the
[releases page](https://github.com/andreabedini/cupid/releases),
without building it.  One command does the whole job; the steps it
takes are also written out below, for doing them by hand.

No release has been published yet.  Until there is one, the install
commands stop with a message saying so; follow
[Build Cupid on Linux and macOS](build-on-linux-and-macos.md) or
[Build Cupid on Windows](build-on-windows.md) instead.

## Prerequisites

- Linux with glibc 2.35 or later (Ubuntu 22.04, Debian 12, Fedora 36 or
  later), or macOS 13 or later; on x86-64 or ARM64
- or Windows 10 or later, on x86-64
- on Linux, libedit, for line editing at the prompt: the mainstream
  distributions install it with OpenSSH; otherwise install `libedit2`
  (Debian, Ubuntu) or `libedit` (Fedora)
- a C compiler, only to use `fficupid`: on Windows, MinGW-w64 gcc for
  the UCRT

## The one-line install

On Linux and macOS:

```sh
curl -fsSL https://andreabedini.github.io/cupid/install.sh | sh
```

On Windows, in PowerShell:

```powershell
irm https://andreabedini.github.io/cupid/install.ps1 | iex
```

Either script finds the latest release, downloads the archive for your
system, checks it against `SHA256SUMS` from the same release, unpacks
it and puts the programs on your PATH.  Nothing needs root or
administrator rights.  On Linux and macOS the release goes in
`~/.local/opt/cupid-VERSION-TARGET` and the programs are linked into
`~/.local/bin`; on Windows it goes in `%LOCALAPPDATA%\Programs` and its
`bin` directory is added to your user PATH.

Three environment variables change what the scripts do:

| Variable            | Meaning                                   | Default                          |
|---------------------|-------------------------------------------|----------------------------------|
| `CUPID_VERSION`     | the version to install, such as `0.2.0`   | the latest release               |
| `CUPID_INSTALL_DIR` | where the release directory goes          | `~/.local/opt` or `%LOCALAPPDATA%\Programs` |
| `CUPID_BIN_DIR`     | where the links go (Linux and macOS only) | `~/.local/bin`                   |

For example, to install a particular version somewhere else:

```sh
CUPID_VERSION=0.2.0 CUPID_INSTALL_DIR=/opt curl -fsSL https://andreabedini.github.io/cupid/install.sh | sh
```

Read the script before you run it if you like:
[install.sh](https://andreabedini.github.io/cupid/install.sh),
[install.ps1](https://andreabedini.github.io/cupid/install.ps1).

## The same by hand

### 1. Download and check the archive

Pick the archive for your system: `cupid-VERSION-TARGET.tar.gz` with
TARGET one of `x86_64-linux`, `aarch64-linux`, `x86_64-macos` and
`aarch64-macos`, or `cupid-VERSION-x86_64-windows.zip`.  Check it
against `SHA256SUMS` from the same release:

```sh
sha256sum --ignore-missing -c SHA256SUMS
```

On macOS, use `shasum -a 256 --ignore-missing -c SHA256SUMS`.  On
Windows, compare the hash PowerShell prints with the line for the
archive in `SHA256SUMS`:

```powershell
(Get-FileHash cupid-VERSION-x86_64-windows.zip).Hash
```

With gh, you can also check that CI built it:

```sh
gh attestation verify cupid-VERSION-TARGET.tar.gz --repo andreabedini/cupid
```

### 2. Unpack it anywhere

```sh
mkdir -p ~/.local/opt ~/.local/bin
tar -xzf cupid-VERSION-TARGET.tar.gz -C ~/.local/opt
```

On Windows:

```powershell
Expand-Archive cupid-VERSION-x86_64-windows.zip $env:LOCALAPPDATA\Programs
```

The archive has one directory, `cupid-VERSION-TARGET`, with `bin`,
`lib/cupid` and `share/man` inside.  The programs find their libraries
next to themselves, so the directory can go anywhere and move later.

### 3. Put the programs on your PATH

```sh
ln -s ~/.local/opt/cupid-VERSION-TARGET/bin/* ~/.local/bin/
```

On Windows, add the `bin` directory to your user PATH, then open a new
terminal:

```powershell
$bin = "$env:LOCALAPPDATA\Programs\cupid-VERSION-x86_64-windows\bin"
[Environment]::SetEnvironmentVariable('Path', "$bin;" + [Environment]::GetEnvironmentVariable('Path', 'User'), 'User')
```

## Verify it worked

```sh
cupid
```

```
Cupid VERSION, a Haskell interpreter written in C
```

If it stops with `FATAL ERROR: Unable to load Prelude`, `CUPIDDIR` is
probably set in your environment and points somewhere else; unset it.

## See also

- [Build Cupid on Linux and macOS](build-on-linux-and-macos.md) and
  [Build Cupid on Windows](build-on-windows.md), to build from source
- [Getting started with Cupid](../tutorials/getting-started.md), for a
  first session once it is installed
